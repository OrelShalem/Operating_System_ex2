# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -fprofile-arcs -ftest-coverage

# Source files
MYNC_SRC = mync.c
TTT_SRC = ttt.c
UDS_SERVER_SRC = uds_server.c

# Object files
MYNC_OBJ = $(MYNC_SRC:.c=.o)
TTT_OBJ = $(TTT_SRC:.c=.o)
UDS_SERVER_OBJ = $(UDS_SERVER_SRC:.c=.o)

# Executables
MYNC_EXECUTABLE = mync
TTT_EXECUTABLE = ttt
UDS_SERVER_EXECUTABLE = uds_server

# Default target
all: $(MYNC_EXECUTABLE) $(TTT_EXECUTABLE) $(UDS_SERVER_EXECUTABLE)

# Build the mync executable
$(MYNC_EXECUTABLE): $(MYNC_OBJ)
	$(CC) $(CFLAGS) $(MYNC_OBJ) -o $(MYNC_EXECUTABLE)

# Build the ttt executable
$(TTT_EXECUTABLE): $(TTT_OBJ)
	$(CC) $(CFLAGS) $(TTT_OBJ) -o $(TTT_EXECUTABLE)

# Build the uds_server executable
$(UDS_SERVER_EXECUTABLE): $(UDS_SERVER_OBJ)
	$(CC) $(CFLAGS) $(UDS_SERVER_OBJ) -o $(UDS_SERVER_EXECUTABLE)

# Compile the mync source file
$(MYNC_OBJ): $(MYNC_SRC)
	$(CC) $(CFLAGS) -c $(MYNC_SRC) -o $(MYNC_OBJ)

# Compile the ttt source file
$(TTT_OBJ): $(TTT_SRC)
	$(CC) $(CFLAGS) -c $(TTT_SRC) -o $(TTT_OBJ)

# Compile the uds_server source file
$(UDS_SERVER_OBJ): $(UDS_SERVER_SRC)
	$(CC) $(CFLAGS) -c $(UDS_SERVER_SRC) -o $(UDS_SERVER_OBJ)

# Clean up
clean:
	rm -f $(MYNC_OBJ) $(TTT_OBJ) $(UDS_SERVER_OBJ) $(MYNC_EXECUTABLE) $(TTT_EXECUTABLE) $(UDS_SERVER_EXECUTABLE) *.gcno *.gcda *.gcov

.PHONY: all clean