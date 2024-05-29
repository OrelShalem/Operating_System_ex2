#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netdb.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void run_program(char *args_as_string)
{
    // tokenize the string - split by space
    char *token = strtok(args_as_string, " ");

    if (token == NULL)
    {
        fprintf(stderr, "No arguments provided\n");
        exit(1);
    }

    // create an array of strings to store the arguments
    char *args[100]; // increase or decrease this number as needed
    int n = 0;
    args[n++] = token; // add the first argument (program name)

    // get the rest of the arguments
    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (token != NULL)
        {
            args[n++] = token;
        }
    }
    args[n] = NULL; // null-terminate the array

    // fork and execute the program
    pid_t pid = fork();
    if (pid < 0)
    { // fork failed
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }
    else if (pid == 0)
    { // child process
        execvp(args[0], args);
        fprintf(stderr, "Exec failed\n");
        exit(1);
    }
    else
    { // parent process
        int status;
        waitpid(pid, &status, 0); // wait for the child process to finish
    }
}

// פונקציה לקריאת הודעה מהלקוח
void receive_message(int sockfd, char *buffer, int buffer_size)
{
    int valread = read(sockfd, buffer, buffer_size);
    if (valread < 0)
    {
        error("Error reading from socket");
    }
    buffer[valread] = '\0'; // Adding null terminator to make buffer a valid string
}

// פונקציה לשליחת הודעה ללקוח
void send_message(int sockfd, const char *message)
{
    if (send(sockfd, message, strlen(message), 0) < 0)
    {
        error("Error sending message");
    }
}

void handle_chat(int input_fd, int output_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t valread;

    while (1)
    {
        valread = read(input_fd, buffer, BUFFER_SIZE);
        if (valread <= 0)
        {
            break;
        }
        buffer[valread] = '\0';
        printf("Received message: %s", buffer);

        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send_message(output_fd, buffer);
    }
}

int handle_client(int input_sockfd, int output_sockfd, char *e_command, char *i_command, char *o_command, char *b_command)
{
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    if (i_command != NULL)
    {
        input_fd = input_sockfd;
    }
    if (o_command != NULL)
    {
        output_fd = output_sockfd;
    }
    if (b_command != NULL)
    {
        input_fd = input_sockfd;
        output_fd = output_sockfd;
    }

    if (e_command != NULL)
    {
        char *full_command = NULL;
        full_command = malloc(strlen(e_command) + strlen(e_command + 6) + 2);
        strcpy(full_command, e_command);
        strcat(full_command, " ");
        strcat(full_command, e_command + 6);

        int pid = fork();
        if (pid == 0)
        { // child process
            if (input_fd != STDIN_FILENO)
            {
                dup2(input_fd, STDIN_FILENO);
            }
            if (output_fd != STDOUT_FILENO)
            {
                dup2(output_fd, STDOUT_FILENO);
            }
            run_program(full_command);
            free(full_command);
            exit(0);
        }
        else if (pid > 0)
        { // parent process
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                return WEXITSTATUS(status);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            error("Fork failed");
        }
    }
    else
    {
        // No -e flag provided, enter chat mode
        handle_chat(input_sockfd, output_sockfd);
    }

    return -1;
}

void run_server_and_client(int server_port, char *client_hostname, int client_port, char *e_command, char *i_command, char *o_command)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        error("Error opening server socket");
    }

    int optval = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting server socket options");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error on binding server socket");
    }

    if (listen(server_sockfd, 5) < 0)
    {
        error("Error on listen");
    }

    printf("Server is listening on port %d\n", server_port);

    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd < 0)
    {
        error("Error opening client socket");
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);

    struct hostent *client_host = gethostbyname(client_hostname);
    if (client_host == NULL)
    {
        error("Invalid client hostname");
    }

    memcpy(&client_addr.sin_addr, client_host->h_addr_list[0], sizeof(struct in_addr));

    if (connect(client_sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        error("Connection to client failed");
    }
    printf("Connected to client\n");

    socklen_t client_addr_len = sizeof(client_addr);
    int input_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (input_sockfd < 0)
    {
        error("Error on accept");
    }

    printf("Client connected to server\n");

    handle_client(input_sockfd, client_sockfd, e_command, i_command, o_command, NULL);

    close(input_sockfd);
    close(client_sockfd);
    close(server_sockfd);
}
void run_server(int port, char *e_command, char *i_command, char *o_command, char *b_command)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        error("Error opening socket");
    }

    int optval = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting socket options");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error on binding");
    }

    if (listen(server_sockfd, 5) < 0)
    {
        error("Error on listen");
    }

    printf("Server is listening on port %d\n", port);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd < 0)
        {
            error("Error on accept");
        }

        printf("Client connected\n");

        int child_return_code = handle_client(client_sockfd, client_sockfd, e_command, i_command, o_command, b_command);

        if (child_return_code == 0)
        {
            printf("Child process finished successfully.\n");
            close(client_sockfd); // Close the client socket
            break;                // Exit the loop if the child process finished successfully
        }
        else if (child_return_code > 0)
        {
            printf("Child process finished with return code %d.\n", child_return_code);
        }
        else
        {
            printf("Child process did not exit normally.\n");
        }

        close(client_sockfd);

        if (child_return_code != 0)
        {
            break; // Exit the loop if the child process has finished with a non-zero return code
        }
    }

    close(server_sockfd);
}

void run_client(char *hostname, int port, char *e_command, char *i_command, char *o_command, char *b_command)
{
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd < 0)
    {
        error("Error opening socket");
    }

    int optval = 1;
    if (setsockopt(client_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting UDP socket options");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname, &serv_addr.sin_addr) <= 0)
    {
        error("Invalid address or hostname");
    }

    if (connect(client_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Connection failed");
    }

    printf("Connected to server\n");

    handle_client(client_sockfd, client_sockfd, e_command, i_command, o_command, b_command);

    close(client_sockfd);
}
void run_udp_server(int port, char *e_command, char *i_command, int timeout)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Error opening UDP socket");
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting UDP socket options");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error binding UDP socket");
    }

    alarm(timeout);

    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                break;
            }
            else
            {
                error("Error receiving UDP message");
            }
        }

        int input_fd = sockfd;
        int output_fd = STDOUT_FILENO;
        handle_client(input_fd, output_fd, e_command, i_command, NULL, NULL);
    }

    close(sockfd);
}

void run_udp_client(char *hostname, int port, char *e_command, char *o_command, int timeout)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Error opening UDP socket");
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting UDP socket options");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0)
    {
        error("Invalid address or hostname");
    }

    alarm(timeout);

    int input_fd = STDIN_FILENO;
    int output_fd = sockfd;
    handle_client(input_fd, output_fd, e_command, NULL, o_command, NULL);

    close(sockfd);
}

int main(int argc, char *argv[])
{
    int opt;
    char *e_command = NULL;
    char *i_command = NULL;
    char *o_command = NULL;
    char *b_command = NULL;
    char *tcp_server_port = NULL;
    char *udp_server_port = NULL;
    int timeout = 0;

    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'e':
            e_command = optarg;
            break;
        case 'i':
            i_command = optarg;
            if (strncmp(i_command, "TCPS", 4) == 0)
            {
                tcp_server_port = i_command + 4;
            }
            else if (strncmp(i_command, "UDPS", 4) == 0)
            {
                udp_server_port = i_command + 4;
            }
            break;
        case 'o':
            o_command = optarg;
            break;
        case 'b':
            b_command = optarg;
            if (strncmp(b_command, "TCPS", 4) == 0)
            {
                tcp_server_port = b_command + 4;
            }
            break;
        case 't':
            timeout = atoi(optarg);
            break;
        default:
            error("Usage: mync [-e command] [-i input_spec] [-o output_spec] [-b io_spec] [-t timeout]");
        }
    }

    if (e_command == NULL)
    {
        error("No -e flag");
    }

    if (tcp_server_port != NULL)
    {
        int port = atoi(tcp_server_port);
        run_server(port, e_command, i_command, o_command, b_command);
    }
    else if (udp_server_port != NULL)
    {
        int port = atoi(udp_server_port);
        run_udp_server(port, e_command, i_command, timeout);
    }
    else if (optind < argc)
    {
        char *hostname = argv[optind];
        int port = atoi(argv[optind + 1]);
        if (i_command != NULL && strncmp(i_command, "UDPC", 4) == 0)
        {
            run_udp_client(hostname, port, e_command, o_command, timeout);
        }
        else
        {
            run_client(hostname, port, e_command, i_command, o_command, b_command);
        }
    }
    else
    {
        error("Missing server or client information");
    }

    return 0;
}