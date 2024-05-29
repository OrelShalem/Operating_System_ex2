# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99

# Source files
SOURCES = ttt.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executables
EXECUTABLES = ttt

# Default target
all: $(EXECUTABLES)

# Build the executables
ttt: ttt.o
	$(CC) $(CFLAGS) ttt.o -o ttt


# Compile the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

.PHONY: all clean