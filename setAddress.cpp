#include <iostream>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

bool setAddress(std::string& address, struct sockaddr_in* local)
{
    std::string ip_address;
    std::string port;

    size_t pos = address.find(":");
    if (pos != std::string::npos)
    {
        ip_address = address.substr(0, pos);
        port = address.substr(pos + 1, address.length() - pos - 1);
    }
    else
    {
        std::cerr << address << " - incorrect address\n";
        return false;
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
    if (*endptr == '\0')
    {
        local->sin_port = htons(port_num);
    }
    else
    {
        std::cerr << port << " - unknown port\n";
        return false;
    }
    return true;
}
