#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

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

int main() {
    char ip[INET_ADDRSTRLEN];
    get_ip("eth0", ip); // Reemplaza "eth0" con el nombre de tu interfaz de red
    printf("Server IP Address: %s\n", ip);
    return 0;
}
