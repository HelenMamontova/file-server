#include "utils.h"

#include <cstring>
#include <arpa/inet.h> //inet_aton

void makeAddress(const std::string& address, struct sockaddr_in* local)
{
    size_t pos = address.find(":");
    if (pos == std::string::npos)
        throw Socket::Error("Error makeAddress: Incorrect address: " + address);

    std::string ip_address = address.substr(0, pos);
    std::string port = address.substr(pos + 1, address.length() - pos - 1);

    bzero(local, sizeof (*local));
    local->sin_family = AF_INET;

    if (!inet_aton(ip_address.c_str(), &local->sin_addr))
        throw Socket::Error("Error makeAddress: Unknown host: " + ip_address);

    char* endptr;
    short port_num = strtol(port.c_str(), &endptr, 10);
    if (*endptr != '\0')
        throw Socket::Error("Error makeAddress: Unknown port: " + port);

    local->sin_port = htons(port_num);
}
