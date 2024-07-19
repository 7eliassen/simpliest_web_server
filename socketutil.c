#include "socketutil.h"


int createTCPIpv4Socket() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    return sfd;
}

struct sockaddr_in* createIpv4Address(char* ip, unsigned int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}
