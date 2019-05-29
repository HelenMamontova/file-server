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

Socket::Socket(int fd)
{
    s = fd;
}

Socket::~Socket()
{
    close(s);
}

// move constructor
Socket::Socket(Socket&& other)
    : s(0)
{
    s = other.s;
    other.s = 0;
}

// move asignment operator
Socket& Socket::operator = (Socket&& other)
{
    if (this != &other)
    {
        s = other.s;
        other.s = 0;
    }
    return *this;
}

int Socket::bindSocket(const sockaddr_in &addr, size_t addrlen)
{
    return bind(s, (const sockaddr*) &addr, addrlen);
}

int Socket::listenSocket(int n)
{
    return listen(s, n);
}


int Socket::sendSocket(const void *buf, size_t len, int n)
{
    return send(s, buf, len, n);
}

int Socket::recvSocket(void *buf, size_t len, int n)
{
    return recv(s, buf, len, n);
}

int Socket::connectSocket(const sockaddr_in &addr, size_t addrlen)
{
    return connect(s, (const sockaddr*) &addr, addrlen);
}

Socket Socket::acceptSocket(const sockaddr_in &addr, int addrlen)
{
    return Socket(accept(s, (sockaddr*) &addr, (socklen_t*) &addrlen));
}
