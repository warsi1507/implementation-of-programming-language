#include "assembler.h"

// Function Declarations
void onePassAssemble(FILE *input, FILE *output);
void parseLine(char *line, char *label, char *opcode, char *operand);
int calculateByteLength(char *operand);
void patchForwardReferences(FILE *output);
void writeTextRecords(FILE *output);
void initializeTables();


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("\x1b[31mERROR:\x1b[0m Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Initialize tables
    initializeTables();

    // Open files
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    if (!input || !output) {
        printf("\x1b[31mERROR:\x1b[0m Could not open files\n");
        return 1;
    }

    // Perform one-pass assembly
    printf("\x1b[34mINFO:\x1b[0m Starting One-Pass Assembly...\n");
    onePassAssemble(input, output);
    
    fclose(input);
    fclose(output);

    printf("\n\x1b[32mRESULT:\x1b[0m Assembly Complete!\n");
    printf("Program Name: %s\n", PROGRAM_NAME);
    printf("Start Address: %04X\n", START_ADDRESS);
    printf("Program Length: %04X\n", PROGRAM_LENGTH);

    return 0;
}

/*************************************
 * Initialize Tables
 *************************************/
void initializeTables() {
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        SYMTAB[i] = NULL;
    }
    textRecordHead = NULL;
    textRecordTail = NULL;
}

// Parse a line into label, opcode, operand
void parseLine(char *line, char *label, char *opcode, char *operand) {
    label[0] = opcode[0] = operand[0] = '\0';
    
    char temp[MAX_LINE_LEN];
    strcpy(temp, line);
    
    if (temp[0] == ' ' || temp[0] == '\t') {
        // No label
        char *token = strtok(temp, " \t\n");
        if (token) strcpy(opcode, token);
        
        token = strtok(NULL, "\n");
        if (token) {
            while (*token == ' ' || *token == '\t') token++;
            strcpy(operand, token);
            trim(operand);
        }
    } else {
        // Has label
        char *token = strtok(temp, " \t\n");
        if (token) strcpy(label, token);
        
        token = strtok(NULL, " \t\n");
        if (token) strcpy(opcode, token);
        
        token = strtok(NULL, "\n");
        if (token) {
            while (*token == ' ' || *token == '\t') token++;
            strcpy(operand, token);
            trim(operand);
        }
    }
}

// Calculate length for Byte directive
int calculateByteLength(char *operand) {
    if (operand[0] == 'C') {
        return strlen(operand) - 3;
    } else if (operand[0] == 'X') {
        return (strlen(operand) - 3 + 1) / 2;
    }
    return 0;
}

// Patch the forward references
void patchForwardReferences(FILE *output) {
    printf("\n\x1b[34mINFO:\x1b[0m Patching Forward References...\n");
    
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        SymtabEntry *ptr = SYMTAB[i];
        while (ptr != NULL) {
            if (ptr->defined && ptr->forwardRefs != NULL) {
                // This symbol has forward references that need patching
                ForwardRef *ref = ptr->forwardRefs;
                while (ref != NULL) {
                    // Find the text record containing this address
                    TextRecord *tr = textRecordHead;
                    while (tr != NULL) {
                        unsigned int recStart = tr->startAddr;
                        unsigned int recEnd = recStart + tr->length;
                        
                        if (ref->address >= recStart && ref->address < recEnd) {
                            // Calculate position in object code string
                            int offset = (ref->address - recStart) * 2;  // 2 hex chars per byte
                            
                            // Calculate the operand address with indexed bit if needed
                            unsigned int operandAddr = ptr->address;
                            if (ref->indexed) {
                                operandAddr |= 0x8000;
                            }
                            
                            // Patch the address (last 4 hex digits of the instruction)
                            char addrStr[5];
                            sprintf(addrStr, "%04X", operandAddr);
                            
                            // Replace in object code (skip opcode, patch address)
                            tr->objectCode[offset + 2] = addrStr[0];
                            tr->objectCode[offset + 3] = addrStr[1];
                            tr->objectCode[offset + 4] = addrStr[2];
                            tr->objectCode[offset + 5] = addrStr[3];
                            
                            printf("\x1b[33mSYMTAB:\x1b[0m Patched %s at address %04X with value %04X\n", 
                                   ref->symbol, ref->address, operandAddr);
                            break;
                        }
                        tr = tr->next;
                    }
                    ref = ref->next;
                }
            }
            ptr = ptr->next;
        }
    }
}

// Write text record in the output file
void writeTextRecords(FILE *output) {
    TextRecord *tr = textRecordHead;
    while (tr != NULL) {
        fprintf(output, "T%06X%02X%s\n", tr->startAddr, tr->length, tr->objectCode);
        printf("T%06X%02X%s\n", tr->startAddr, tr->length, tr->objectCode);
        tr = tr->next;
    }
}


void onePassAssemble(FILE *input, FILE *output) {
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
    
    // Check if START directive
    if (strcmp(opcode, "START") == 0) {
        strcpy(PROGRAM_NAME, label);
        START_ADDRESS = (unsigned int)strtol(operand, NULL, 16);
        LOCCTR = START_ADDRESS;
        
        printf("%04X  %s  %s  %s\n", LOCCTR, label, opcode, operand);
        
        // Read next line
        if (!fgets(line, MAX_LINE_LEN, input)) return;
        lineNumber++;
        parseLine(line, label, opcode, operand);
    } else {
        LOCCTR = 0;
    }
    
    // Process all lines
    while (strcmp(opcode, "END") != 0) {
        unsigned int currentLOCCTR = LOCCTR;
        
        if (!isComment(line)) {
            // If there's a label, insert into SYMTAB
            if (strlen(label) > 0) {
                insertSYMTAB(label, LOCCTR, 1);
                printf("\x1b[33mSYMTAB:\x1b[0m Defined %s at address %04X\n", label, LOCCTR);
                
                // Check if this symbol has forward references to patch
                SymtabEntry *entry = searchSYMTABEntry(label);
                if (entry && entry->forwardRefs != NULL) {
                    printf("%s has forward references to be patched later\n", label);
                }
            }
            
            char objectCode[20] = "";
            int objCodeLength = 0;
            
            // Search OPTAB for opcode
            OptabEntry *entry = searchOPTAB(opcode);
            if (entry != NULL) {
                // Machine instruction
                unsigned int operandAddr = 0;
                int indexed = 0;
                int isForwardRef = 0;
                
                // Parse operand
                char operandCopy[32];
                strcpy(operandCopy, operand);
                
                // Check for indexed addressing
                char *comma = strchr(operandCopy, ',');
                if (comma != NULL) {
                    *comma = '\0';
                    indexed = 1;
                }
                
                // Search symbol table for operand
                if (strlen(operandCopy) > 0) {
                    int symAddr = searchSYMTAB(operandCopy);
                    if (symAddr != -1) {
                        // Symbol is defined
                        operandAddr = symAddr;
                        if (indexed) {
                            operandAddr |= 0x8000;
                        }
                    } else {
                        // Forward reference - add to table
                        printf("\x1b[34mINFO:\x1b[0m Forward reference: %s\n", operandCopy);
                        addForwardRef(operandCopy, currentLOCCTR, indexed);
                        operandAddr = 0;  // Placeholder
                        isForwardRef = 1;
                    }
                }
                
                // Generate object code
                sprintf(objectCode, "%02X%04X", entry->opcode, operandAddr);
                objCodeLength = 3;
                
                LOCCTR += entry->length;
            } 
            else if (strcmp(opcode, "BYTE") == 0) {
                if (operand[0] == 'C') {
                    for (int i = 2; operand[i] != '\''; i++) {
                        sprintf(objectCode + strlen(objectCode), "%02X", (unsigned char)operand[i]);
                    }
                    objCodeLength = calculateByteLength(operand);
                } else if (operand[0] == 'X') {
                    int i = 2;
                    while (operand[i] != '\'') {
                        objectCode[i-2] = operand[i];
                        i++;
                    }
                    objectCode[i-2] = '\0';
                    objCodeLength = calculateByteLength(operand);
                }
                LOCCTR += objCodeLength;
            } 
            else if (strcmp(opcode, "WORD") == 0) {
                int value = atoi(operand);
                sprintf(objectCode, "%06X", value);
                objCodeLength = 3;
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
            else {
                printf("\x1b[31mERROR:\x1b[0m Invalid operation code %s at line %d\n", opcode, lineNumber);
            }
            
            // Add object code to text records
            if (objCodeLength > 0) {
                addToTextRecord(currentLOCCTR, objectCode, objCodeLength);
            }
            
            printf("%04X  %-8s%-8s%s", currentLOCCTR, label, opcode, operand);
            if (objCodeLength > 0) {
                printf("  [%s]", objectCode);
            }
            printf("\n");
        }
        
        // Read next line
        if (!fgets(line, MAX_LINE_LEN, input)) break;
        lineNumber++;
        parseLine(line, label, opcode, operand);
    }
    
    printf("%04X  %s  %s  %s\n", LOCCTR, label, opcode, operand);
    
    // Calculate program length
    PROGRAM_LENGTH = LOCCTR - START_ADDRESS;
    
    // Patch forward references
    patchForwardReferences(output);
    
    // Write Header record
    fprintf(output, "H%-6s%06X%06X\n", PROGRAM_NAME, START_ADDRESS, PROGRAM_LENGTH);
    printf("\n\x1b[32mRESULT:\x1b[0m H%-6s%06X%06X\n", PROGRAM_NAME, START_ADDRESS, PROGRAM_LENGTH);
    
    // Write all text records
    writeTextRecords(output);
    
    // Write End record
    char endOperand[32];
    strcpy(endOperand, operand);
    if (strlen(endOperand) > 0) {
        int startAddr = searchSYMTAB(endOperand);
        if (startAddr != -1) {
            fprintf(output, "E%06X\n", startAddr);
            printf("\x1b[32mRESULT:\x1b[0m E%06X\n", startAddr);
        } else {
            fprintf(output, "E%06X\n", START_ADDRESS);
            printf("\x1b[32mRESULT:\x1b[0m E%06X\n", START_ADDRESS);
        }
    } else {
        fprintf(output, "E%06X\n", START_ADDRESS);
        printf("\x1b[32mRESULT:\x1b[0m E%06X\n", START_ADDRESS);
    }
    
    printf("\n\x1b[32mRESULT:\x1b[0m One-Pass Assembly Complete!\n");
}