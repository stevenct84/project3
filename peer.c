#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8081
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
} peer_t;

void send_file(int socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("No se pudo abrir el archivo");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(socket, buffer, bytesRead, 0) < 0) {
            perror("Error al enviar archivo");
            break;
        }
    }
    fclose(file);
}

void receive_file(int socket, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("No se pudo abrir el archivo");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;
    while ((bytesReceived = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
    }
    fclose(file);
}


void *handle_client(void *arg) {
    peer_t peer = *(peer_t *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    recv(peer.socket, buffer, BUFFER_SIZE, 0);
    printf("Solicitud de archivo recibida: %s\n", buffer);

    send_file(peer.socket, buffer);

    close(peer.socket);
    pthread_exit(NULL);
}

void start_server() {
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error en bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Error en listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor en espera de conexiones...\n");

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }

        pthread_t thread;
        peer_t *peer = malloc(sizeof(peer_t));
        peer->socket = client_socket;
        peer->address = client_address;

        pthread_create(&thread, NULL, handle_client, peer);
        pthread_detach(thread);
    }

    close(server_socket);
}

void request_file(const char *peer_ip, const char *filename) {
    int client_socket;
    struct sockaddr_in peer_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    peer_address.sin_family = AF_INET;
    peer_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, peer_ip, &peer_address.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&peer_address, sizeof(peer_address)) < 0) {
        perror("Error en connect");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    send(client_socket, filename, strlen(filename), 0);
    receive_file(client_socket, filename);

    close(client_socket);
}


int main() {
    pthread_t server_thread;

    pthread_create(&server_thread, NULL, (void *(*)(void *))start_server, NULL);
    pthread_detach(server_thread);

    char peer_ip[16];
    char filename[256];

    while (1) {
        printf("Ingrese la IP del peer y el nombre del archivo (IP archivo): ");
        scanf("%s %s", peer_ip, filename);
        request_file(peer_ip, filename);
    }

    return 0;
}
