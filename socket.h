#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in

class Socket
{
private:
    int m_var;
    Socket(const Socket&);
    Socket& operator = (const Socket&);

    Socket(int fd);

public:
    Socket();
    ~Socket();

    Socket(Socket&& other);
    Socket& operator = (Socket&& other);

    int bind(const sockaddr_in &addr, size_t addrlen);
    int listen(int n);
    int send(const void *buf, size_t len, int n);
    int recv(void *buf, size_t len, int n);
    int connect(const sockaddr_in &addr, size_t addrlen);
    Socket accept(sockaddr_in &addr, int addrlen);
};

#endif
