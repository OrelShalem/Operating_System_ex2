/*
Program: mync (My Neat C)
Description: This program takes another program as an argument and runs it,
             redirecting its input and output streams.

Usage: mync -e <program_name> [program_arguments]

Example: mync -e ttt 198345762
         (This will run the 'ttt' program from the previous step with the argument '198345762')
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Check if the number of arguments is less than 3 or the first argument is not "-e"
    if (argc < 3 || strcmp(argv[1], "-e") != 0) {
        fprintf(stderr, "Usage: %s -e <program_name> [program_arguments]\n", argv[0]);
        return 1;
    }

    char *program_name = argv[2]; // The name of the program to run
    char **program_args = &argv[2]; // The arguments to the program to run

    // Create a pipe for redirecting the program's output
    int output_pipe[2];
    if (pipe(output_pipe) == -1) {
        perror("pipe");
        return 1;
    }

    // Fork a child process to run the program
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;
    }

    if (child_pid == 0) {
        // Child process

        // Redirect the child's output to the write end of the pipe
        if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            return 1;
        }

        // Close the unused pipe ends
        close(output_pipe[0]);
        close(output_pipe[1]);

        // Execute the program
        execvp(program_name, program_args);
        perror("execvp");
        return 1;
    } else {
        // Parent process

        // Close the write end of the pipe
        close(output_pipe[1]);

        // Read the program's output from the read end of the pipe
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }

        // Wait for the child process to finish
        wait(NULL);

        // Close the read end of the pipe
        close(output_pipe[0]);
    }

    return 0;
}
