# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Source files
MYNC_SRC = mync.c
TTT_SRC = ttt.c

# Object files
MYNC_OBJ = $(MYNC_SRC:.c=.o)
TTT_OBJ = $(TTT_SRC:.c=.o)

# Executables
MYNC_EXECUTABLE = mync
TTT_EXECUTABLE = ttt

# Default target
all: $(MYNC_EXECUTABLE) $(TTT_EXECUTABLE)

# Build the mync executable
$(MYNC_EXECUTABLE): $(MYNC_OBJ)
	$(CC) $(CFLAGS) $(MYNC_OBJ) -o $(MYNC_EXECUTABLE)

# Build the ttt executable
$(TTT_EXECUTABLE): $(TTT_OBJ)
	$(CC) $(CFLAGS) $(TTT_OBJ) -o $(TTT_EXECUTABLE)

# Compile the mync source file
$(MYNC_OBJ): $(MYNC_SRC)
	$(CC) $(CFLAGS) -c $(MYNC_SRC) -o $(MYNC_OBJ)

# Compile the ttt source file
$(TTT_OBJ): $(TTT_SRC)
	$(CC) $(CFLAGS) -c $(TTT_SRC) -o $(TTT_OBJ)

# Clean up
clean:
	rm -f $(MYNC_OBJ) $(TTT_OBJ) $(MYNC_EXECUTABLE) $(TTT_EXECUTABLE)

.PHONY: all clean