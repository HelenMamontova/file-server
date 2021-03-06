#include "protocol.h"
#include "socket.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint> //uint8_t, uint32_t
#include <sys/stat.h> //stat, struct stat
#include <sys/types.h> //socket, connect
#include <sys/socket.h> //socket, connect
#include <errno.h>


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

void receiveList(Socket& s)
{
    s.send<uint8_t>(LIST);

    // getting comand to send the file list from server
    uint8_t response_code = s.receive<uint8_t>();

    if (response_code != SEND_LIST )
    {
        std:: cerr << "Wrong comad to send list: " << response_code << "\n";
        return;
    }

    std::cout << s.receiveString() << "\n";
}

void receiveFile(Socket& s, const std::string& file_name)
{
    s.send<uint8_t>(GET);

    s.sendString(file_name);

    // getting server response about file existence
    uint8_t response_code = s.receive<uint8_t>();

    if (response_code == ERROR)
    {
        std::cerr << s.receiveString() << "\n";
        return;
    }
    if (response_code != SEND_FILE)
    {
        std::cerr << "Unknown command: " << response_code << "\n";
        return;
    }

    // getting file length from server
    uint32_t filesize = s.receive<uint32_t>();

    // open file for writing
    std::ofstream fout(file_name);
    if (!fout)
    {
        std::cerr << file_name << " File failed to open.\n";
        return;
    }

    // getting buffer contents from server
    size_t bytes_recv = 0;
    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        size_t len = filesize - bytes_recv;

        if (len > sizeof(buff))
            len = sizeof(buff);

        s.recv(buff, len, 0);

    // write file
        fout.write(buff, len);
        bytes_recv += len;
    }
}

void sendFile(Socket& s, const std::string& file_name)
{
    struct stat st_buff;
    int res = stat(file_name.c_str(), &st_buff);
    if (res < 0)
    {
        std::cerr << "File does not exist or does not have access. " << strerror(errno) << "\n";
        return;
    }

    // open file
    std::ifstream fin(file_name);
    if (!fin)
    {
        std::cerr << file_name << " File failed to open.\n";
        return;
    }

    // send file write command
    s.send<uint8_t>(PUT);

    s.sendString(file_name);

    // getting permission or prohibition for the file name
    uint8_t response_code = s.receive<uint8_t>();

    if (response_code == ERROR)
    {
        std::cerr << s.receiveString() << "\n";
        return;
    }
    if (response_code != SUCCESS)
    {
        std::cerr << "Unknown command: " << response_code << "\n";
        return;
    }

    // sending file length
    s.send<uint32_t>(st_buff.st_size);

    // read file to buffer
    char buff[1024] = {0};
    while (!fin.eof())
    {
        fin.read(buff, 1024);

    // sending buffer contents
        std::streamsize len = fin.gcount();
        if (len > 0)
            s.send(buff, len, 0);
    }

    // getting the file write status code
    uint8_t server_response = s.receive<uint8_t>();

    if (server_response == ERROR)
    {
        std::string error_message = s.receiveString();

        std::cerr << error_message << "\n";
        return;
    }
    if (server_response != SUCCESS)
    {
        std::cerr << "Unknown command: " << server_response << "\n";
        return;
    }
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

    try
    {
        Socket s;

        s.connect(server_address);

        if (command == "get")
            receiveFile(s, file_name);
        else if (command == "put")
            sendFile(s, file_name);
        else if (command == "list")
            receiveList(s);
    }
    catch (const Socket::Error &exception)
    {
        std::cerr << "Error: " << exception.what() << "\n";
    }
    return 0;
}
