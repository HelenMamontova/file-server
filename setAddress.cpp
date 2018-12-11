#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h> //exit
#include <netinet/in.h> //struct sockaddr_in
#include <arpa/inet.h> //inet_aton

#include "setaddress.h"

bool setAddress(const std::string& address, struct sockaddr_in* local)
{
    std::string ip_address;
    std::string port;

    size_t pos = address.find(":");
    if (!(pos != std::string::npos))
    {
        std::cerr << address << " - incorrect address\n";
        return false;
    }
    else
    {
        ip_address = address.substr(0, pos);
        port = address.substr(pos + 1, address.length() - pos - 1);
    }

    bzero(local, sizeof (*local));
    local->sin_family = AF_INET;

    if (!inet_aton(ip_address.c_str(), &local->sin_addr))
    {
        std::cerr << ip_address << " - unknown host\n";
        return false;
    }

    char* endptr;
    short port_num = strtol(port.c_str(), &endptr, 0);
    if (*endptr != '\0')
    {
        std::cerr << port << " - unknown port\n";
        return false;
    }
    else
    {
        local->sin_port = htons(port_num);
    }
    return true;
}
