#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/*************************************
 * Global Variables & Constants
 *************************************/
#define SYMTAB_SIZE 1031
#define LITTAB_SIZE 1031
#define MAX_LINE_LEN 256

// The Location Counter (LOCCTR)
static unsigned int LOCCTR = 0;
static unsigned int START_ADDRESS = 0;
static unsigned int PROGRAM_LENGTH = 0;
static char PROGRAM_NAME[10] = "";

/*************************************
 * OPTAB (OPERATION CODE TABLE)
 *************************************/

// Structure for a single OPTAB entry
typedef struct {
    char mnemonic[10];    // name of the instruction
    unsigned char opcode; // opcode in hexadecimal for the instruction
    int length;           // length of the instruction
} OptabEntry ;

// The static OPTAB (from opcodes.txt)
static OptabEntry OPTAB[] = {
    {"LDA",  0x00, 3}, {"LDX",  0x04, 3}, {"LDL",  0x08, 3}, {"STA",  0x0C, 3}, {"STX",  0x10, 3}, {"STL",  0x14, 3},
    {"LDCH", 0x50, 3}, {"STCH", 0x54, 3}, {"ADD",  0x18, 3}, {"SUB",  0x1C, 3}, {"MUL",  0x20, 3}, {"DIV",  0x24, 3},
    {"COMP", 0x28, 3}, {"J",    0x3C, 3}, {"JLT",  0x38, 3}, {"JEQ",  0x30, 3}, {"JGT",  0x34, 3}, {"JSUB", 0x48, 3},
    {"RSUB", 0x4C, 3}, {"TIX",  0x2C, 3}, {"TD",   0xE0, 3}, {"RD",   0xD8, 3}, {"WD",   0xDC, 3}
};

// Function to search for an Opcode
// Returns: Pointer to the entry if found, NULL otherwise
static OptabEntry* searchOPTAB(char* mnemonic) {
    int totalOpcodes = sizeof(OPTAB) / sizeof(OptabEntry);
    for (int i = 0; i < totalOpcodes; i++) {
        if(strcmp(mnemonic, OPTAB[i].mnemonic) == 0) {
            return &OPTAB[i];
        }
    }
    return NULL;
}

/*************************************
 * SYMTAB (SYMBOL TABLE)
 *************************************/

// Structure for a single SYMTAB entry (in form of Linked-List node)
typedef struct SymtabEntry {
    char label[32];       // name of the symbol
    int defined;          // 1 if defined, 0 if forward ref
    unsigned int address; // assigned memory address for the symbol
    struct SymtabEntry* next;
} SymtabEntry ;

// The SYMTAB as a Hash Table
static SymtabEntry *SYMTAB[SYMTAB_SIZE];

// Hash Function that converts label string to table index
static int hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;  // hash := hash*33 + c
    return hash % SYMTAB_SIZE;
}

// Function to insert a label into SYMTAB (Pass 1)
static void insertSYMTAB(char *label, int address) {
    int index = hash(label);

    // Check if symbol already exists
    SymtabEntry *ptr = SYMTAB[index];
    while (ptr != NULL) {
        if (strcmp(ptr->label, label) == 0) {
            printf("Error: Duplicate symbol '%s'\n", label);
            return;
        }
        ptr = ptr->next;
    }

    // allocate memory for new node
    SymtabEntry *newEntry = (SymtabEntry *)malloc(sizeof(SymtabEntry));
    if (!newEntry){
        printf("Error: Memory allocation failed for SYMTAB entry\n");
        exit(1);
    }

    strcpy(newEntry->label, label);
    newEntry->address = address;
    newEntry->defined = 1;

    // Insert at head of linked list
    newEntry->next = SYMTAB[index];
    SYMTAB[index] = newEntry;
}

// Function to search for a label in SYMTAB (Pass 2)
// Returns: address if found, -1 if not found
static unsigned int searchSYMTAB(char *label) {
    int index = hash(label);
    SymtabEntry *ptr = SYMTAB[index];
    
    while (ptr != NULL) {
        if (strcmp(ptr->label, label) == 0) {
            return ptr->address;
        }
        ptr = ptr->next;
    }
    return -1; // Symbol not found
}

/*  =========== LITERAL SUPPORT FOR FUTURE ===========  */
/*************************************
 * LITTAB (LITERAL TABLE)
 *************************************/

typedef struct LittabEntry {
    char literal[32];        // name of literal
    char value[32];          // hex-value
    int length;              // length in bytes
    int address;             // assigned memory address
    int addressAssigned;     // 0 if pending, 1 if assigned
    struct LittabEntry *next;
} LittabEntry;

// The Hash Table Array
static LittabEntry *LITTAB[LITTAB_SIZE];

// Function to Add a Literal (Pass 1)
static void insertLiteral(char *literal, char *value, int length) {
    int index = hash(literal);
    
    // Check for duplicate
    LittabEntry *ptr = LITTAB[index];
    while (ptr != NULL) {
        if (strcmp(ptr->literal, literal) == 0) {
            return;
        }
        ptr = ptr->next;
    }

    // Insert new literal
    LittabEntry *newEntry = (LittabEntry *)malloc(sizeof(LittabEntry));
    if (!newEntry) { 
        printf("Memory Error\n"); 
        exit(1); 
    }
    
    strcpy(newEntry->literal, literal);
    strcpy(newEntry->value, value);
    newEntry->length = length;
    newEntry->addressAssigned = 0;
    
    // Insert at head of the bucket
    newEntry->next = LITTAB[index];
    LITTAB[index] = newEntry;
}

// Function to handle LTORG or End of Pass 1
static void processLiterals() {
    for (int i = 0; i < LITTAB_SIZE; i++) {
        LittabEntry *ptr = LITTAB[i];
        while (ptr != NULL) {
            if (ptr->addressAssigned == 0) {
                // Assign address
                ptr->address = LOCCTR;
                ptr->addressAssigned = 1;
                
                printf("LITTAB: Assigned %s to Address %X\n", ptr->literal, LOCCTR);
                
                // Update Location Counter
                LOCCTR += ptr->length;
            }
            ptr = ptr->next;
        }
    }
}

// Function to retrieve Address (Pass 2)
// Returns: address if found, -1 if not found
static int getLiteralAddress(char *literal) {
    int index = hash(literal);
    LittabEntry *ptr = LITTAB[index];
    
    while (ptr != NULL) {
        if (strcmp(ptr->literal, literal) == 0) {
            return ptr->address;
        }
        ptr = ptr->next;
    }
    return -1; // Not found
}

/*************************************
 * Utility Functions
 *************************************/

// Function to check if a line is a comment
static int isComment(char *line) {
    // Skip leading whitespace
    while (*line == ' ' || *line == '\t') line++;
    return (*line == '.');
}

// Function to trim whitespace from both ends
static void trim(char *str) {
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    // All spaces?
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
}