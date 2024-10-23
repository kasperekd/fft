// my_socket.c
#include "my_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SocketServer *init_server(int port) {
    SocketServer *server = (SocketServer *)malloc(sizeof(SocketServer));
    if (!server) {
        perror("Failed to allocate memory for server");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    if ((server->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        free(server);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt failed");
        free(server);
        close(server->server_fd);
        exit(EXIT_FAILURE);
    }

    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);
    server->addrlen = sizeof(server->address);

    if (bind(server->server_fd, (struct sockaddr *)&server->address,
             sizeof(server->address)) < 0) {
        perror("bind failed");
        free(server);
        close(server->server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server->server_fd, 3) < 0) {
        perror("listen failed");
        free(server);
        close(server->server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);
    return server;
}

int accept_connection(SocketServer *server) {
    server->client_socket =
        accept(server->server_fd, (struct sockaddr *)&server->address,
               (socklen_t *)&server->addrlen);
    if (server->client_socket < 0) {
        perror("accept failed");
        return -1;
    }
    return 0;
}

int send_data(SocketServer *server, const char *data) {
    return send(server->client_socket, data, strlen(data), 0);
}

void close_server(SocketServer *server) {
    if (server->client_socket) {
        close(server->client_socket);
    }
    if (server->server_fd) {
        close(server->server_fd);
    }
    free(server);
}
