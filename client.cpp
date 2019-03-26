#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint> //uint8_t, uint32_t
#include <sys/stat.h> //stat, struct stat
#include <sys/types.h> //socket, connect
#include <sys/socket.h> //socket, connect
#include <netinet/in.h> //struct sockaddr_in
#include <errno.h>

#include "utils.h"

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

int receiveList(int s)
{
    if (sendUint8(s, LIST))
    {
        std::cerr << "Send command LIST error.\n";
        return 1;
    }

// getting command to send the file list from server
    uint8_t response_code;
    if (receiveUint8(s, response_code))
    {
        std::cerr << "Receive response_code error.\n";
        return 1;
    }

    if (response_code != SEND_LIST )
    {
        std:: cerr << "Wrong commad to send list." << "\n";
        return 1;
    }

    std::string file_list;
    if (receiveString(s, file_list))
    {
        std::cerr << "Receive string list error.\n";
        return 1;
    }
    std::cout << file_list << "\n";
    return 0;
}

int receiveFile(int s, const std::string& file_name)
{
    if (sendUint8(s, GET))
    {
        std::cerr << "Send command GET error.\n";
        return 1;
    }

    if (sendString(s, file_name))
    {
        std::cerr << "Send string file_name error.\n";
        return 1;
    }

// getting server response about file existence
    uint8_t response_code;
    if (receiveUint8(s, response_code))
    {
        std::cerr << "Receive response_code  error.\n";
        return 1;
    }

    if (response_code == ERROR)
    {
        std::string error_message;
        if (receiveString(s, error_message))
        {
            std::cerr << "Receive string error_message error.\n";
            return 1;
        }
        std::cerr << error_message << "\n";
        return 1;
    }
    else if (response_code != SEND_FILE)
    {
        std::cerr << "Unknown command: " << response_code << "\n";
        return 1;
    }
// getting file length from server
    uint32_t filesize;
    if (receiveUint32(s, filesize))
    {
        std::cerr << "Receive file length error.\n";
        return 1;
    }

// open file for writing
    std::ofstream fout(file_name);
    if (!fout)
    {
        std::cerr << file_name << "File not open.\n";
        return 1;
    }

// getting buffer contents from server
    size_t bytes_recv = 0;
    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        int  res = recv(s, buff, sizeof(buff), 0);
        bytes_recv += res;
        if (res < 0)
        {
            std::cerr << "Recv call error buff. " << strerror(errno) << "\n";
            return 1;
        }

// write file
        fout.write(buff, res);
    }
    return 0;
}

int sendFile(int s, const std::string& file_name)
{
// send file write command
    if (sendUint8(s, PUT))
    {
        std::cerr << "Send command PUT error.\n";
        return 1;
    }

    if (sendString(s, file_name))
    {
        std::cerr << "Send string file_name error.\n";
        return 1;
    }

// getting permission or prohibition for the file name
    uint8_t response_code;
    if (receiveUint8(s, response_code))
    {
        std::cerr << "Receive response_code error.\n";
        return 1;
    }

    if (response_code == ERROR)
    {
        std::string error_message;
        if (receiveString(s, error_message))
        {
            std::cerr << "Receive string error_message error.\n";
            return 1;
        }
        std::cerr << error_message << "\n";
        return 1;
    }
    else if (response_code != SUCCESS)
    {
        std::cerr << "Unknown command: " << response_code << "\n";
        return 1;
    }

// file length determination
    struct stat st_buff;
    int res = stat(file_name.c_str(), &st_buff);
    if (res < 0)
    {
        std::cerr << "Stat call error. " << strerror(errno) << "\n";
        return 1;
    }

    uint32_t filesize = st_buff.st_size;

// sending file length
    if (sendUint32(s, filesize))
    {
        std::cerr << "Send file length error.\n";
        return 1;
    }

// open file
    std::ifstream fin(file_name);
    if (!fin)
    {
        std::cerr << file_name << "File not open.\n";
        return 1;
    }

// read file to buffer
    char buff[1024] = {0};
    while (!fin.eof())
    {
        fin.read(buff, 1024);

// sending buffer contents
        if (fin.gcount() > 0)
        {
            res = send(s, buff, fin.gcount(), 0);
            if (res < 0 || res != (int)fin.gcount())
            {
                std::cerr << "Send call error buff. " << strerror(errno) << "\n";
                return 1;
            }
        }

    }

// getting the file write status code
    uint8_t server_response;
    if (receiveUint8(s, server_response))
    {
        std::cerr << "Receive server_response error.\n";
        return 1;
    }

    if (server_response == ERROR)
    {
        std::string error_message;
        if (receiveString(s, error_message))
        {
            std::cerr << "Receive string error_message error.\n";
            return 1;
        }
        std::cerr << error_message << "\n";
        return 1;
    }
    else if (server_response != SUCCESS)
    {
        std::cerr << "Unknown command: " << server_response << "\n";
        return 1;
    }
    return 0;
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
        if (receiveFile(s, file_name))
        {
            std::cerr << "Receive file error.\n";
            return 1;
        }
    }
    else if (command == "put")
    {
        if (sendFile(s, file_name))
        {
            std::cerr << "Send file error.\n";
            return 1;
        }
    }
    else if (command == "list")
    {
        if (receiveList(s))
        {
            std::cerr << "Receive list error.\n";
            return 1;
        }
    }
    return 0;
}
