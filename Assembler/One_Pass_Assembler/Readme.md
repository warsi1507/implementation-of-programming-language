# One-Pass Assembler Implementation in C

This is a complete implementation of a **one-pass assembler** for SIC (Simplified Instructional Computer) assembly language with forward reference handling and backpatching.

## Features

- **Single Pass Assembly**: Assembles the entire program in one pass
- **Forward Reference Handling**: Automatically handles forward references using backpatching
- **Symbol Table with Forward References**: Hash table tracks both defined symbols and forward references
- **Dynamic Object Code Generation**: Generates object code on-the-fly with placeholder values
- **Backpatching**: Patches all forward references after symbol definitions are encountered
- **Same Output as Two-Pass**: Produces identical object code to the two-pass assembler

## How It Works

Unlike the two-pass assembler that requires two complete passes through the source code, the one-pass assembler:

1. **Processes each line once** while maintaining:
   - Symbol table with definition status
   - List of forward references for each undefined symbol
   - Object code with placeholders for undefined symbols

2. **When a symbol is referenced before definition**:
   - Generates object code with a placeholder address (0000)
   - Records the location and context of the forward reference
   - Continues assembly

3. **When a symbol is defined**:
   - Updates symbol table with the actual address
   - Notes that this symbol has pending forward references

4. **After processing all lines**:
   - Scans all forward references
   - Patches the object code with correct addresses
   - Writes the final object program

## Advantages of One-Pass Assembly

✅ **Faster**: Only reads the source file once  
✅ **Less Memory**: Doesn't need intermediate file  
✅ **Simpler I/O**: No need to reopen and reread files  
✅ **Still Correct**: Produces identical output via backpatching  

## Disadvantages

❌ **More Complex Logic**: Forward reference tracking adds complexity  
❌ **Memory Usage**: Must store all text records and forward references in memory  
❌ **Limited to Small Programs**: Memory constraints for very large programs  

## File Structure

- **assembler.h**: Header file containing:
  - OPTAB (Operation Code Table)
  - SYMTAB (Symbol Table with forward reference support)
  - ForwardRef structure for tracking undefined symbols
  - TextRecord structure for buffering object code
  - Utility functions

- **assembler.c**: Main implementation:
  - Single-pass assembly algorithm
  - Forward reference tracking
  - Backpatching logic
  - Object code generation

## Data Structures

### Symbol Table Entry
```c
typedef struct SymtabEntry {
    char label[32];           // Symbol name
    int defined;              // 1 if defined, 0 if forward ref
    unsigned int address;     // Memory address
    ForwardRef *forwardRefs;  // List of forward references
    struct SymtabEntry* next; // Hash table chaining
} SymtabEntry;
```

### Forward Reference Entry
```c
typedef struct ForwardRef {
    unsigned int address;     // Where the reference occurs
    char symbol[32];          // Symbol being referenced
    int indexed;              // Indexed addressing flag
    struct ForwardRef *next;  // Next reference
} ForwardRef;
```

### Text Record Buffer
```c
typedef struct TextRecord {
    unsigned int startAddr;   // Starting address
    char objectCode[70];      // Object code string
    int length;               // Length in bytes
    struct TextRecord *next;  // Next record
} TextRecord;
```

## Compilation

```bash
gcc -o assembler assembler.c -Wall
```

## Usage

```bash
./assembler <input_file> <output_file>
```

### Example:
```bash
./assembler sample_input.txt output.txt
```

## Input Format

Same as the two-pass assembler - standard SIC assembly language:

```assembly
LABEL   OPCODE  OPERAND
```

## Output Format

Identical to two-pass assembler output:
- Header record (H)
- Text records (T)
- End record (E)

## Example Execution

```
=== Starting One-Pass Assembly ===
1000  COPY  START  1000
SYMTAB: Defined FIRST at address 1000
  Forward reference: RETADR
1000  FIRST   STL     RETADR  [140000]    <- Placeholder
...
SYMTAB: Defined RETADR at address 1033
  -> Symbol has forward references to be patched later

=== Patching Forward References ===
Patched RETADR at address 1000 with value 1033  <- Backpatching!
...

HCOPY  00100000107A
T0010001E1410334820390010362810303010154820613C1003...
E001000
```

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

### Main Assembly Loop
```
1. Read line
2. If label exists:
   - Add to SYMTAB as defined
   - Check for pending forward references
3. Generate object code:
   - If operand is defined: use its address
   - If operand is undefined: use 0000 and add forward reference
4. Add object code to text records
5. Repeat until END
```

### Backpatching Phase
```
1. For each symbol in SYMTAB:
   - If defined and has forward references:
     - For each forward reference:
       - Find text record containing reference address
       - Calculate position in object code
       - Replace placeholder with actual address
```

## Supported Instructions

Same 23 instructions as two-pass assembler:
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
- Duplicate symbol definitions
- Invalid operation codes
- Memory allocation failures
- Undefined symbols (after backpatching)

## Limitations

- Must hold all text records in memory
- Forward references must be resolved by end of program
- No support for external references (linking)
- Limited to SIC architecture (no SIC/XE)

## Technical Details

### Forward Reference Resolution
When a symbol is used before it's defined:
1. Object code generated with address 0000
2. Forward reference recorded with:
   - Location of the reference
   - Symbol name
   - Indexing flag
3. When symbol is later defined:
   - Address recorded in SYMTAB
   - Forward reference list preserved
4. After all lines processed:
   - Each forward reference located in text records
   - Correct address patched in

### Text Record Management
- Text records accumulated in linked list
- Maximum 30 bytes per record
- New record started when:
  - Current record would exceed 30 bytes
  - Address gap detected (RESW/RESB)
- All records preserved for backpatching