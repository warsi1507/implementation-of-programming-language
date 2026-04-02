%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Output files */
FILE *token_file;
FILE *st_file;

/* Line number tracking */
int line_num = 1;

/* ---- Symbol Table ---- */
#define MAX_SYMBOLS 2048
#define MAX_ID_LEN  256

typedef struct {
    char name[MAX_ID_LEN];
    int  first_line;
} Symbol;

Symbol sym_table[MAX_SYMBOLS];
int sym_count = 0;

/* Returns index if found, else -1 */
int lookup_symbol(const char *name) {
    for (int i = 0; i < sym_count; i++)
        if (strcmp(sym_table[i].name, name) == 0)
            return i;
    return -1;
}

/* Insert if new, returns index */
int insert_symbol(const char *name, int line) {
    int idx = lookup_symbol(name);
    if (idx != -1) return idx;
    if (sym_count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error (line %d): Symbol table full.\n", line);
        return -1;
    }
    strncpy(sym_table[sym_count].name, name, MAX_ID_LEN - 1);
    sym_table[sym_count].name[MAX_ID_LEN - 1] = '\0';
    sym_table[sym_count].first_line = line;
    return sym_count++;
}

%}

/* ---- Definitions ---- */
DIGIT        [0-9]
NONZERO      [1-9]
LETTER       [a-zA-Z_]
ID           {LETTER}({LETTER}|{DIGIT})*
INT_CONST    {NONZERO}{DIGIT}*|0
DIGIT_SEQ    {DIGIT}+
FLOAT_CONST  ({DIGIT_SEQ}"."{DIGIT_SEQ})|({DIGIT_SEQ}".")|("."{DIGIT_SEQ})
ESCAPE       \\['"?\\abfnrtv]
C_CHAR       [^'\\\n]|{ESCAPE}
CHAR_CONST   '({C_CHAR})+'
S_CHAR       [^"\\\n]|{ESCAPE}
STR_LIT      \"({S_CHAR})*\"
ML_COMMENT   "/*"([^*]|\*+[^*/])*\*+"/"
SL_COMMENT   "//"[^\n]*
WS           [ \t\r]+

BAD_ESCAPE    \\[^'"?\\abfnrtv\n]
BAD_ESC_CHAR  '({C_CHAR})*{BAD_ESCAPE}({C_CHAR})*'
BAD_ESC_STR   \"({S_CHAR})*{BAD_ESCAPE}({S_CHAR})*\"


%option noyywrap

%%

{SL_COMMENT}    { /* skip */ }
{ML_COMMENT}    {
                    for (int i = 0; yytext[i]; i++)
                        if (yytext[i] == '\n') line_num++;
                }

"void"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"short"     { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"int"       { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"float"     { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"double"    { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"long"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"Bool"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"char"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"signed"    { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"unsigned"  { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"static"    { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"do"        { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"while"     { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"for"       { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"break"     { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"continue"  { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"return"    { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"case"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"default"   { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"if"        { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }
"else"      { fprintf(token_file, "Line %d: KEYWORD(%s)\n",    line_num, yytext); }

{ID}            {
                    fprintf(token_file, "Line %d: IDENTIFIER(%s)\n", line_num, yytext);
                    insert_symbol(yytext, line_num);
                }

{FLOAT_CONST}   { fprintf(token_file, "Line %d: FLOAT_CONST(%s)\n",  line_num, yytext); }
{INT_CONST}     { fprintf(token_file, "Line %d: INT_CONST(%s)\n",    line_num, yytext); }

''              {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Empty character constant.\n", line_num);
                    fprintf(token_file, "Line %d: ERROR(Empty char constant)\n", line_num);
                }

{BAD_ESC_CHAR}  {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Invalid escape sequence in '%s'.\n", line_num, yytext);
                    fprintf(token_file, "Line %d: ERROR(Invalid escape sequence '%s')\n", line_num, yytext);
                }

{CHAR_CONST}    { fprintf(token_file, "Line %d: CHAR_CONST(%s)\n",   line_num, yytext); }

{BAD_ESC_STR}   {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Invalid escape sequence in string '%s'.\n", line_num, yytext);
                    fprintf(token_file, "Line %d: ERROR(Invalid escape sequence in string '%s')\n", line_num, yytext);
                }
{STR_LIT}       { fprintf(token_file, "Line %d: STRING_LITERAL(%s)\n", line_num, yytext); }

"["     { fprintf(token_file, "Line %d: PUNCTUATOR([)\n",    line_num); }
"]"     { fprintf(token_file, "Line %d: PUNCTUATOR(])\n",    line_num); }
"("     { fprintf(token_file, "Line %d: PUNCTUATOR(()\n",    line_num); }
")"     { fprintf(token_file, "Line %d: PUNCTUATOR())\n",    line_num); }
"{"     { fprintf(token_file, "Line %d: PUNCTUATOR({)\n",    line_num); }
"}"     { fprintf(token_file, "Line %d: PUNCTUATOR(})\n",    line_num); }
"->"    { fprintf(token_file, "Line %d: PUNCTUATOR(->)\n",   line_num); }
"."     { fprintf(token_file, "Line %d: PUNCTUATOR(.)\n",    line_num); }
"++"    { fprintf(token_file, "Line %d: PUNCTUATOR(++)\n",   line_num); }
"--"    { fprintf(token_file, "Line %d: PUNCTUATOR(--)\n",   line_num); }
"&"     { fprintf(token_file, "Line %d: PUNCTUATOR(&)\n",    line_num); }
"*"     { fprintf(token_file, "Line %d: PUNCTUATOR(*)\n",    line_num); }
"+"     { fprintf(token_file, "Line %d: PUNCTUATOR(+)\n",    line_num); }
"-"     { fprintf(token_file, "Line %d: PUNCTUATOR(-)\n",    line_num); }
"~"     { fprintf(token_file, "Line %d: PUNCTUATOR(~)\n",    line_num); }
"!"     { fprintf(token_file, "Line %d: PUNCTUATOR(!)\n",    line_num); }
"/"     { fprintf(token_file, "Line %d: PUNCTUATOR(/)\n",    line_num); }
"%"     { fprintf(token_file, "Line %d: PUNCTUATOR(%%)\n",   line_num); }
"<<"    { fprintf(token_file, "Line %d: PUNCTUATOR(<<)\n",   line_num); }
">>"    { fprintf(token_file, "Line %d: PUNCTUATOR(>>)\n",   line_num); }
"<="    { fprintf(token_file, "Line %d: PUNCTUATOR(<=)\n",   line_num); }
">="    { fprintf(token_file, "Line %d: PUNCTUATOR(>=)\n",   line_num); }
"<"     { fprintf(token_file, "Line %d: PUNCTUATOR(<)\n",    line_num); }
">"     { fprintf(token_file, "Line %d: PUNCTUATOR(>)\n",    line_num); }
"=="    { fprintf(token_file, "Line %d: PUNCTUATOR(==)\n",   line_num); }
"!="    { fprintf(token_file, "Line %d: PUNCTUATOR(!=)\n",   line_num); }
"^"     { fprintf(token_file, "Line %d: PUNCTUATOR(^)\n",    line_num); }
"|"     { fprintf(token_file, "Line %d: PUNCTUATOR(|)\n",    line_num); }
"&&"    { fprintf(token_file, "Line %d: PUNCTUATOR(&&)\n",   line_num); }
"||"    { fprintf(token_file, "Line %d: PUNCTUATOR(||)\n",   line_num); }
"?"     { fprintf(token_file, "Line %d: PUNCTUATOR(?)\n",    line_num); }
":"     { fprintf(token_file, "Line %d: PUNCTUATOR(:)\n",    line_num); }
";"     { fprintf(token_file, "Line %d: PUNCTUATOR(;)\n",    line_num); }
"..."   { fprintf(token_file, "Line %d: PUNCTUATOR(...)\n",  line_num); }
"="     { fprintf(token_file, "Line %d: PUNCTUATOR(=)\n",    line_num); }
"*="    { fprintf(token_file, "Line %d: PUNCTUATOR(*=)\n",   line_num); }
"/="    { fprintf(token_file, "Line %d: PUNCTUATOR(/=)\n",   line_num); }
"%="    { fprintf(token_file, "Line %d: PUNCTUATOR(%%=)\n",  line_num); }
"+="    { fprintf(token_file, "Line %d: PUNCTUATOR(+=)\n",   line_num); }
"-="    { fprintf(token_file, "Line %d: PUNCTUATOR(-=)\n",   line_num); }
"<<="   { fprintf(token_file, "Line %d: PUNCTUATOR(<<=)\n",  line_num); }
">>="   { fprintf(token_file, "Line %d: PUNCTUATOR(>>=)\n",  line_num); }
"&="    { fprintf(token_file, "Line %d: PUNCTUATOR(&=)\n",   line_num); }
"^="    { fprintf(token_file, "Line %d: PUNCTUATOR(^=)\n",   line_num); }
"|="    { fprintf(token_file, "Line %d: PUNCTUATOR(|=)\n",   line_num); }
","     { fprintf(token_file, "Line %d: PUNCTUATOR(,)\n",    line_num); }
"#"     { fprintf(token_file, "Line %d: PUNCTUATOR(#)\n",    line_num); }

\n      { line_num++; }
{WS}    { /* skip */ }

'({C_CHAR})*\n  {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Unterminated character constant.\n", line_num);
                    fprintf(token_file, "Line %d: ERROR(Unterminated char constant)\n", line_num);
                    line_num++;
                }

\"({S_CHAR})*\n {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Unterminated string literal.\n", line_num);
                    fprintf(token_file, "Line %d: ERROR(Unterminated string literal)\n", line_num);
                    line_num++;
                }

"/*"([^*]|\*+[^*/])*  {
                    fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Unterminated multi-line comment.\n", line_num);
                    fprintf(token_file, "Line %d: ERROR(Unterminated comment)\n", line_num);
                }

.       {
            fprintf(stderr, "%d: \033[31mLexical Error:\033[0m Unknown token '%s'.\n", line_num, yytext);
            fprintf(token_file, "Line %d: ERROR(Unknown token '%s')\n", line_num, yytext);
        }

%%

int main(int argc, char *argv[]) {
    /* Open input */
    if (argc > 1) {
        FILE *fin = fopen(argv[1], "r");
        if (!fin) { perror(argv[1]); return 1; }
        yyin = fin;
    }

    /* Open output files */
    token_file = fopen("lexer_token.txt", "w");
    if (!token_file) { perror("lexer_token.txt"); return 1; }

    st_file = fopen("lexer_st.txt", "w");
    if (!st_file) { perror("lexer_st.txt"); return 1; }

    /* Run lexer */
    yylex();

    /* Write symbol table */
    fprintf(st_file, "%-5s  %-40s  %s\n", "Index", "Identifier", "Line");
    fprintf(st_file, "%-5s  %-40s  %s\n", "-----", "----------", "----");
    for (int i = 0; i < sym_count; i++)
        fprintf(st_file, "%-5d  %-40s  %d\n", i, sym_table[i].name, sym_table[i].first_line);

    fclose(token_file);
    fclose(st_file);

    printf("\nTokens are present in lexer_token.txt \nSymbol Table is present in lexer_st.txt\n");
    return 0;
}
