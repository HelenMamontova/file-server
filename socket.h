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

    void bind(const std::string &address);
    void connect(const std::string &address);

    void bind(const sockaddr_in &addr, size_t addrlen);
    void listen(int n);
    size_t send(const void *buf, size_t len, int n);
    size_t recv(void *buf, size_t len, int n);
    void connect(const sockaddr_in &addr, size_t addrlen);
    Socket accept(sockaddr_in &addr, socklen_t &addrlen);

    void sendString(const std::string& source);
    std::string receiveString();
    void sendUint8(uint8_t source);
    uint8_t receiveUint8();
    void sendUint32(uint32_t source);
    uint32_t receiveUint32();


    struct Error: public std::runtime_error
    {
       explicit Error(const std::string& error_message) noexcept
        : runtime_error(error_message)
        {
        }
    };
};

#endif
