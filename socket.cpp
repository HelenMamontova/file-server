#include "socket.h"
#include "utils.h"

#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <unistd.h> //close

Socket::Socket()
    : m_sock(socket(AF_INET, SOCK_STREAM, 0))
{
}

Socket::Socket(int fd)
    : m_sock(fd)
{
}

Socket::~Socket()
{
    if (m_sock != 0)
        close(m_sock);
}

// move constructor
Socket::Socket(Socket&& other)
    : m_sock(other.m_sock)
{
    other.m_sock = 0;
}

// move asignment operator
Socket& Socket::operator = (Socket&& other)
{
    if (this != &other)
    {
        m_sock = other.m_sock;
        other.m_sock = 0;
        return *this;
    }
    return *this;
}

int Socket::bind(const std::string &address)
{
    struct sockaddr_in addr;

    if (!setAddress(address, &addr))
        return 1;

    if (bind(addr, sizeof(addr)))
        return 1;
    return 0;
}

int Socket::connect(const std::string &address)
{
    struct sockaddr_in addr;

    if (!setAddress(address, &addr))
        return 1;

    if (connect(addr, sizeof(addr)))
        return 1;
    return 0;
}

int Socket::bind(const sockaddr_in &addr, size_t addrlen)
{
    return ::bind(m_sock, (const sockaddr*) &addr, addrlen);
}

int Socket::listen(int n)
{
    return ::listen(m_sock, n);
}

int Socket::send(const void *buf, size_t len, int n)
{
    return ::send(m_sock, buf, len, n);
}

int Socket::recv(void *buf, size_t len, int n)
{
    return ::recv(m_sock, buf, len, n);
}

int Socket::connect(const sockaddr_in &addr, size_t addrlen)
{
    return ::connect(m_sock, (const sockaddr*) &addr, addrlen);
}

Socket Socket::accept(sockaddr_in &addr, socklen_t &addrlen)
{
    return Socket(::accept(m_sock, (sockaddr*) &addr, &addrlen));
}
