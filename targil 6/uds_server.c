#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void uds_datagram_server(const char *socket_path)
{
    int sockfd;
    struct sockaddr_un serv_addr;
    char buffer[BUFFER_SIZE];
    socklen_t cli_len;
    ssize_t num_bytes;

    // Create a Unix domain socket
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, socket_path);

    // Remove any existing socket file
    unlink(socket_path);
    cli_len = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    // Bind the socket to the specified path
    if (bind(sockfd, (struct sockaddr *)&serv_addr, cli_len) < 0)
    {
        error("Error binding socket");
    }

    printf("UDS datagram server is listening on %s\n", socket_path);

    // Receive data from the client and display it on the server's stdout
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        num_bytes = read(sockfd, buffer, sizeof(buffer));
        if (num_bytes < 0)
        {
            error("Error receiving data");
            
        }

        buffer[num_bytes++] = '\0';
        if(write(1, buffer, num_bytes) < 0){
            error("Error writing to stdout");
            
        }
    }

    close(sockfd);
}

int main()
{
    const char *socket_path = "/tmp/echo_socket";
    uds_datagram_server(socket_path);
    return 0;
}