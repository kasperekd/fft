
// TODO: add atexit
// TODO: save state when conn. is down
// TODO: config/argv
// Не используем примитивы синхронизации (проверить возможные подводные)

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_socket.h"

#define PORT 8080
#define PI 3.14159265358979323846
#define FREQ 10

typedef struct {
    SocketServer *server;
    double t;
    double value;
} ThreadData;

// Функция для расчетов
void *calculate(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    while (1) {
        double value =
            sin(2 * PI * FREQ * data->t) + cos(3 * PI * FREQ * data->t);
        data->t += 0.001;
        data->value = value;

        usleep(10000);  // Задержка 10 мс
    }

    return NULL;
}

void *send_data_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char buffer[1024] = {0};

    while (1) {
        double value = data->value;
        snprintf(buffer, sizeof(buffer), "%f\n", value);
        if (send_data(data->server, buffer) < 0) {
            perror("send failed");
            break;
        }

        usleep(10000);  // Задержка 10 мс
    }

    return NULL;
}

int main() {
    SocketServer *server = init_server(PORT);
    if (accept_connection(server) < 0) {
        close_server(server);
        return EXIT_FAILURE;
    }

    ThreadData data;
    data.server = server;
    data.t = 0.0;
    data.value = 0.0;
    pthread_t calc_thread, send_thread;

    // поток для расчетов
    if (pthread_create(&calc_thread, NULL, calculate, &data) != 0) {
        perror("Failed to create calculation thread");
        close_server(server);
        return EXIT_FAILURE;
    }

    // поток для отправки
    if (pthread_create(&send_thread, NULL, send_data_thread, &data) != 0) {
        perror("Failed to create send thread");
        close_server(server);
        return EXIT_FAILURE;
    }

    pthread_join(calc_thread, NULL);
    pthread_join(send_thread, NULL);

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
