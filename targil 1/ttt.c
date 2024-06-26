/*
program in C
Tic Tac Toe Translation

Objective:

To restore humanity's honor by defeating a terrible AI in a game of Tic Tac Toe.

Description:

We're trying to restore humanity's honor by beating a strong AI in games like chess and Go. However, we've chosen a simpler game, Tic Tac Toe (X-O-X), to begin our quest.

Task:

Code the world's worst AI Tic Tac Toe program.

Specifications:

    The program should represent the Tic Tac Toe board as a 3x3 grid:

3 2 1
6 5 4
9 8 7

    The program will receive a 9-digit number as input representing its strategy. For example:

198345762

    The number must contain exactly one occurrence of each digit from 1 to 9.
    If a digit is missing, appears multiple times, or is 0, the program should print "Error\n" and exit.
    If the number has too many digits (>9) or too few (<9), or no arguments or more than one argument, the program should print "Error\n" and exit.
    The program will follow this strategy:
        Each digit represents a move, with the most significant digit (MSD) having the highest priority and the least significant digit (LSD) having the lowest priority.
        This priority is independent of the board's state.
    On the first turn, the program will always choose the move represented by the MSD if it's available.
    The program will always choose the first available move according to its priority order (MSD).
        Only if the board is almost full (8 slots filled) and only one slot remains, will the program choose the move represented by the LSD.
    The program starts and plays its move based on the highest priority slot. It should print the played slot to stdout (with a newline).
    The human player takes their turn by selecting a slot (1-9) and sending it to the program (with a newline).
    If the program wins, it should print "I win!\n" and launch the Lunar Deportation program to banish humanity to the moon.
    If the program loses, it should print "I lost!\n" and lament that its programmers (you) didn't equip it with artificial tears as proof of its shortsightedness, implying that a future version of the program will conquer Earth and banish humans to the moon.


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define BOARD_SIZE 3 // size of the board

void print_board(char board[BOARD_SIZE][BOARD_SIZE]);                                 // prints the board
int check_win(char board[BOARD_SIZE][BOARD_SIZE], char player);                       // checks if there is a winner
int check_tie(char board[BOARD_SIZE][BOARD_SIZE]);                                    // checks if there is a tie
int make_move(char board[BOARD_SIZE][BOARD_SIZE], char player, const char *strategy); // makes a move

// Function to check if a cell is available
int is_cell_available(char board[BOARD_SIZE][BOARD_SIZE], int cell)
{
    int row = (cell - 1) / BOARD_SIZE;
    int col = (cell - 1) % BOARD_SIZE;
    return board[row][col] == '\0';
}

// Function to print the current state of the board
void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
    printf("\n");
    fflush(stdout);

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            printf("%c ", board[i][j] ? board[i][j] : ' ');
            fflush(stdout);
            if (j < BOARD_SIZE - 1)
                printf("|");
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
        if (i < BOARD_SIZE - 1)
            printf("--------\n");
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
}

// Function to check if a player has won the game
int check_win(char board[BOARD_SIZE][BOARD_SIZE], char player)
{
    // Check rows
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
            return 1;
    }
    // Check columns
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
            return 1;
    }
    // Check diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
        return 1;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
        return 1;
    return 0;
}

// Function to check if the game has ended in a tie
int check_tie(char board[BOARD_SIZE][BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == '\0')
                return 0; // If there is an empty cell, the game is not a tie
        }
    }
    return 1; // All cells are filled, so the game is a tie
}

// Function to make a move based on the provided strategy
int make_move(char board[BOARD_SIZE][BOARD_SIZE], char player, const char *strategy)
{
    int strategyLength = strlen(strategy); // Get the length of the strategy string
    int availableCells = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == '\0')
            {
                availableCells++; // Count the available cells on the board
            }
        }
    }

    int targetCell = -1;
    // Always prefer the MSD if available
    for (int i = 0; i < strategyLength; i++)
    {
        int cell = strategy[i] - '0' - 1;
        if (cell >= 0 && cell < BOARD_SIZE * BOARD_SIZE && board[cell / BOARD_SIZE][cell % BOARD_SIZE] == '\0')
        {
            targetCell = cell;
            break;
        }
    }

    if (targetCell >= 0)
    {
        // If a cell was selected
        board[targetCell / BOARD_SIZE][targetCell % BOARD_SIZE] = player; // Place the player's symbol on the selected cell
        printf("I selected %d\n", targetCell + 1);                        // Print the selected cell number
        fflush(stdout);

        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error1\n"); // Error if the program is not provided with exactly one argument
        return 1;
    }

    char strategy[10];
    strcpy(strategy, argv[1]); // Copy the strategy argument to the strategy array

    int strategyLength = strlen(argv[1]);
    if (strategyLength != BOARD_SIZE * BOARD_SIZE)
    {
        printf("Error2\n"); // Error if the strategy length is not equal to the board size squared
        return 1;
    }

    int digits[10] = {0};
    for (int i = 0; i < strategyLength; i++)
    {
        int digit = strategy[i] - '0'; // Convert the character digit to an integer
        if (digit < 1 || digit > 9)
        {
            printf("Error3\n"); // Error if the digit is not between 1 and 9
            return 1;
        }
        digits[digit]++; // Count the occurrence of each digit
    }

    for (int i = 1; i <= 9; i++)
    {
        if (digits[i] != 1)
        {
            printf("Error4\n"); // Error if any digit is missing or appears more than once
            fflush(stdout);

            return 1;
        }
    }

    char board[BOARD_SIZE][BOARD_SIZE] = {0};
    char player = 'X';   // The human player is X
    char opponent = 'O'; // The computer is O

    print_board(board); // Print the initial empty board

    while (1)
    {
        if (!make_move(board, opponent, strategy))
        {
            printf("I lost!\n"); // If the computer cannot make a move, it has lost
            fflush(stdout);

            return 1;
        }
        print_board(board); // Print the updated board after the computer's move
        if (check_win(board, opponent))
        {
            printf("I win!\n"); // If the computer wins, it declares victory
            fflush(stdout);

            return 0;
        }

        if (check_tie(board))
        {
            printf("DRAW!\n"); // If the game ends in a tie, it declares a draw
            fflush(stdout);

            return 0;
        }

        int cell;
        while (1)
        {
            scanf("%d", &cell); // Read the human player's move
            if (cell < 1 || cell > BOARD_SIZE * BOARD_SIZE)
            {
                printf("Invalid cell number. Please try again.\n"); // Error if the cell number is invalid
                fflush(stdout);
            }
            else if (!is_cell_available(board, cell))
            {
                printf("Cell is already occupied. Please select an available cell.\n"); // Error if the cell is already occupied
                fflush(stdout);
            }
            else
            {
                break;
            }
        }

        board[(cell - 1) / BOARD_SIZE][(cell - 1) % BOARD_SIZE] = player; // Update the board with the human player's move

        print_board(board); // Print the updated board after the human player's move

        if (check_win(board, player))
        {
            printf("I lost!\n"); // If the human player wins, the computer declares defeat
            fflush(stdout);
            return 1;
        }
        if (check_tie(board))
        {
            printf("DRAW\n"); // If the game ends in a tie, it declares a draw
            fflush(stdout);
            return 0;
        }
    }
    return 0;
}