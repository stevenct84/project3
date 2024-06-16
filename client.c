// Exercise 3 - Client Side

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>
#define BUFFER_SIZE 1024

int network_socket;
int flag=0;//flag to finish the client

void get_ip(char *iface, char *ip) {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    // Specify the interface name
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    // Convert the address to a string
    strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}


//Send message to the server
void send_msg()
{
    while (1) {
        char buffer[BUFFER_SIZE];
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            // delete \n at the end of the string
            buffer[strcspn(buffer, "\n")] = '\0';
        }
        //if the string is exit, break and activate the falg
        if (strcmp(buffer, "exit") == 0)
        {
            flag=1;//activate the flag to exit the client
            break;
        }
        //send the buffer(string) to the server
        send(network_socket, buffer, strlen(buffer), 0);
        // clean the buffer
        memset(buffer, 0, BUFFER_SIZE);
    }
}

//recive message from the server
void recive_msg()
{
    while(1){
        // Receive a response
        char buffer[BUFFER_SIZE];
        int receive=recv(network_socket, buffer, sizeof(buffer), 0);

        if (receive >0){   
            // Print the response
            printf("Received: %s\n", buffer); 
        }
        //clean the buffer
        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main()
{
    //two threads for handle send and recive messages from server
    pthread_t send_msg_thread, recv_msg_thread;
    char server_ip[INET_ADDRSTRLEN];
    get_ip("eth0", server_ip);

    // Create a stream socket
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Initialise port number and address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip); 
    server_address.sin_port = htons(8000);

    // Initiate a socket connection
    int connection_status = connect(network_socket,
                                    (struct sockaddr *)&server_address,
                                    sizeof(server_address));

    // Check for connection error
    if (connection_status < 0)
    {
        puts("Error connecting \n");
        return 0;
    }
    //connection works well
    printf("Connection established\n");
    printf("Write a message: \n");

    //start the thread for send messages to the server
    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg, NULL) != 0)
    {
        printf("ERROR: thread\n");
        return EXIT_FAILURE;
    }

    //start the thread for recive messages of the server
    if (pthread_create(&recv_msg_thread, NULL, (void *)recive_msg, NULL) != 0)
    {
        printf("ERROR: thread\n");
        return EXIT_FAILURE;
    }

    while(1){//run the program until the flag is activated
        if(flag){
            break;
        }
    }
    //close socket connection
    close(network_socket);
	return 0;
 
}
