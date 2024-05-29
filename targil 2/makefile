# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99

# Source files
SOURCES = ttt.c mync.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executables
EXECUTABLES = ttt mync

# Default target
all: $(EXECUTABLES)

# Build the executables
ttt: ttt.o
	$(CC) $(CFLAGS) ttt.o -o ttt

mync: mync.o
	$(CC) $(CFLAGS) mync.o -o mync

# Compile the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

.PHONY: all clean