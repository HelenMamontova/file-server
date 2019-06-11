#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
private:
    int s;
    Socket(const Socket&);
    Socket& operator = (const Socket&);

public:
    Socket();
    ~Socket();

    Socket(int fd);

    Socket(Socket&& other);
    Socket& operator = (Socket&& other);

    int bindSocket(const sockaddr_in &addr, size_t addrlen);
    int listenSocket(int n);
    int sendSocket(const void *buf, size_t len, int n);
    int recvSocket(void *buf, size_t len, int n);
    int connectSocket(const sockaddr_in &addr, size_t addrlen);
    Socket acceptSocket(const sockaddr_in &addr, int addrlen);
};

#endif
