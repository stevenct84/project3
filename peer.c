#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define CHUNK_SIZE 1024

void *send_file(void *arg) {
    int client_sock;
    struct sockaddr_in server_addr;
    char *filename = "archivo_a_enviar.txt";
    char *server_ip = " 172.24.64.1"; // IP de la segunda computadora

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        fclose(file);
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Dirección inválida/ no soportada");
        close(client_sock);
        fclose(file);
        return NULL;
    }

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en la conexión");
        close(client_sock);
        fclose(file);
        return NULL;
    }

    char buffer[CHUNK_SIZE] = {0};
    while (fgets(buffer, CHUNK_SIZE, file) != NULL) {
        if (send(client_sock, buffer, sizeof(buffer), 0) == -1) {
            perror("Error al enviar el archivo");
            close(client_sock);
            fclose(file);
            return NULL;
        }
        memset(buffer, 0, CHUNK_SIZE);
    }

    printf("Archivo enviado con éxito\n");
    close(client_sock);
    fclose(file);
    return NULL;
}

void *receive_file(void *arg) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char *filename = "archivo_recibido.txt";

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        fclose(file);
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en el bind");
        close(server_sock);
        fclose(file);
        return NULL;
    }

    if (listen(server_sock, 3) < 0) {
        perror("Error en el listen");
        close(server_sock);
        fclose(file);
        return NULL;
    }

    printf("Esperando conexiones...\n");
    if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Error en el accept");
        close(server_sock);
        fclose(file);
        return NULL;
    }

    char buffer[CHUNK_SIZE] = {0};
    int bytes_received = 0;
    while ((bytes_received = recv(client_sock, buffer, CHUNK_SIZE, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, file);
        memset(buffer, 0, CHUNK_SIZE);
    }

    if (bytes_received < 0) {
        perror("Error al recibir el archivo");
    } else {
        printf("Archivo recibido con éxito\n");
    }

    close(client_sock);
    close(server_sock);
    fclose(file);
    return NULL;
}

int main() {
    pthread_t send_thread, receive_thread;

    if (pthread_create(&send_thread, NULL, send_file, NULL) != 0) {
        perror("Error al crear el hilo de envío");
        return EXIT_FAILURE;
    }

    if (pthread_create(&receive_thread, NULL, receive_file, NULL) != 0) {
        perror("Error al crear el hilo de recepción");
        return EXIT_FAILURE;
    }

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    return 0;
}
