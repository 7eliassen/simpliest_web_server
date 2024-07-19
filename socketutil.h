#ifndef SOCKETUTIL_H_INCLUDED
#define SOCKETUTIL_H_INCLUDED


#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>


int createTCPIpv4Socket();
struct sockaddr_in* createIpv4Address(char* ip, unsigned int port);


#endif // SOCKETUTIL_H_INCLUDED
