#include <iostream>
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in
#include <errno.h>
#include <unistd.h> //close

#include "socket.h"


Socket::Socket()
    : m_var(socket(AF_INET, SOCK_STREAM, 0))
{
}

Socket::Socket(int fd)
    : m_var(fd)
{
}

Socket::~Socket()
{
    close(m_var);
}

// move constructor
Socket::Socket(Socket&& other)
    : m_var(0)
{
    m_var = other.m_var;
    other.m_var = 0;
}

// move asignment operator
Socket& Socket::operator = (Socket&& other)
{
    if (this != &other)
    {
        m_var = other.m_var;
        other.m_var = 0;
        return *this;
    }
    return *this;
}

int Socket::bind(const sockaddr_in &addr, size_t addrlen)
{
    return ::bind(m_var, (const sockaddr*) &addr, addrlen);
}

int Socket::listen(int n)
{
    return ::listen(m_var, n);
}

int Socket::send(const void *buf, size_t len, int n)
{
    return ::send(m_var, buf, len, n);
}

int Socket::recv(void *buf, size_t len, int n)
{
    return ::recv(m_var, buf, len, n);
}

int Socket::connect(const sockaddr_in &addr, size_t addrlen)
{
    return ::connect(m_var, (const sockaddr*) &addr, addrlen);
}

Socket Socket::accept(const sockaddr_in &addr, int addrlen)
{
    return Socket(::accept(m_var, (sockaddr*) &addr, (socklen_t*) &addrlen));
}
