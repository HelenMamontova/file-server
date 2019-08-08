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
        throw Socket::Error("Error sendString: Cannot send string length.");

    if (sock.send(source.c_str(), source.length(), 0) != source.length())
        throw Socket::Error("Error sendString: Cannot send string data.");
}

void receiveString(Socket& sock, std::string& destination)
{
    uint32_t length;
    if (sock.recv(&length, sizeof(length), 0) != sizeof(length))
        throw Socket::Error("Error receiveString: Cannot receive string length.");

    std::vector<char> str(length);
    if (sock.recv(str.data(), length, 0) != length)
        throw Socket::Error("Error receiveString: Cannot receive string data.");

    destination.assign(str.begin(), str.end());
}

void sendUint8(Socket& sock, uint8_t source)
{
    if (sock.send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint8: Cannot send uint8_t data.");
}

void receiveUint8(Socket& sock, uint8_t& destination)
{
    if (sock.recv(&destination, sizeof(destination), 0) != sizeof(destination))
        throw Socket::Error("Error receiveUint8: Cannot receive uint8_t data.");
}

void sendUint32(Socket& sock, uint32_t source)
{
    if (sock.send(&source, sizeof(source), 0) != sizeof(source))
        throw Socket::Error("Error sendUint32: Cannot send uint32_t data.");
}

void receiveUint32(Socket& sock, uint32_t& destination)
{
    if (sock.recv(&destination, sizeof(destination), 0) != sizeof(destination))
        throw Socket::Error("Errorr receiveUint32: Cannot receive uint32_t data.");
}
