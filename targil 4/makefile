# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -fprofile-arcs -ftest-coverage

# Linker flags
LDFLAGS = -lgcov

# Source files
SOURCES = mync.c ttt.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executables
EXECUTABLES = mync ttt

# Default target
all: $(EXECUTABLES)

# Build the executables
$(EXECUTABLES): %: %.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Compile the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test target
.PHONY: test
test:
	./mync test
	./ttt test

# Code coverage target
.PHONY: coverage
coverage:
	$(MAKE) clean
	$(MAKE) test
	gcov $(SOURCES)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

# Clean up
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLES) *.gcda *.gcno coverage.info
	rm -rf coverage_report

.PHONY: all clean test coverage