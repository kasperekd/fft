
// // TODO: add atexit
// // TODO: save state when conn. is down
// // TODO: config/argv
// // Не используем примитивы синхронизации (проверить возможные подводные)

// #include <assert.h>
// #include <errno.h>
// #include <math.h>
// #include <pthread.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #include "my_socket.h"

// #define PORT 8080
// #define PI 3.14159265358979323846
// #define FREQ 10

// typedef struct {
//     SocketServer *server;
//     double t;
//     double value;
// } ThreadData;

// // Функция для расчетов
// void *calculate(void *arg) {
//     ThreadData *data = (ThreadData *)arg;

//     while (1) {
//         double value =
//             sin(2 * PI * FREQ * data->t);  // sin(2 * PI * FREQ * data->t) +
//                                            // cos(3 * PI * FREQ * data->t);
//         data->t += 0.001;
//         data->value = value;

//         usleep(10000);  // Задержка 10 мс
//     }

//     return NULL;
// }

// void *send_data_thread(void *arg) {
//     ThreadData *data = (ThreadData *)arg;
//     char buffer[1024] = {0};

//     while (1) {
//         double value = data->value;
//         snprintf(buffer, sizeof(buffer), "%f\n", value);
//         if (send_data(data->server, buffer) < 0) {
//             perror("send failed");
//             break;
//         }

//         usleep(10000);  // Задержка 10 мс
//     }

//     return NULL;
// }

// int main() {
//     SocketServer *server = init_server(PORT);
//     if (accept_connection(server) < 0) {
//         close_server(server);
//         return EXIT_FAILURE;
//     }

//     ThreadData data;
//     data.server = server;
//     data.t = 0.0;
//     data.value = 0.0;
//     pthread_t calc_thread, send_thread;

//     // поток для расчетов
//     if (pthread_create(&calc_thread, NULL, calculate, &data) != 0) {
//         perror("Failed to create calculation thread");
//         close_server(server);
//         return EXIT_FAILURE;
//     }

//     // поток для отправки
//     if (pthread_create(&send_thread, NULL, send_data_thread, &data) != 0) {
//         perror("Failed to create send thread");
//         close_server(server);
//         return EXIT_FAILURE;
//     }

//     pthread_join(calc_thread, NULL);
//     pthread_join(send_thread, NULL);

//     close_server(server);
//     return 0;
// }

#include <assert.h>
#include <complex.h>
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
#define FFT_SIZE 1024

typedef struct {
    SocketServer *server;
    double t;
    double complex signal[FFT_SIZE];
    int ready;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} ThreadData;

void fft(double complex *x, int N) {
    int n = N;
    int logN = log2(N);
    for (int i = 0; i < n; i++) {
        int j = 0;
        for (int k = 0; k < logN; k++) {
            j = (j << 1) | ((i >> k) & 1);
        }
        if (i < j) {
            double complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    // FFT
    for (int s = 1; s <= logN; s++) {
        int m = 1 << s;  // 2^s
        double complex wm =
            cos(2 * PI / m) - sin(2 * PI / m) * I;  // e^(-2pi * i / m)

        for (int k = 0; k < n; k += m) {
            double complex w = 1;
            for (int j = 0; j < (m / 2); j++) {
                double complex t = w * x[k + j + (m / 2)];
                double complex u = x[k + j];
                x[k + j] = u + t;
                x[k + j + (m / 2)] = u - t;
                w *= wm;
            }
        }
    }
}

// Функция для расчетов
void *calculate(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int index = 0;

    while (1) {
        double value = sin(2 * PI * FREQ * data->t);
        // double value =
        //     sin(2 * PI * FREQ * data->t) + cos(3 * PI * FREQ * data->t);
        data->signal[index] = value;
        data->t += 0.001;

        index++;
        if (index >= FFT_SIZE) {
            index = 0;
            fft(data->signal, FFT_SIZE);

            pthread_mutex_lock(&data->lock);
            data->ready = 1;
            pthread_cond_signal(&data->cond);
            pthread_mutex_unlock(&data->lock);
        }

        // usleep(10000);  // Задержка 10 мс
    }

    return NULL;
}

void *send_data_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char buffer[1024];

    while (1) {
        pthread_mutex_lock(&data->lock);
        while (!data->ready) {
            pthread_cond_wait(&data->cond, &data->lock);
        }

        for (int i = 0; i < FFT_SIZE; i++) {
            snprintf(buffer, sizeof(buffer), "%f + i%f\n",
                     creal(data->signal[i]), cimag(data->signal[i]));
            printf("%s", buffer);
            if (send_data(data->server, buffer) < 0) {
                perror("send failed");
                break;
            }
        }

        data->ready = 0;
        pthread_mutex_unlock(&data->lock);
    }

    return NULL;
}

// void *send_data_thread(void *arg) {
//     ThreadData *data = (ThreadData *)arg;
//     char buffer[8192];
//     int offset;

//     while (1) {
//         pthread_mutex_lock(&data->lock);
//         while (!data->ready) {
//             pthread_cond_wait(&data->cond, &data->lock);
//         }

//         offset = 0;
//         for (int i = 0; i < FFT_SIZE; i++) {
//             offset +=
//                 snprintf(buffer + offset, sizeof(buffer) - offset, "%f %f\n",
//                          creal(data->signal[i]), cimag(data->signal[i]));
//             if (offset >= sizeof(buffer)) {
//                 fprintf(stderr,
//                         "Buffer overflow detected. Adjust buffer size.\n");
//                 break;
//             }
//         }

//         if (send_data(data->server, buffer) < 0) {
//             perror("send failed");
//             break;
//         }

//         data->ready = 0;
//         pthread_mutex_unlock(&data->lock);
//     }

//     return NULL;
// }

int main() {
    SocketServer *server = init_server(PORT);
    if (accept_connection(server) < 0) {
        close_server(server);
        return EXIT_FAILURE;
    }

    ThreadData data;
    data.server = server;
    data.t = 0.0;
    data.ready = 0;
    pthread_mutex_init(&data.lock, NULL);
    pthread_cond_init(&data.cond, NULL);
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

    // Освобождение ресурсов
    pthread_mutex_destroy(&data.lock);
    pthread_cond_destroy(&data.cond);
    close_server(server);
    return 0;
}
