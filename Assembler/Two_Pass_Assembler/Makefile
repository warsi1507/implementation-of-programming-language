# Makefile for Two-Pass Assembler

CC = gcc
CFLAGS = -w -Wall -Wextra -std=c99
TARGET = assembler
SOURCES = main.c
HEADERS = assembler.h

# Default target
all: $(TARGET)

# Build the assembler
$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Run with sample input
test: $(TARGET)
	./$(TARGET) sample_input.txt intermediate.txt output.txt

# Clean build artifacts
clean:
	rm -f $(TARGET) intermediate.txt output.txt

# Clean everything including test files
distclean: clean
	rm -f *.o *~

# Help
help:
	@echo "Two-Pass Assembler Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the assembler (default)"
	@echo "  test      - Build and run with sample_input.txt"
	@echo "  clean     - Remove build artifacts"
	@echo "  distclean - Remove all generated files"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build the assembler"
	@echo "  make test         # Run test"
	@echo "  ./assembler <input> <intermediate> <output>"

.PHONY: all test clean distclean help