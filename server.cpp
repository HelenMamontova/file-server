#include <iostream>
#include <cstring>
#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <netinet/in.h> //struct sockaddr_in
#include <errno.h>
#include <unistd.h> //close

#include "setaddress.h"

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "server -b/--bind <ip_address:port> -d/--dir <path> [-v/--version] [-h/--help]\n\n";

    std::cout << "--bind, -b <ip_address:port> - specifies local IP address and port where the server will work;\n";
    std::cout << "--dir, -d <path> - specifies the working directory for files on the server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}

int main(int argc, char* argv[])
{
    std::string version = "1.0";
    std::string address;
    std::string path;

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

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        std::cerr << "Socket call error. " << strerror(errno) << "\n";
        return 1;
    }

    struct sockaddr_in local;

    if (!setAddress(address, &local))
    {
        std::cout << "Address error.\n";
        return 1;
    }

    if (bind(s, (struct sockaddr*) &local, sizeof(local)))
    {
        std::cerr << "Bind call error. " << strerror(errno) << "\n";
        return 1;
    }

    if (listen(s, 5))
    {
        std::cerr << "Listen call error. " << strerror(errno) << "\n";
        return 1;
    }

    while (true)
    {
        struct sockaddr_in peer;
        int peerlen = sizeof(peer);
        int s1 = accept(s, (struct sockaddr*) &peer, (socklen_t*) &peerlen);
        if (s1 < 0)
        {
            std::cerr << "Accept call error. " << strerror(errno) << "\n";
            return 1;
        }

        uint8_t com;
        int rec_com =recv(s1, &com, sizeof(com), 0);
        if (rec_com < 0 || rec_com != sizeof(com))
        {
            std::cerr << "Recv call error command. " << strerror(errno) << "\n";
            return 1;
        }

        uint32_t file_name_len;
        int rec_file_name_len = recv(s1, &file_name_len, sizeof(file_name_len), 0);
        if (rec_file_name_len < 0 || rec_file_name_len != sizeof(file_name_len))
        {
            std::cerr << "Recv call error file name length. " << strerror(errno) << "\n";
            return 1;
        }

        char* file_name = new char[file_name_len];
        int rec_file_name = recv(s1, file_name, file_name_len, 0);
        if (rec_file_name < 0 || rec_file_name != (int)file_name_len)
        {
            std::cerr << "Recv call error file name. " << strerror(errno) << "\n";
            return 1;
        }
        std::string name(file_name);

        delete [] file_name;
        close(s1);
    }

    return 0;
}
