#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <sys/types.h> //socket, connect
#include <sys/socket.h> //socket, connect
#include <netinet/in.h> //struct sockaddr_in
#include <errno.h>

#include "setaddress.h"

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "client -a/--address <ip_address:port> -g/--get <name> -p/--put <name> -l/--list [-v/--version] [-h/--help]\n\n";

    std::cout << "--address, -a <ip_address:port> - specifies IP address and port of server;\n";
    std::cout << "--get, -g <name> - specifies the file name to get from server;\n";
    std::cout << "--put, -p <name> - specifies the file name to write to the server;\n";
    std::cout << "--list, -l - gets file list from server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}

int main(int argc, char* argv[])
{
    std::string version = "1.0";
    std::string server_address;
    std::string file_name;
    std::string command;

    if (argc < 2)
    {
        reference();
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        if ((!strcmp(argv[i], "-a") || !strcmp(argv[i], "--address")))
        {
            if (i + 1 == argc)
            {
                std::cerr << argv[i] << " needs and argument - a server address.\n";
                return 1;
            }
            server_address = argv[++i];
        }
        else if ((!strcmp(argv[i], "-g") || !strcmp(argv[i], "--get")))
        {
            if (i + 1 == argc)
            {
                std::cerr << argv[i] << " needs and argument - a file to get.\n";
                return 1;
            }
            file_name = argv[++i];
            command = "get";
        }
        else if ((!strcmp(argv[i], "-p") || !strcmp(argv[i], "--put")))
        {
            if (i + 1 == argc)
            {
                std::cerr << argv[i] << " needs and argument - a file to put.\n";
                return 1;
            }
            file_name = argv[++i];
            command = "put";
        }
        else if ((!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")))
        {
            command = "list";
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

    struct sockaddr_in peer;

    if (!setAddress(server_address, &peer))
    {
        std::cerr << "Address error.\n";
        return 1;
    }

    if (connect(s, (struct sockaddr*) &peer, sizeof(peer)))
    {
        std::cerr << "Connect call error. " << strerror(errno) << "\n";
        return 1;
    }

    if (command == "get")
    {
        uint8_t com = 0;
        int res = send(s, &com, sizeof(com), 0);
        if (res < 0 || res != sizeof(com))
        {
            std::cerr << "Send call error. " << strerror(errno) << "\n";
            return 1;
        }

        uint32_t file_name_len = file_name.length();
        res = send(s, &file_name_len, sizeof(file_name_len), 0);
        if (res < 0 || res != sizeof(file_name_len))
        {
            std::cerr << "Send call error. " << strerror(errno) << "\n";
            return 1;
        }

        res = send(s, file_name.c_str(), file_name.length(), 0);
        if (res < 0 || res != (int)file_name.length())
        {
            std::cerr << "Send call error. " << strerror(errno) << "\n";
            return 1;
        }

// получение кода команды отправки файла сервером
        uint8_t command_recv;
        res = recv(s, &command_recv, sizeof(command_recv), 0);
        if (res < 0 || res != sizeof(command_recv))
        {
            std::cerr << "Recv call error command. " << strerror(errno) << "\n";
            return 1;
        }

        if (command_recv != 130)
        {
            std::cerr << command_recv << "Wrong command.\n";
            return 1;
        }

// получение длины файла от сервера
        uint32_t filesize;
        res = recv(s, &filesize, sizeof(filesize), 0);
        if (res < 0 || res != sizeof(filesize))
        {
            std::cerr << "Recv call error filesize. " << strerror(errno) << "\n";
            return 1;
        }

//открытие файла для записи
        std::ofstream fout(file_name);
        if (!fout)
        {
            std::cerr << file_name << "File not open.\n";
            return 1;
        }

// получение содержимого буфера от сервера
        size_t bytes_recv = 0;
        while (bytes_recv < filesize)
        {
            char buff[1024] = {0};
            res = recv(s, buff, sizeof(buff), 0);
            bytes_recv += res;
            if (res < 0)
            {
                std::cerr << "Recv call error buff. " << strerror(errno) << "\n";
                return 1;
            }

// запись файла
            fout.write(buff, res);
        }
    }
    return 0;
}
