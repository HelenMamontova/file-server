#include "utils.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h> //inet_aton

bool setAddress(const std::string& address, struct sockaddr_in* local)
{
    size_t pos = address.find(":");
    if (!(pos != std::string::npos))
    {
        std::cerr << address << " - incorrect address\n";
        return false;
    }
    std::string ip_address = address.substr(0, pos);
    std::string port = address.substr(pos + 1, address.length() - pos - 1);

    bzero(local, sizeof (*local));
    local->sin_family = AF_INET;

    if (!inet_aton(ip_address.c_str(), &local->sin_addr))
    {
        std::cerr << ip_address << " - unknown host\n";
        return false;
    }

    char* endptr;
    short port_num = strtol(port.c_str(), &endptr, 10);
    if (*endptr != '\0')
    {
        std::cerr << port << " - unknown port\n";
        return false;
    }
    local->sin_port = htons(port_num);

    return true;
}

void sendString(Socket& sock, const std::string& source)
{
    uint32_t length = source.length();
    if (sock.send(&length, sizeof(length), 0) != sizeof(length))
        throw Socket::Error("Error sendString.");

    if (sock.send(source.c_str(), source.length(), 0) != source.length())
        throw Socket::Error("Error sendString.");
}

int receiveString(Socket& sock, std::string& destination)
{
    uint32_t length;
    int res = sock.recv(&length, sizeof(length), 0);
    if (res < 0 || res != sizeof(length))
    {
        std::cerr << "Cannot receive string length. " << strerror(errno) << "\n";
        return 1;
    }

    std::vector<char> str(length);
    res = sock.recv(str.data(), length, 0);
    if (res < 0 || res != (int)length)
    {
        std::cerr << "Cannot receive string data. " << strerror(errno) << "\n";
        return 1;
    }
    destination.assign(str.begin(), str.end());
    return 0;
}

void sendUint8(Socket& sock, uint8_t source)
{
    if (sock.send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint8.");
}

int receiveUint8(Socket& sock, uint8_t& destination)
{
    int res = sock.recv(&destination, sizeof(destination), 0);
    if (res < 0 || res != sizeof(destination))
    {
        std::cerr << "Cannot receive uint8_t data. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

void sendUint32(Socket& sock, uint32_t source)
{
    if (sock.send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint32.");
}

int receiveUint32(Socket& sock, uint32_t& destination)
{
    int res = sock.recv(&destination, sizeof(destination), 0);
    if (res < 0 || res != sizeof(destination))
    {
        std::cerr << "Cannot receive uint32_t data. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}
