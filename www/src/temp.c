#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define RESPONSE_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
} connection_t;

void *handle_connection(void *arg) {
    connection_t *conn = (connection_t *)arg;
    char *response_body = "<html><body><h1>Hello, world!</h1></body></html>";

    char buffer[30000] = {0};
    read(conn->socket_fd, buffer, 30000);
    printf("%s\n", buffer);

    char response[35000] = {0};
    strcat(response, RESPONSE_HEADER);
    strcat(response, response_body);
    send(conn->socket_fd, response, strlen(response), 0);
    close(conn->socket_fd);

    free(conn);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Crear socket del servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asignar dirección al socket del servidor
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar por nuevas conexiones
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Configurar threadpool
    int num_threads = 4;
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    int i;
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, &handle_connection, NULL);
    }

    // Esperar por nuevas conexiones y enviar respuesta
    while (1) {
        int new_socket;
        struct sockaddr_in client_address;
        int client_addrlen = sizeof(client_address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&client_addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        connection_t *conn = malloc(sizeof(connection_t));
        conn->socket_fd = new_socket;
        conn->address = client_address;

        // Agregar conexión al threadpool
        for (i = 0; i < num_threads; i++) {
            if (pthread_tryjoin_np(threads[i], NULL) == 0) {
                pthread_create(&threads[i], NULL, &handle_connection, (void *)conn);
                break;
            }
        }

        if (i == num_threads) {
            printf("Threadpool is full, cannot accept new connection.\n");
            close(new_socket);
            free(conn);
        }
    }

    return 0;
}
