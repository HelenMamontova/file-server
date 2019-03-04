#include <iostream>
#include <cstring>
#include <arpa/inet.h> //inet_aton

#include "utils.h"

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
