#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
    private:
    int s;
    int s1;
    Socket(const Socket&);
    Socket operator = (const Socket&);

    public:
    Socket();
    ~Socket();

    int bindSocket(const sockaddr_in &addr, size_t addrlen);
    int listenSocket(int n);
};

#endif
