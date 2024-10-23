#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen;
} SocketServer;

SocketServer *init_server(int port);
int accept_connection(SocketServer *server);
int send_data(SocketServer *server, const char *data);
void close_server(SocketServer *server);

#endif  // MY_SOCKET_H
