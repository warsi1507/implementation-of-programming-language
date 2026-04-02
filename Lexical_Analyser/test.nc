/* =========================================================
   test.nc
   Combined Test file for nanoC lexer.
   Exercises all lexical rules, edge cases, and errors.
   ========================================================= */

/* ---- 1. Keywords ---- */
void short int float double long Bool
char signed unsigned static do while for
break continue return case default if else

/* ---- 2. Identifiers & Edge Cases ---- */
myVar _temp count123 MAX_SIZE x y z
CamelCase snake_case __internal__ A B
validIdentifier _underscoreStart abc123
A1B2C3 veryVeryVeryLongIdentifierName123456

/* ---- 3. Integer Constants ---- */
0 1 42 9999 1234567890 12345 999999999

/* ---- 4. Floating Constants ---- */
3.14 0.5 100. .75 123.456
123. .456 0.0 999.

/* ---- 5. Character Constants ---- */
'a' 'Z' '0' ' '
'\'' '\\' '\"' '\?'
'\a' '\b' '\f' '\n' '\r' '\t' '\v'
'x'

/* ---- 6. String Literals ---- */
"hello world"
"escape test: \n \t \\ \""
""
"line1\nline2"
"quote: \" inside"
"backslash \\ test"

/* ---- 7. Punctuators & Operators ---- */
[ ] ( ) { } . ->
++ -- & * + - ~ !
/ % << >> < > <= >=
== != ^ | && || ? :
; ... = *= /= %= += -=
<<= >>= &= ^= |= , #

/* ---- 8. Tricky Cases (Maximal Munch) ---- */
int mix = 10; /* comment */ float val = 20.5;
int tricky1 = 10/*comment*/20;   // token separation
int tricky2 = a+++b;             // should be a++ + b
int tricky3 = a---b;             // should be a-- - b

/* ---- 9. Realistic Program Fragment ---- */
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    float pi = 3.14159;
    char letter = 'A';
    char *msg = "Result is computed";
    unsigned long count = 0;
    Bool flag = 1;

    for (int i = 0; i < 10; i++) {
        count += i;
    }

    do {
        count--;
    } while (count > 0);

    int x = 7, y = 3;
    int z = (x << 1) | (y >> 1);
    z ^= 0xFF;   /* bitwise XOR */
    z &= ~0x0F;  /* bitwise AND with complement */

    /* compound assignment operators */
    x += 2; x -= 1; x *= 3; x /= 4; x %= 5;
    x <<= 1; x >>= 1; x &= 0xFF; x ^= 0x0F; x |= 0x10;

    int max = (x > y) ? x : y; // ternary

    int arr[5]; // pointer / arrow
    arr[0] = 10;
    
    a++; b--; c+=2; d-=3;
    e*=4; f/=5; g%=6;
    h<<=2; i>>=3;
    j==k; j!=k;
    l<=m; l>=m;
    n&&o; p||q;
    z = arr->b;

    return 0;
}

/* ---- 10. Lexical Errors ---- */

@                   // Unknown token
#hash               // '#' is a punctuator, 'hash' is an identifier
1abc                // Evaluates as INT_CONST(1) and IDENTIFIER(abc) depending on max munch
123..456            // Evaluates as FLOAT_CONST(123.) PUNCTUATOR(.) FLOAT_CONST(.456) or similar
''                  // Empty char constant
'ab'                // Multi-char literal (usually matches the C_CHAR loop if not strictly capped, or throws error depending on implementation)
'\x'                // Invalid escape sequence
'a\xb'              // Invalid escape sequence
"invalid \x escape" // String with invalid escape

'
// Unterminated character constant (newline reached)

"unterminated string
// Unterminated string literal (newline reached)

/* ===== UNTERMINATED COMMENT ===== 
   This comment does not end and should consume the rest of the file.
   Place no valid tests below this line!