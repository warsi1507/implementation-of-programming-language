# Makefile for nanoC Lexer

CC      = gcc
FLEX    = flex
LEX_SRC = main.l
LEX_C   = lex.yy.c
TEST    = test.nc
TARGET  = lexer

.PHONY: all clean run

all: $(TARGET)

$(LEX_C): $(LEX_SRC)
	$(FLEX) $(LEX_SRC)

$(TARGET): $(LEX_C)
	$(CC) -o $(TARGET) $(LEX_C) -lfl

run: $(TARGET)
	./$(TARGET) $(TEST)

clean:
	rm -f $(LEX_C) $(TARGET) $(TARGET)_token.txt $(TARGET)_st.txt
