#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netdb.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define UNIX_DOMAIN_PATH_MAX 108

// This function prints an error message and exits the program
void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// This function runs the specified program with the given arguments
void run_program(char *args_as_string)
{
    // Tokenize the string - split by space
    char *token = strtok(args_as_string, " ");

    if (token == NULL)
    {
        fprintf(stderr, "No arguments provided\n");
        exit(1);
    }

    // Create an array of strings to store the arguments
    char *args[100]; // Increase or decrease this number as needed
    int n = 0;
    args[n++] = token; // Add the first argument (program name)

    // Get the rest of the arguments
    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (token != NULL)
        {
            args[n++] = token;
        }
    }
    args[n] = NULL; // Null-terminate the array

    // Fork and execute the program
    pid_t pid = fork();
    if (pid < 0)
    { // Fork failed
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }
    else if (pid == 0)
    { // Child process
        execvp(args[0], args);
        fprintf(stderr, "Exec failed\n");
        exit(1);
    }
    else
    { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
    }
}

// This function reads a message from the client
void receive_message(int sockfd, char *buffer, int buffer_size)
{
    int valread = read(sockfd, buffer, buffer_size);
    if (valread < 0)
    {
        error("Error reading from socket");
    }
    buffer[valread] = '\0'; // Adding null terminator to make buffer a valid string
}

// This function sends a message to the client
void send_message(int sockfd, const char *message)
{
    if (send(sockfd, message, strlen(message), 0) < 0)
    {
        error("Error sending message");
    }
}

// This function handles the chat mode between the client and server
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

// This function handles the client connection and executes the specified command or enters chat mode
int handle_client(int input_sockfd, int output_sockfd, char *e_command, char *i_command, char *o_command, char *b_command)
{
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    // Set the input and output file descriptors based on the command-line options
    if (i_command != NULL)
    {
        if (strncmp(i_command, "UDSSD", 5) == 0 || strncmp(i_command, "UDSSS", 5) == 0)
        {
            input_fd = input_sockfd;
        }
        else
        {
            input_fd = input_sockfd;
        }
    }
    if (o_command != NULL)
    {
        if (strncmp(o_command, "UDSCD", 5) == 0 || strncmp(o_command, "UDSCS", 5) == 0)
        {
            output_fd = output_sockfd;
        }
        else
        {
            output_fd = output_sockfd;
        }
    }
    if (b_command != NULL)
    {
        if (strncmp(b_command, "UDSSS", 5) == 0)
        {
            input_fd = input_sockfd;
            output_fd = output_sockfd;
        }
        else
        {
            input_fd = input_sockfd;
            output_fd = output_sockfd;
        }
    }

    // If the -e command is specified, execute the command
    if (e_command != NULL)
    {
        char *full_command = NULL;
        full_command = malloc(strlen(e_command) + strlen(e_command + 6) + 2);
        strcpy(full_command, e_command);
        strcat(full_command, " ");
        strcat(full_command, e_command + 6);

        int pid = fork();
        if (pid == 0)
        { // Child process
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
        { // Parent process
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

// This function runs the server and client in a TCP connection
void run_server_and_client(int server_port, char *client_hostname, int client_port, char *e_command, char *i_command, char *o_command)
{
    // Set up the server socket
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

    // Set up the client socket
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

    // Accept the client connection to the server
    socklen_t client_addr_len = sizeof(client_addr);
    int input_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (input_sockfd < 0)
    {
        error("Error on accept");
    }

    printf("Client connected to server\n");

    // Handle the client connection
    handle_client(input_sockfd, client_sockfd, e_command, i_command, o_command, NULL);

    // Clean up
    close(input_sockfd);
    close(client_sockfd);
    close(server_sockfd);
}

// This function runs the UDP server and TCP client
void run_udp_server_tcp_client(int udp_port, char *tcp_hostname, int tcp_port, char *e_command, char *i_command, char *o_command)
{
    // Set up the UDP server socket
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0)
    {
        error("Error opening UDP server socket");
    }

    int optval = 1;
    if (setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        error("Error setting UDP server socket options");
    }

    struct sockaddr_in udp_serv_addr;
    memset(&udp_serv_addr, 0, sizeof(udp_serv_addr));
    udp_serv_addr.sin_family = AF_INET;
    udp_serv_addr.sin_port = htons(udp_port);
    udp_serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_sockfd, (struct sockaddr *)&udp_serv_addr, sizeof(udp_serv_addr)) < 0)
    {
        error("Error on binding UDP server socket");
    }

    printf("UDP server is listening on port %d\n", udp_port);

    // Set up the TCP client socket
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0)
    {
        error("Error opening TCP client socket");
    }

    struct sockaddr_in tcp_client_addr;
    memset(&tcp_client_addr, 0, sizeof(tcp_client_addr));
    tcp_client_addr.sin_family = AF_INET;
    tcp_client_addr.sin_port = htons(tcp_port);

    struct hostent *tcp_host = gethostbyname(tcp_hostname);
    if (tcp_host == NULL)
    {
        error("Invalid TCP client hostname");
    }

    memcpy(&tcp_client_addr.sin_addr, tcp_host->h_addr_list[0], sizeof(struct in_addr));

    if (connect(tcp_sockfd, (struct sockaddr *)&tcp_client_addr, sizeof(tcp_client_addr)) < 0)
    {
        error("Connection to TCP client failed");
    }

    printf("Connected to TCP client\n");

    // Handle the client connection
    char buffer[BUFFER_SIZE];
    struct sockaddr_in udp_client_addr;
    socklen_t udp_client_addr_len = sizeof(udp_client_addr);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t valread = recvfrom(udp_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_addr, &udp_client_addr_len);
        if (valread < 0)
        {
            error("Error reading from UDP socket");
        }

        handle_client(udp_sockfd, tcp_sockfd, e_command, i_command, o_command, NULL);
    }

    // Clean up
    close(tcp_sockfd);
    close(udp_sockfd);
}

// This function runs the server and handles client connections
void run_server(int port, char *e_command, char *i_command, char *o_command, char *b_command)
{
    // Set up the server socket
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

    // Handle client connections
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

// This function runs the client and handles the connection
void run_client(char *hostname, int port, char *e_command, char *i_command, char *o_command, char *b_command)
{
    // Set up the client socket
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

    // Handle the client connection
    handle_client(client_sockfd, client_sockfd, e_command, i_command, o_command, b_command);

    close(client_sockfd);
}

// This function runs the UDP server
void run_udp_server(int port, char *e_command, char *i_command, int timeout)
{
    // Set up the UDP server socket
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

    // Set a timeout for the server
    alarm(timeout);

    // Handle client connections
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

// This function runs the UDP client
void run_udp_client(char *hostname, int port, char *e_command, char *o_command, int timeout)
{
    // Set up the UDP client socket
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

    // Set a timeout for the client
    alarm(timeout);

    // Handle the client connection
    int input_fd = STDIN_FILENO;
    int output_fd = sockfd;
    handle_client(input_fd, output_fd, e_command, NULL, o_command, NULL);

    close(sockfd);
}

// This function runs the Unix Domain Socket (UDS) server
void run_uds_server(char *socket_path, char *e_command, char *i_command, char *o_command, char *b_command, int is_datagram)
{
    int sockfd;
    struct sockaddr_un serv_addr;

    // Create the UDS socket based on the specified type (datagram or stream)
    if (is_datagram)
    {
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    }
    else
    {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (sockfd < 0)
    {
        error("Error opening UDS socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, socket_path, sizeof(serv_addr.sun_path) - 1);

    // Remove the socket file if it already exists
    unlink(socket_path);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error binding UDS socket");
    }

    if (!is_datagram)
    {
        if (listen(sockfd, 5) < 0)
        {
            error("Error listening on UDS socket");
        }
    }

    printf("UDS server is listening on %s\n", socket_path);

    // Handle client connections
    while (1)
    {
        int client_sockfd;
        struct sockaddr_un cli_addr;
        socklen_t cli_len = sizeof(cli_addr);

        if (is_datagram)
        {
            client_sockfd = sockfd;
        }
        else
        {
            client_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
            if (client_sockfd < 0)
            {
                error("Error accepting UDS connection");
            }
        }

        handle_client(client_sockfd, client_sockfd, e_command, i_command, o_command, b_command);

        if (!is_datagram)
        {
            close(client_sockfd);
        }
    }

    close(sockfd);
    unlink(socket_path);
}

// This function runs the Unix Domain Socket (UDS) client
void run_uds_client(char *socket_path, char *e_command, char *i_command, char *o_command, char *b_command, int is_datagram)
{
    int sockfd;
    struct sockaddr_un serv_addr;

    // Create the UDS client socket based on the specified type (datagram or stream)
    if (is_datagram)
    {
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    }
    else
    {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (sockfd < 0)
    {
        error("Error opening UDS socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, socket_path, sizeof(serv_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error connecting to UDS socket");
    }

    printf("Connected to UDS server\n");

    // Handle the client connection
    handle_client(sockfd, sockfd, e_command, i_command, o_command, b_command);

    close(sockfd);
}

// This function runs the Unix Domain Socket (UDS) server and client
void run_uds_server_client(char *server_socket_path, char *client_socket_path, char *e_command, char *i_command, char *o_command)
{
    int server_sockfd, client_sockfd;
    struct sockaddr_un serv_addr, cli_addr;
    int is_server_datagram = 0, is_client_datagram = 0;

    // Determine the socket types based on the command prefixes
    if (strncmp(i_command, "UDSSD", 5) == 0)
    {
        is_server_datagram = 1;
    }
    if (strncmp(o_command, "UDSCD", 5) == 0 || strncmp(o_command, "UDPC", 4) == 0)
    {
        is_client_datagram = 1;
    }

    // Create server socket
    if (is_server_datagram)
    {
        server_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    }
    else
    {
        server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    }
    if (server_sockfd < 0)
    {
        error("Error opening server UDS socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, server_socket_path, sizeof(serv_addr.sun_path) - 1);

    // Remove the socket file if it already exists
    unlink(server_socket_path);

    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error binding server UDS socket");
    }

    if (!is_server_datagram)
    {
        if (listen(server_sockfd, 5) < 0)
        {
            error("Error listening on server UDS socket");
        }
    }

    printf("UDS server is listening on %s\n", server_socket_path);

    // Create client socket
    if (is_client_datagram)
    {
        client_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    }
    else
    {
        client_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    }
    if (client_sockfd < 0)
    {
        error("Error opening client UDS socket");
    }

    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sun_family = AF_UNIX;
    strncpy(cli_addr.sun_path, client_socket_path, sizeof(cli_addr.sun_path) - 1);

    // Connect the client socket to the server socket
    if (connect(client_sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        error("Error connecting to client UDS socket");
    }

    printf("Connected to UDS client on %s\n", client_socket_path);

    // Accept connection from the client (if server is not datagram)
    int input_sockfd;
    if (!is_server_datagram)
    {
        input_sockfd = accept(server_sockfd, NULL, NULL);
        if (input_sockfd < 0)
        {
            error("Error accepting connection from client");
        }
    }
    else
    {
        input_sockfd = server_sockfd;
    }

    // Handle the client connection
    handle_client(input_sockfd, client_sockfd, e_command, i_command, o_command, NULL);

    // Clean up
    close(input_sockfd);
    close(client_sockfd);
    close(server_sockfd);
    unlink(server_socket_path);
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
    char *uds_server_path = NULL;
    char *uds_client_path = NULL;
    int timeout = 0;
    int is_datagram = 0;

    // Parse the command-line options
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
            else if (strncmp(i_command, "UDSSD", 5) == 0)
            {
                uds_server_path = i_command + 5;
                is_datagram = 1;
            }
            else if (strncmp(i_command, "UDSSS", 5) == 0)
            {
                uds_server_path = i_command + 5;
                is_datagram = 0;
            }
            else if (strncmp(i_command, "UDSCS", 5) == 0)
            {
                uds_client_path = i_command + 5;
                is_datagram = 0;
            }
            break;
        case 'o':
            o_command = optarg;
            if (strncmp(o_command, "UDSCD", 5) == 0)
            {
                uds_client_path = o_command + 5;
                is_datagram = 1;
            }
            else if (strncmp(o_command, "UDSCS", 5) == 0)
            {
                uds_client_path = o_command + 5;
                is_datagram = 0;
            }
            else if (strncmp(o_command, "UDSSS", 5) == 0)
            {
                uds_server_path = o_command + 5;
                is_datagram = 0;
            }
            else if (strncmp(o_command, "UDPC", 4) == 0)
            {
                udp_server_port = o_command + 4;
            }
            break;
        case 'b':
            b_command = optarg;
            if (strncmp(b_command, "TCPS", 4) == 0)
            {
                tcp_server_port = b_command + 4;
            }
            else if (strncmp(b_command, "UDSSS", 5) == 0)
            {
                uds_server_path = b_command + 5;
                is_datagram = 0;
            }
            else if (strncmp(b_command, "UDSCS", 5) == 0)
            {
                uds_client_path = b_command + 5;
                is_datagram = 0;
            }
            break;
        case 't':
            timeout = atoi(optarg);
            break;
        default:
            error("Usage: mync [-e command] [-i input_spec] [-o output_spec] [-b io_spec] [-t timeout]");
        }
    }

    // Check if the necessary options are provided
    if (tcp_server_port != NULL && o_command != NULL && strncmp(o_command, "TCPC", 4) == 0)
    {
        char *client_info = o_command + 4;
        char *client_hostname = strtok(client_info, ",");
        char *client_port_str = strtok(NULL, ",");
        if (client_hostname == NULL || client_port_str == NULL)
        {
            error("Invalid client information");
        }
        int client_port = atoi(client_port_str);
        run_server_and_client(atoi(tcp_server_port), client_hostname, client_port, e_command, i_command, o_command);
    }
    else if (udp_server_port != NULL && o_command != NULL && strncmp(o_command, "TCPC", 4) == 0)
    {
        char *client_info = o_command + 4;
        char *client_hostname = strtok(client_info, ",");
        char *client_port_str = strtok(NULL, ",");
        if (client_hostname == NULL || client_port_str == NULL)
        {
            error("Invalid client information");
        }
        int client_port = atoi(client_port_str);
        run_udp_server_tcp_client(atoi(udp_server_port), client_hostname, client_port, e_command, i_command, o_command);
    }
    else if (tcp_server_port != NULL)
    {
        int port = atoi(tcp_server_port);
        run_server(port, e_command, i_command, o_command, b_command);
    }
    else if (udp_server_port != NULL)
    {
        int port = atoi(udp_server_port);
        run_udp_server(port, e_command, i_command, timeout);
    }
    else if (uds_server_path != NULL && uds_client_path != NULL)
    {
        run_uds_server_client(uds_server_path, uds_client_path, e_command, i_command, o_command);
    }
    else if (uds_server_path != NULL)
    {
        run_uds_server(uds_server_path, e_command, i_command, o_command, b_command, is_datagram);
    }
    else if (uds_client_path != NULL)
    {
        run_uds_client(uds_client_path, e_command, i_command, o_command, b_command, is_datagram);
    }
    else if (o_command != NULL && strncmp(o_command, "UDPC", 4) == 0)
    {
        char *hostname = o_command + 4;
        char *port_str = strchr(hostname, ',');
        if (port_str == NULL)
        {
            error("Invalid UDPC format");
        }
        *port_str = '\0';
        port_str++;
        int port = atoi(port_str);
        run_udp_client(hostname, port, e_command, o_command, timeout);
    }
    else if (o_command != NULL && strncmp(o_command, "TCPC", 4) == 0)
    {
        char *hostname = o_command + 4;
        char *port_str = strchr(hostname, ',');
        if (port_str == NULL)
        {
            error("Invalid TCPC format");
        }
        *port_str = '\0';
        port_str++;
        int port = atoi(port_str);
        run_client(hostname, port, e_command, i_command, o_command, b_command);
    }
    else
    {
        error("Missing server or client information");
    }

    return 0;
}