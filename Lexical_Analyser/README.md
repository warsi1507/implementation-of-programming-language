# Lexical Analyser Implementation in C (Flex)

This is a complete lexical analyser (scanner) for the nanoC programming language, generated using `flex`. It tokenizes source code into a stream of categorized tokens and maintains a symbol table for identifiers.

## Features

- **Lexical Tokenization**: Accurately recognizes keywords, identifiers, constants (integer, floating, and character), string literals, and punctuators.
- **Symbol Table Generation**: Builds a symbol table recording all unique identifiers and their line of first appearance.
- **Maximal Munch Rule**: Correctly resolves overlapping token patterns (e.g. operators like `++`, `<<=`).
- **Comprehensive Error Checking**: Detects and reports invalid symbols, unterminated strings, unterminated character constants, and unterminated multi-line comments.
- **Line Tracking**: Tracks line numbers flawlessly across expressions and multi-line comments.

## How It Works

The lexical analyser breaks the compilation process down into identifying atomic language elements:

1. **Token Processing**:
   - The lexer reads characters from the input stream.
   - Using regular expressions defined in `main.l`, it matches the longest possible sequence of characters (maximal munch).
   - Once matched, it categorizes the matching string (e.g. `KEYWORD`, `IDENTIFIER`, `INT_CONST`).
   - Line numbers are tracked using flex's state and by analyzing newline characters during comment digestion.

2. **Symbol Table Maintenance**:
   - Every time an `IDENTIFIER` matches, the lexer performs a lookup in its internal Symbol Table (an array-based list limitation of 2048 entries).
   - If the identifier is new, it records its name and the `first_line` where it appeared.

3. **Output Generation**:
   - Two text files are continuously updated during execution: the Token Stream (`lexer_token.txt`) and the Symbol Table (`lexer_st.txt`).

## File Structure

- **main.l**: The Flex specification file containing:
  - C Declarations & Helper functions (`insert_symbol`, `lookup_symbol`).
  - Regular Expression Definitions for nanoC tokens.
  - Flex Rules associating patterns with C actions to write tokens.
  - Required C `main()` wrapper that initializes streams and begins validation via `yylex()`.
- **Makefile**: Build rules for compiling the flex project.
- **test.nc**: A comprehensive test script covering edge cases, syntax, and known token structures.

## Data Structures

### Symbol Table Entry
```c
typedef struct {
    char name[MAX_ID_LEN];  // Identifier name
    int  first_line;        // Line number first seen
} Symbol;

// Global Table
Symbol sym_table[MAX_SYMBOLS];
```

## Compilation

```bash
make
```
This generates the C lexer script `lex.yy.c` and compiles it into the executable `lexer`.

## Usage

```bash
./lexer <input_file.nc>
```

### Example:
```bash
./lexer test.nc
```
(Alternatively, simply run `make run` to test the lexer against the included `test.nc`).

## Output Format

- **lexer_token.txt**: Lists one token per line in the format: `Line <n>: <TOKEN_TYPE>(<value>)`.
- **lexer_st.txt**: A structured tabular layout detailing Index, Identifier strings, and First Detection Line.

## Lexical Rules Implemented

- **Keywords**: Matches 20 specific reserved standard types and branches (`int`, `float`, `if`, etc.). Processed prior to identifiers.
- **Identifiers**: Alpha-numeric sequences starting with characters or an underscore (`[a-zA-Z_][a-zA-Z_0-9]*`).
- **Constants**: Evaluates `INT_CONST` (numbers or bare `0`), `FLOAT_CONST` (with `.`), and `CHAR_CONST` resolving character escape codes (`\n`, `\t`...).
- **String Literals**: Defined between quotes with embedded escape sequences recognized properly.
- **Punctuators**: Rejects strings inside logic blocks that don't match 45 specific combinations (`->`, `==`, `*=`, etc.).
- **Comments**: Correctly ignores `//` single-line comments and computes space inside `/* ... */` to ensure `line_num` maintains synchronization.

## Error Handling

The Lexer emits errors specifically to `stderr` and the Token Log when evaluating exceptions:
- **Unknown Tokens**: Captures unrecognizable inputs `.` and throws `Unknown token '@'`.
- **Unterminated Strings/Chars**: Errors upon reaching trailing newlines if literal quotes haven't been closed.
- **Unterminated Multi-Line Comments**: Reaches `EOF` and reports incomplete blocks.
- **Table Constraints**: Maximum symbol definitions locked at 2048 entries.
