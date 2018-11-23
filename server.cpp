#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void reference();
int setAddress(std::string& address, struct sockaddr_in* local);

int main(int argc, char* argv[])
{
    std::string version = "1.0";
    std::string address;
    std::string path;

    struct sockaddr_in local;

    if (argc < 2)
    {
        reference();
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        if ((!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bind")))
        {
            if (i + 1 == argc)
            {
                std::cerr << argv[i] << " needs and argument - an address.\n";
                return 1;
            }
            address = argv[++i];
        }
        else if ((!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir")))
        {
            if (i + 1 == argc)
            {
                std::cerr << argv[i] << " needs and argument - a path to files.\n";
                return 1;
            }
            path = argv[++i];
        }
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
        {
            std::cout << "Version " << version << "\n";
            return 0;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            reference();
            return 0;
        }
        else
        {
            std::cerr << "Unknown command line argument\n";
            return 1;
        }
    }

    setAddress(address, &local);

    return 0;
}

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "server -b/--bind <ip_address:port> -d/--dir <path> [-v/--version] [-h/--help]\n\n";

    std::cout << "--bind, -b <ip_address:port> - specifies local IP address and port where the server will work;\n";
    std::cout << "--dir, -d <path> - specified the working directory for files on the server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}

int setAddress(std::string& address, struct sockaddr_in* local)
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
        return 1;
    }

    bzero(local, sizeof (*local));
    local->sin_family = AF_INET;

    if (!inet_aton(ip_address.c_str(), &local->sin_addr))
    {
        std::cerr << ip_address << " - unknown host\n";
        return 1;
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
        return 1;
    }
    return 0;
}
