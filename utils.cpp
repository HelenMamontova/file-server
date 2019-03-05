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

int sendString(int s, const std::string& str)
{
    uint32_t str_len = str.length();
    int res = send(s, &str_len, sizeof(str_len), 0);
    if (res < 0 || res != sizeof(str_len))
    {
        std::cerr << "Send call error string length. " << strerror(errno) << "\n";
        return 1;
    }

    res = send(s, str.c_str(), str.length(), 0);
    if (res < 0 || res != (int)str.length())
    {
        std::cerr << "Send call error string. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}
