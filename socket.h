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

    void bind(const sockaddr_in &addr);
    void connect(const sockaddr_in &addr);
    void listen(int n);
    void send(const void *buf, size_t len, int n);
    void recv(void *buf, size_t len, int n);
    Socket accept(sockaddr_in &addr);

    void sendString(const std::string& source);
    std::string receiveString();
    template <typename T>
    void send(T source)
    {
        send(&source, sizeof(source), 0);
    }

    template <typename T>
    T receive()
    {
        T destination;
        recv(&destination, sizeof(destination), 0);
        return destination;
    }

    static sockaddr_in makeAddress(const std::string& address);

    struct Error: public std::runtime_error
    {
       explicit Error(const std::string& error_message) noexcept
        : runtime_error(error_message)
        {
        }
    };
};

#endif
