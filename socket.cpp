#include "socket.h"

#include <vector>
#include <cstring> //strerror
#include <cerrno>
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <unistd.h> //close
#include <arpa/inet.h> //inet_aton

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

sockaddr_in Socket::makeAddress(const std::string& address)
{
    size_t pos = address.find(":");
    if (pos == std::string::npos)
        throw Socket::Error("Error makeAddress: Incorrect address: " + address);

    std::string ip_address = address.substr(0, pos);
    std::string port = address.substr(pos + 1, address.length() - pos - 1);

    struct sockaddr_in local;
    local.sin_family = AF_INET;

    if (!inet_aton(ip_address.c_str(), &local.sin_addr))
        throw Socket::Error("Error makeAddress: Unknown host: " + ip_address);

    char* endptr;
    short port_num = strtol(port.c_str(), &endptr, 10);
    if (*endptr != '\0')
        throw Socket::Error("Error makeAddress: Unknown port: " + port);

    local.sin_port = htons(port_num);
    return local;
}

void Socket::bind(const std::string &address)
{
    bind(makeAddress(address));
}

void Socket::connect(const std::string &address)
{
    connect(makeAddress(address));
}

void Socket::bind(const sockaddr_in &addr)
{
    if (::bind(m_sock, (const sockaddr*) &addr, sizeof(addr)))
        throw Error("Error bind. " + std::string(strerror(errno)));
}

void Socket::connect(const sockaddr_in &addr)
{
    if (::connect(m_sock, (const sockaddr*) &addr, sizeof(addr)))
        throw Error("Error connect. " + std::string(strerror(errno)));
}

void Socket::listen(int n)
{
    if (::listen(m_sock, n))
        throw Error("Error listen. " + std::string(strerror(errno)));
}

void Socket::send(const void *buf, size_t len, int n)
{
    int cnt = len;
    const char* buffer = static_cast<const char*>(buf);
    while (cnt > 0)
    {
        int res = ::send(m_sock, buffer, cnt, n);
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            throw Error("Error send. " + std::string(strerror(errno)));
        }
        buffer += res;
        cnt -= res;
    }
}

void Socket::recv(void *buf, size_t len, int n)
{
    int cnt = len;
    char* buffer = static_cast<char*>(buf);
    while (cnt > 0)
    {
        int res = ::recv(m_sock, buffer, cnt, n);
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            throw Error("Error recv. " + std::string(strerror(errno)));
        }
        buffer += res;
        cnt -= res;
    }
}

Socket Socket::accept(sockaddr_in &addr)
{
    socklen_t addrlen = sizeof(addr);
    int fd = ::accept(m_sock, (sockaddr*) &addr, &addrlen);
    if (fd == -1)
        throw Error("Error accept. " + std::string(strerror(errno)));
    return Socket(fd);
}

void Socket::sendString(const std::string& source)
{
    uint32_t length = source.length();
    send(length);
    send(source.c_str(), source.length(), 0);
}

std::string Socket::receiveString()
{
    uint32_t length = receive<uint32_t>();

    std::vector<char> str(length);
    recv(str.data(), length, 0);

    std::string destination;
    return destination.assign(str.begin(), str.end());
}
