#include "socket.h"
#include "utils.h"

#include <vector>
#include <cstring> //strerror
#include <cerrno>
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <unistd.h> //close

Socket::Socket()
    : m_sock(socket(AF_INET, SOCK_STREAM, 0))
{
    if (m_sock == -1)
        throw Error("Error creating socket. " + std::string(strerror(errno)));
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

void Socket::bind(const std::string &address)
{
    struct sockaddr_in addr;

    setAddress(address, &addr);

    bind(addr, sizeof(addr));
}

void Socket::connect(const std::string &address)
{
    struct sockaddr_in addr;

    setAddress(address, &addr);

    connect(addr, sizeof(addr));
}

void Socket::bind(const sockaddr_in &addr, size_t addrlen)
{
    if (::bind(m_sock, (const sockaddr*) &addr, addrlen))
        throw Error("Error bind. " + std::string(strerror(errno)));
}

void Socket::listen(int n)
{
    if (::listen(m_sock, n))
        throw Error("Error listen. " + std::string(strerror(errno)));
}

size_t Socket::send(const void *buf, size_t len, int n)
{
    int res = ::send(m_sock, buf, len, n);
    if (res < 0)
        throw Error("Error send. " + std::string(strerror(errno)));
    return res;
}

size_t Socket::recv(void *buf, size_t len, int n)
{
    int res = ::recv(m_sock, buf, len, n);
    if (res < 0)
        throw Error("Error recv. " + std::string(strerror(errno)));
    return res;
}

void Socket::connect(const sockaddr_in &addr, size_t addrlen)
{
    if (::connect(m_sock, (const sockaddr*) &addr, addrlen))
        throw Error("Error connect. " + std::string(strerror(errno)));
}

Socket Socket::accept(sockaddr_in &addr, socklen_t &addrlen)
{
    int fd = ::accept(m_sock, (sockaddr*) &addr, &addrlen);
    if (fd == -1)
        throw Error("Error accept. " + std::string(strerror(errno)));
    return Socket(fd);
}

void Socket::sendString(const std::string& source)
{
    uint32_t length = source.length();
    if (send(&length, sizeof(length), 0) != sizeof(length))
        throw Socket::Error("Error sendString: Cannot send string length.");

    if (send(source.c_str(), source.length(), 0) != source.length())
        throw Socket::Error("Error sendString: Cannot send string data.");
}

std::string Socket::receiveString()
{
    uint32_t length;
    if (recv(&length, sizeof(length), 0) != sizeof(length))
        throw Socket::Error("Error receiveString: Cannot receive string length.");

    std::vector<char> str(length);
    if (recv(str.data(), length, 0) != length)
        throw Socket::Error("Error receiveString: Cannot receive string data.");
    std::string destination;
    return destination.assign(str.begin(), str.end());
}

void Socket::sendUint8(uint8_t source)
{
    if (send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint8: Cannot send uint8_t data.");
}

uint8_t Socket::receiveUint8()
{
    uint8_t destination;
    if (recv(&destination, sizeof(destination), 0) != sizeof(destination))
        throw Socket::Error("Error receiveUint8: Cannot receive uint8_t data.");
    return destination;
}

void Socket::sendUint32(uint32_t source)
{
    if (send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint32: Cannot send uint32_t data.");
}

uint32_t Socket::receiveUint32()
{
    uint32_t destination;
    if (recv(&destination, sizeof(destination), 0) != sizeof(destination))
        throw Socket::Error("Errorr receiveUint32: Cannot receive uint32_t data.");
    return destination;
}
