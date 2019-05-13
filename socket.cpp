#include <iostream>
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in
#include <errno.h>
#include <unistd.h> //close

#include "socket.h"


Socket::Socket()
{
    s = socket(AF_INET, SOCK_STREAM, 0);
}

Socket::~Socket()
{
    close(s);
}

int Socket::bindSocket(const sockaddr_in &addr, size_t addrlen)
{
    return bind(s, (const sockaddr*) &addr, addrlen);
}

int Socket::listenSocket(int n)
{
    return listen(s, n);
}
