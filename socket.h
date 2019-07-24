#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <stdexcept>
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in

class Socket
{
private:
    int m_sock;
    Socket(const Socket&);
    Socket& operator = (const Socket&);

    Socket(int fd);

public:
    Socket();
    ~Socket();

    Socket(Socket&& other);
    Socket& operator = (Socket&& other);

    size_t bind(const std::string &address);
    int connect(const std::string &address);

    void bind(const sockaddr_in &addr, size_t addrlen);
    void listen(int n);
    size_t send(const void *buf, size_t len, int n);
    int recv(void *buf, size_t len, int n);
    int connect(const sockaddr_in &addr, size_t addrlen);
    Socket accept(sockaddr_in &addr, socklen_t &addrlen);

    class Error: public std::runtime_error
    {
    public:
       explicit Error(std::string error_message) noexcept
        : runtime_error(error_message)
        {
        }
    };
};

#endif
