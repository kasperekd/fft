
// TODO: add atexit
// TODO: save state when conn. is down

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _XOPEN_SOURCE 500
#define __USE_MISC
#include <unistd.h>

#include "my_socket.h"

#define PORT 8080
#define PI 3.14159265358979323846
#define FREQ 10

int main() {
    SocketServer *server = init_server(8080);
    if (accept_connection(server) < 0) {
        close_server(server);
        return EXIT_FAILURE;
    }

    double t = 0.0;
    char buffer[1024];
    while (1) {
        double value = sin(2 * PI * FREQ * t);
        t += 0.01;
        snprintf(buffer, sizeof(buffer), "%f\n", value);
        if (send_data(server, buffer) < 0) {
            perror("send failed");
            break;
        }

        usleep(100000);  // Задержка 100 мс
    }

    close_server(server);
    return 0;
}

// #define PI 3.14159265358979323846
// #define SIZE 100000
// #define FREQ 1000

// int main() {
//     double buffer[SIZE] = {0};

//     for (size_t i = 0; i < SIZE; i++) {
//         double t = (double)i / SIZE;
//         buffer[i] = sin(2 * PI * FREQ * t);
//     }

//     for (size_t i = 0; i < SIZE; i++) {
//         printf("%ld = %f\n", i, buffer[i]);
//     }

//     return EXIT_SUCCESS;
// }
