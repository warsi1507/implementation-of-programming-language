# Two-Pass Assembler Implementation in C

This is a complete implementation of a **two-pass assembler** for SIC (Simplified Instructional Computer) assembly language. It processes the assembly source code in two distinct passes to resolve forward references and generate the final machine object code.

## Features

- **Two-Pass Assembly**: Clearly separated Pass-1 and Pass-2.
- **Symbol Table (SYMTAB)**: Utilizes a hash table to efficiently store and look up symbol addresses.
- **Literal Table (LITTAB)**: Includes support for literals and assigns addresses to them.
- **Intermediate File Generation**: Creates a structured intermediate file containing location counters and parsed instructions during Pass-1.
- **Object Code Generation**: Produces properly formatted Header, Text, and End records.

## How It Works

The two-pass assembler divides the compilation process into two distinct phases:

1. **Pass-1 (Define Symbols)**:
   - Reads the input assembly program line by line.
   - Maintains the Location Counter (`LOCCTR`).
   - If a label is found, it inserts the label and its assigned `LOCCTR` value into the Symbol Table (`SYMTAB`).
   - Determines the length of each instruction (via `OPTAB`) and directives (`BYTE`, `WORD`, `RESW`, `RESB`).
   - Writes the source line along with its calculated location to an intermediate file.
   - Identifies the total program length.

2. **Pass-2 (Assemble Instructions)**:
   - Reads the intermediate file line by line.
   - Translates operation codes to their machine language equivalent using `OPTAB`.
   - Looks up operand addresses in the `SYMTAB`.
   - Calculates the byte length and generates object code for data directives (`BYTE`, `WORD`).
   - Accumulates Object Code and writes Header (`H`), Text (`T`), and End (`E`) records to the output file.

## Advantages of Two-Pass Assembly

✅ **Simpler Logic**: Forward references are naturally handled because all symbols are defined in Pass-1 before any code generation occurs in Pass-2.  
✅ **Memory Efficiency**: Does not require holding all text records in memory simultaneously. Records are written directly to the output file as they are generated.  
✅ **No Backpatching Required**: Does not need to retrospectively patch forward references since `SYMTAB` is fully populated.  

## Disadvantages

❌ **Slower Execution**: Requires reading the source program twice (once from the original file, once from the intermediate file).  
❌ **File I/O Overhead**: Involves reading from and writing to disk multiple times, including creating an intermediate file.  

## File Structure

- **assembler.h**: Header file containing:
  - `OPTAB` (Operation Code Table)
  - `SYMTAB` (Symbol Table implemented as a Hash Table)
  - `LITTAB` (Literal Table)
  - Utility and hashing functions.

- **main.c**: Main implementation:
  - `pass1()` for Symbol Table construction and intermediate file generation.
  - `pass2()` for Object Code generation.
  - Main loop and parsing logic (`parseLine`).

## Data Structures

### Symbol Table Entry
```c
typedef struct SymtabEntry {
    char label[32];       // Symbol name
    int defined;          // 1 if defined
    unsigned int address; // Assigned memory address
    struct SymtabEntry* next; // Hash table chaining
} SymtabEntry;
```

### Literal Table Entry (Future parsing support)
```c
typedef struct LittabEntry {
    char literal[32];        // Literal name
    char value[32];          // Hexadecimal value
    int length;              // Length in bytes
    int address;             // Assigned memory address
    int addressAssigned;     // 0 if pending, 1 if assigned
    struct LittabEntry *next;
} LittabEntry;
```

## Compilation

```bash
gcc -o assembler main.c -Wall
```

## Usage

```bash
./assembler <input_file> <intermediate_file> <output_file>
```

### Example:
```bash
./assembler sample_input.txt intermediate.txt output.txt
```

## Input Format

Standard SIC assembly language format:
```assembly
LABEL   OPCODE  OPERAND
```

## Output Format

- **Header record (H)**: Contains program name, starting address, and program length.
- **Text records (T)**: Contains the starting address, length of object code in this record, and the object code itself (maximum 30 bytes per record).
- **End record (E)**: Marks the end of the program and specifies the starting execution address.

## Sample Input

```assembly
COPY    START   1000
FIRST   STL     RETADR
CLOOP   JSUB    RDREC
        LDA     LENGTH
        COMP    ZERO
        JEQ     ENDFIL
        JSUB    WRREC
        J       CLOOP
ENDFIL  LDA     EOF
        STA     BUFFER
        LDA     THREE
        STA     LENGTH
        JSUB    WRREC
        LDL     RETADR
        RSUB
EOF     BYTE    C'EOF'
THREE   WORD    3
ZERO    WORD    0
RETADR  RESW    1
LENGTH  RESW    1
BUFFER  RESB    4096
.
.       SUBROUTINE TO READ RECORD INTO BUFFER
.
RDREC   LDX     ZERO
        LDA     ZERO
RLOOP   TD      INPUT
        JEQ     RLOOP
        RD      INPUT
        COMP    ZERO
        JEQ     EXIT
        STCH    BUFFER,X
        TIX     MAXLEN
        JLT     RLOOP
EXIT    STX     LENGTH
        RSUB
INPUT   BYTE    X'F1'
MAXLEN  WORD    4096
.
.       SUBROUTINE TO WRITE RECORD FROM BUFFER
.
WRREC   LDX     ZERO
WLOOP   TD      OUTPUT
        JEQ     WLOOP
        LDCH    BUFFER,X
        WD      OUTPUT
        TIX     LENGTH
        JLT     WLOOP
        RSUB
OUTPUT  BYTE    X'05'
        END     FIRST
```

## Sample Intermediate

```text
1000 COPY    START   1000
1000 FIRST   STL     RETADR
1003 CLOOP   JSUB    RDREC
1006         LDA     LENGTH
1009         COMP    ZERO
100C         JEQ     ENDFIL
100F         JSUB    WRREC
1012         J       CLOOP
1015 ENDFIL  LDA     EOF
1018         STA     BUFFER
101B         LDA     THREE
101E         STA     LENGTH
1021         JSUB    WRREC
1024         LDL     RETADR
1027         RSUB
102A EOF     BYTE    C'EOF'
102D THREE   WORD    3
1030 ZERO    WORD    0
1033 RETADR  RESW    1
1036 LENGTH  RESW    1
1039 BUFFER  RESB    4096
2039 RDREC   LDX     ZERO
203C         LDA     ZERO
203F RLOOP   TD      INPUT
2042         JEQ     RLOOP
2045         RD      INPUT
2048         COMP    ZERO
204B         JEQ     EXIT
204E         STCH    BUFFER,X
2051         TIX     MAXLEN
2054         JLT     RLOOP
2057 EXIT    STX     LENGTH
205A         RSUB
205D INPUT   BYTE    X'F1'
205E MAXLEN  WORD    4096
2061 WRREC   LDX     ZERO
2064 WLOOP   TD      OUTPUT
2067         JEQ     WLOOP
206A         LDCH    BUFFER,X
206D         WD      OUTPUT
2070         TIX     LENGTH
2073         JLT     WLOOP
2076         RSUB
2079 OUTPUT  BYTE    X'05'
207A         END     FIRST
```

## Sample Output

```text
HCOPY  00100000107A
T0010001E1410334820390010362810303010154820613C100300102A0C103900102D
T00101E150C10364820610810334C0000454F46000003000000
T0020391E041030001030E0205D30203FD8205D2810303020575490392C205E38203F
T0020571C1010364C0000F1001000041030E02079302064509039DC20792C1036
T002073073820644C000005
E001000
```

## Algorithm Overview

### Pass 1
1. Read first line. If `START`, save program name, address and initialize `LOCCTR`.
2. Loop over lines until `END` is reached.
3. If line is not a comment and has a label, add label and `LOCCTR` to `SYMTAB`.
4. Lookup `OPCODE` in `OPTAB`. If found, add length to `LOCCTR`.
5. If directive (`WORD`, `RESW`, `RESB`, `BYTE`), add appropriate length to `LOCCTR`.
6. Write the line and its `LOCCTR` to the intermediate file.

### Pass 2
1. Read first line from intermediate file. Write Header Record if `START`.
2. Loop over lines until `END` is reached.
3. If not a comment, look up `OPCODE` in `OPTAB`.
4. If found, look up operand in `SYMTAB` to get its address.
5. Create machine object code based on instruction or directive.
6. Accumulate object code into Text Records (up to 30 bytes). Write Text Record to output and start a new one when full or when address jumps.
7. Write End Record to output.

## Supported Instructions

23 SIC instructions supported:
LDA, LDX, LDL, STA, STX, STL, LDCH, STCH, ADD, SUB, MUL, DIV, COMP, J, JLT, JEQ, JGT, JSUB, RSUB, TIX, TD, RD, WD

## Supported Directives

- **START**: Define program name and starting address
- **END**: Mark end of program
- **BYTE**: Define byte constant (C'...' or X'...')
- **WORD**: Define word constant (3 bytes)
- **RESW**: Reserve words
- **RESB**: Reserve bytes

## Error Handling

The assembler detects:
- Duplicate symbol definitions.
- Invalid operation codes.
- Undefined symbols.
- Memory allocation failures.

## Limitations

- No support for external references (linking).
- Limited strictly to SIC architecture (no SIC/XE features like formatting or extended addressing).
