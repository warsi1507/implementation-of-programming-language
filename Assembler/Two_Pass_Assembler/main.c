#include "assembler.h"

// Function Declarations
void pass1(FILE *input, FILE *intermediate);
void pass2(FILE *intermediate, FILE *output);
void parseLine(char *line, char *label, char *opcode, char *operand);
int calculateByteLength(char *operand);
int hexToInt(char c);
void writeTextRecord(FILE *output, unsigned int startAddr, char *objCode, int length);
void initializeTables();

// Global Variables for Pass-2
char textRecord[70];
int textLength = 0;
unsigned int textStartAddr = 0;
int firstTextRecord = 1;

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Usage: %s <input_file> <intermediate_file> <output_file>\n", argv[0]);
        return 1;
    }

    // initialize symbol and literal tables
    initializeTables();

    // open files
    FILE *input = fopen(argv[1], "r");
    FILE *intermediate = fopen(argv[2], "w");
    FILE *output = fopen(argv[3], "w");

    if (!input || !intermediate || !output) {
        printf("\x1b[31mERROR:\x1b[0m Could not open files\n");
        return 1;
    }

    // perform Pass-1
    printf("\n\x1b[34mINFO:\x1b[0m Starting Pass-1...\n");
    pass1(input, intermediate);
    fclose(input);
    fclose(intermediate);

    // reopen intermediate file for reading
    intermediate = fopen(argv[2], "r");
    if (!intermediate) {
        printf("\x1b[31mERROR:\x1b[0m Could not reopen intermediate file\n");
        return 1;
    }

    // perform Pass-2
    printf("\n\x1b[34mINFO:\x1b[0m Starting Pass-2...\n");
    pass2(intermediate, output);
    fclose(intermediate);
    fclose(output);

    printf("\n\x1b[32mRESULT:\x1b[0m Assembly Complete !\n");
    printf("Program Name: %s\n", PROGRAM_NAME);
    printf("Start Address: %04X\n", START_ADDRESS);
    printf("Program Length: %04X\n", PROGRAM_LENGTH);

    return 0;
}

// Initialize Tables
void initializeTables() {
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        SYMTAB[i] = NULL;
    }
    for (int i = 0; i < LITTAB_SIZE; i++) {
        LITTAB[i] = NULL;
    }
}

// Parse a line into label, opcode, operand
void parseLine(char *line, char *label, char *opcode, char *operand) {
    label[0] = opcode[0] = operand[0] = '\0';

    char temp[MAX_LINE_LEN];
    strcpy(temp, line);

    // check if line starts with whitespace (no label)
    if (temp[0] == ' ' || temp[0] == '\t') {
        char *token = strtok(temp, " \t\n");
        if (token) strcpy(opcode, token);

        token = strtok(NULL, "\n");
        if (token) {
            // Skip leading whitespace
            while (*token == ' ' || *token == '\t') token++;
            strcpy(operand, token);
            trim(operand);
        }
    } else {
        char *token = strtok(temp, " \t\n");
        if (token) strcpy(label, token);
        
        token = strtok(NULL, " \t\n");
        if (token) strcpy(opcode, token);
        
        token = strtok(NULL, "\n");
        if (token) {
            // Skip leading whitespace
            while (*token == ' ' || *token == '\t') token++;
            strcpy(operand, token);
            trim(operand);
        }
    }
}

// Calculate length for Byte directive
int calculateByteLength(char *operand) {
    if (operand[0] == 'C') {
        // character constant C'...'
        int len = strlen(operand) - 3;
        return len;
    } else if (operand[0] == 'X') {
        // hexadecimal constant X'...'
        int len = strlen(operand) - 3;
        return (len + 1) / 2;
    }
    return 0;
}

// Pass-1 : Build Symbol Table and Generate Intermediate File
void pass1(FILE *input, FILE *intermediate) {
    char line[MAX_LINE_LEN];
    char label[32], opcode[32], operand[32];
    int lineNumber = 0;

    // Read first line
    if (!fgets(line, MAX_LINE_LEN, input)) {
        printf("\x1b[31mERROR:\x1b[0m Empty input file\n");
        return;
    }

    lineNumber++;
    parseLine(line, label, opcode, operand);

    // check if its START directive
    if (strcmp(opcode, "START") == 0) {
        strcpy(PROGRAM_NAME, label);
        START_ADDRESS = (unsigned int)strtol(operand, NULL, 16);
        LOCCTR = START_ADDRESS;

        // write first line in intermediate file
        fprintf(intermediate, "%04X %-8s%-8s%s\n", LOCCTR, label, opcode, operand);
        printf("%04X %s %s %s\n", LOCCTR, label, opcode, operand);

        // read next line 
        if (!fgets(line, MAX_LINE_LEN, input)) return;
        lineNumber++;
        parseLine(line, label, opcode, operand);
    } else {
        LOCCTR = 0;
    }

    // process remaining lines
    while (strcmp(opcode, "END") != 0)
    {
        unsigned int currentLOCCTR = LOCCTR;

        // skip comment line
        if(!isComment(line)) {
            // if there is a label, insert into SYMTAB
            if (strlen(label) > 0) {
                int addr = searchSYMTAB(label);
                if (addr == -1) {
                    insertSYMTAB(label, LOCCTR);
                    printf("\x1b[33mSYMTAB:\x1b[0m Added %s at address %04X\n", label, LOCCTR);
                } else {
                    printf("\x1b[31mERROR:\x1b[0m Duplicate symbol %s\n", label);
                }
            }

            // search OPTAB for opcode
            OptabEntry *entry = searchOPTAB(opcode);
            if (entry != NULL) {
                // Found in OPTAB - add instruction length
                LOCCTR += entry->length;
            } 
            else if (strcmp(opcode, "WORD") == 0) {
                LOCCTR += 3;
            } 
            else if (strcmp(opcode, "RESW") == 0) {
                int count = atoi(operand);
                LOCCTR += 3 * count;
            } 
            else if (strcmp(opcode, "RESB") == 0) {
                int count = atoi(operand);
                LOCCTR += count;
            } 
            else if (strcmp(opcode, "BYTE") == 0) {
                LOCCTR += calculateByteLength(operand);
            } 
            else {
                printf("\x1b[31mERROR:\x1b[0m Invalid operation code %s at line %d\n", opcode, lineNumber);
            }

            // write line to intermediate file with address
            fprintf(intermediate, "%04X %-8s%-8s%s\n", currentLOCCTR, label, opcode, operand);
            printf("%04X  %-8s%-8s%s\n", currentLOCCTR, label, opcode, operand);
        }

        // read next line 
        if (!fgets(line, MAX_LINE_LEN, input)) break;
        lineNumber++;
        parseLine(line, label, opcode, operand);
    }
    
    // write last line (END)
    fprintf(intermediate, "%04X %-8s%-8s%s\n", LOCCTR, label, opcode, operand);
    printf("%04X  %s  %s  %s\n", LOCCTR, label, opcode, operand);

    // Calculate program length
    PROGRAM_LENGTH = LOCCTR - START_ADDRESS;
    
    printf("\n\x1b[32mRESULT:\x1b[0m Pass-1 Complete with Program Length: %04X (%d) !\n", PROGRAM_LENGTH, PROGRAM_LENGTH);
}

// hex character to integer
int hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

// Write accumulated text record
void writeTextRecord(FILE *output, unsigned int startAddr, char *objCode, int length) {
    if (length > 0) {
        fprintf(output, "T%06X%02X%s\n", startAddr, length, objCode);
        printf("T%06X%02X%s\n", startAddr, length, objCode);
    }
}

// Pass-2: Generate Oject Code
void pass2(FILE *intermediate, FILE *output) {
    char line[MAX_LINE_LEN];
    char label[32], opcode[32], operand[32];
    unsigned int address;

    textRecord[0] = '\0';
    textLength = 0;
    firstTextRecord = 1;

    // read first line
    if (!fgets(line, MAX_LINE_LEN, intermediate)) {
        printf("\x1b[31mERROR:\x1b[0m Empty intermediate file\n");
        return;
    }

    // parse first line 
    sscanf(line, "%X", &address);
    parseLine(line + 5, label, opcode, operand);

    // if START then write header record
    if (strcmp(opcode, "START") == 0) {
        fprintf(output, "H%-6s%06X%06X\n", PROGRAM_NAME, START_ADDRESS, PROGRAM_LENGTH);
        printf("H%-6s%06X%06X\n", PROGRAM_NAME, START_ADDRESS, PROGRAM_LENGTH);

        // read next line
        if (!fgets(line, MAX_LINE_LEN, intermediate)) return;
    }

    // process remaining lines
    while (1)
    {
        // parse line
        sscanf(line, "%X", &address);
        parseLine(line + 5, label, opcode, operand);

        if (strcmp(opcode, "END") == 0) break;
        
        if (isComment(line)) {
            if (!fgets(line, MAX_LINE_LEN, intermediate)) break;
            continue;
        }

        char objectCode[20] = "";
        int objCodeLength = 0;

        // search OPTAB
        OptabEntry *entry = searchOPTAB(opcode);
        if (entry != NULL) {
            //  machine instruction
            unsigned int operandAddr = 0;
            int indexed = 0;

            // parse operand for address and indexing
            char operandCopy[32];
            strcpy(operandCopy, operand);

            char *comma = strchr(operandCopy, ',');
            if (comma != NULL) {
                *comma = '\0';
                indexed = 1;
            }

            // search symbol table for operand
            if (strlen(operandCopy) > 0) {
                int symAddr = searchSYMTAB(operandCopy);
                if (symAddr != -1) {
                    operandAddr = symAddr;
                } else {
                    printf("\x1b[31mERROR:\x1b[0m Undefined symbol %s\n", operandCopy);
                    operandAddr = 0;
                }
            }

            // set indexed bit if needed
            if (indexed) {
                operandAddr |= 0x8000;  // Set bit 15
            }

            // generate object code
            sprintf(objectCode, "%02X%04X", entry->opcode, operandAddr);
            objCodeLength = 3;
        } 
        else if (strcmp(opcode, "BYTE") == 0) {
            // BYTE directive
            if (operand[0] == 'C') {
                // Character constant
                for (int i = 2; operand[i] != '\''; i++) {
                    sprintf(objectCode + strlen(objectCode), "%02X", (unsigned char)operand[i]);
                }
                objCodeLength = calculateByteLength(operand);
            } 
            else if (operand[0] == 'X') {
                // Hexadecimal constant
                int i = 2;
                while (operand[i] != '\'') {
                    objectCode[i-2] = operand[i];
                    i++;
                }
                objectCode[i-2] = '\0';
                objCodeLength = calculateByteLength(operand);
            }
        }
        else if (strcmp(opcode, "WORD") == 0) {
            // WORD directive
            int value = atoi(operand);
            sprintf(objectCode, "%06X", value);
            objCodeLength = 3;
        }

        // add object code to text record
        if (objCodeLength > 0) {
            // check if we need to start a new text record
            if (firstTextRecord) {
                textStartAddr = address;
                strcpy(textRecord, objectCode);
                textLength = objCodeLength;
                firstTextRecord = 0;
            } 
            else if (textLength + objCodeLength > 30 || address != (textStartAddr + textLength)) {
                // write current text record and start new one
                writeTextRecord(output, textStartAddr, textRecord, textLength);
                textStartAddr = address;
                strcpy(textRecord, objectCode);
                textLength = objCodeLength;
            }
            else {
                strcat(textRecord, objectCode);
                textLength += objCodeLength;
            }
        }

        // read next line
        if (!fgets(line, MAX_LINE_LEN, intermediate)) break;
    }
    
    // write final text record
    if (textLength > 0) {
        writeTextRecord(output, textStartAddr, textRecord, textLength);
    }

    // write end record
    char endOperand[32];
    strcpy(endOperand, operand);
    if (strlen(endOperand) > 0) {
        int startAddr = searchSYMTAB(endOperand);
        if (startAddr != -1) {
            fprintf(output, "E%06X\n", startAddr);
            printf("E%06X\n", startAddr);
        } else {
            fprintf(output, "E%06X\n", START_ADDRESS);
            printf("E%06X\n", START_ADDRESS);
        }
    } else {
        fprintf(output, "E%06X\n", START_ADDRESS);
        printf("E%06X\n", START_ADDRESS);
    }
    
    printf("\n\x1b[32mRESULT:\x1b[0m Pass-2 Complete !\n");
}