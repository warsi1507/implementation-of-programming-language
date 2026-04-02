#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/*************************************
 * Global Variables & Constants
 *************************************/
#define SYMTAB_SIZE 1031
#define MAX_LINE_LEN 256
#define MAX_FORWARD_REFS 100

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

// Structure to hold forward reference information
typedef struct ForwardRef {
    unsigned int address;     // Address where the reference occurs
    char symbol[32];          // Symbol being referenced
    int indexed;              // 1 if indexed addressing, 0 otherwise
    struct ForwardRef *next;
} ForwardRef ;

/*************************************
 * SYMTAB (SYMBOL TABLE)
 *************************************/

// Structure for a single SYMTAB entry with forward reference support
typedef struct SymtabEntry {
    char label[32];           // name of the symbol
    int defined;              // 1 if defined, 0 if forward reference
    unsigned int address;     // assigned memory address for the symbol
    ForwardRef *forwardRefs;  // Linked list of forward references
    struct SymtabEntry* next;
} SymtabEntry;

// The SYMTAB as a Hash Table
static SymtabEntry *SYMTAB[SYMTAB_SIZE];

// Hash Function
static int hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % SYMTAB_SIZE;
}

// Function to insert or update a symbol
static void insertSYMTAB(char *label, unsigned int address, int defined) {
    int index = hash(label);
    
    // Check if symbol already exists
    SymtabEntry *ptr = SYMTAB[index];
    while (ptr != NULL) {
        if (strcmp(ptr->label, label) == 0) {
            if (ptr->defined && defined) {
                printf("Error: Duplicate symbol '%s'\n", label);
                return;
            }
            // Update existing entry
            if (defined) {
                ptr->address = address;
                ptr->defined = 1;
            }
            return;
        }
        ptr = ptr->next;
    }
    
    // Create new entry
    SymtabEntry *newEntry = (SymtabEntry *)malloc(sizeof(SymtabEntry));
    if (!newEntry) {
        printf("Error: Memory allocation failed\n");
        exit(1);
    }
    
    strcpy(newEntry->label, label);
    newEntry->address = address;
    newEntry->defined = defined;
    newEntry->forwardRefs = NULL;

    newEntry->next = SYMTAB[index];
    SYMTAB[index] = newEntry;
}

// Function to add forward reference to a symbol
static void addForwardRef(char *label, unsigned int refAddress, int indexed) {
    int index = hash(label);
    SymtabEntry *ptr = SYMTAB[index];
    
    // Find or create symbol entry
    while (ptr != NULL) {
        if (strcmp(ptr->label, label) == 0) {
            break;
        }
        ptr = ptr->next;
    }
    
    if (ptr == NULL) {
        // Symbol not in table, create undefined entry
        insertSYMTAB(label, 0, 0);
        ptr = SYMTAB[index];
        while (ptr != NULL && strcmp(ptr->label, label) != 0) {
            ptr = ptr->next;
        }
    }
    
    // Add forward reference
    ForwardRef *newRef = (ForwardRef *)malloc(sizeof(ForwardRef));
    if (!newRef) {
        printf("Error: Memory allocation failed\n");
        exit(1);
    }
    
    newRef->address = refAddress;
    strcpy(newRef->symbol, label);
    newRef->indexed = indexed;

    newRef->next = ptr->forwardRefs;
    ptr->forwardRefs = newRef;
}

// Function to search for a symbol
static SymtabEntry* searchSYMTABEntry(char *label) {
    int index = hash(label);
    SymtabEntry *ptr = SYMTAB[index];
    
    while (ptr != NULL) {
        if (strcmp(ptr->label, label) == 0) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

// Function to get symbol address
// Returns: address if found, -1 if not found
static int searchSYMTAB(char *label) {
    SymtabEntry *entry = searchSYMTABEntry(label);
    if (entry != NULL && entry->defined) {
        return entry->address;
    }
    return -1;
}

/*************************************
 * TEXT RECORD BUFFER
 *************************************/

typedef struct TextRecord {
    unsigned int startAddr;
    char objectCode[70];
    int length;
    struct TextRecord *next;
} TextRecord;

static TextRecord *textRecordHead = NULL;
static TextRecord *textRecordTail = NULL;

// Function to add object code to text records for efficient back-patching
static void addToTextRecord(unsigned int address, char *objCode, int length) {
    // Check if we can append to last record
    if (textRecordTail != NULL 
        && textRecordTail->length + length <= 30 
        && address == textRecordTail->startAddr + textRecordTail->length)
    {
        // Append to existing record
        strcat(textRecordTail->objectCode, objCode);
        textRecordTail->length += length;
    } else {
        // Create new record
        TextRecord *newRecord = (TextRecord *)malloc(sizeof(TextRecord));
        if (!newRecord) {
            printf("Error: Memory allocation failed\n");
            exit(1);
        }
        
        newRecord->startAddr = address;
        strcpy(newRecord->objectCode, objCode);
        newRecord->length = length;
        newRecord->next = NULL;
        
        if (textRecordHead == NULL) {
            textRecordHead = textRecordTail = newRecord;
        } else {
            textRecordTail->next = newRecord;
            textRecordTail = newRecord;
        }
    }
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