// Exercise 3 - Server Side

#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 10
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

//array for the sockets and count the number of actual clients
int client_sockets[MAX_CLIENTS];
int client_count = 0;

//send message to all the clients connected to the server
void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&client_mutex);
	for (int i = 0; i < client_count; i++) {
		//verify to dont sent the message to the client owner
		if (client_sockets[i] != exclude_socket) {
            send(client_sockets[i], message, strlen(message),0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}


//create a new client and recive messages 
void *handle_client(void *arg)
{
	int client_socket = *(int *)arg;
	int read_size;

	// Manage communication with the client
	while (1)
	{
		char buffer[1024];
		bzero(buffer, 1024);
		int read_size=recv(client_socket, buffer, 1024, 0);
		
		if(read_size>0){
			printf("Received: %s\n", buffer);
			broadcast_message(buffer, client_socket);
		}
		else
		{
			printf("Disconnected client\n");
			break;
		}
	}

	

	close(client_socket);
}

int main()
{
	// Initialize variables
	int serverSocket, newSocket, *client_socket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;

	socklen_t addr_size;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);

	// Bind the socket to the address and port.
	bind(serverSocket,
		 (struct sockaddr *)&serverAddr,
		 sizeof(serverAddr));

	// Listen on the socket,
	if (listen(serverSocket, 50) == 0)
	{
		printf("Listening\n");

		while (1)
		{
			addr_size = sizeof(serverStorage);

			// Extract the client
			newSocket = accept(serverSocket,
							   (struct sockaddr *)&serverStorage,
							   &addr_size);

			pthread_mutex_lock(&client_mutex);
			if (client_count < MAX_CLIENTS)
			{
				//add to the clients array
				client_sockets[client_count++] = newSocket;
			}
			else
			{
				printf("Maximum number of clients reached. Connection refused.\n");
				close(newSocket);
				pthread_mutex_unlock(&client_mutex);
				continue;
			}
			pthread_mutex_unlock(&client_mutex);

			pthread_t client_thread;
			client_socket = malloc(sizeof(int));
			*client_socket = newSocket;
			//thread for handle a new client
			pthread_create(&client_thread, NULL, handle_client, (void *)client_socket);
		}
	}
	else 
		printf("Error\n");

	return 0;
}
