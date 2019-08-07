#include "utils.h"
#include "socket.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint> //uint8_t, uint32_t
#include <sys/stat.h> //stat, struct stat
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <netinet/in.h> //struct sockaddr_in
#include <linux/in.h> //struct sockaddr_in
#include <dirent.h> //struct dirent, opendir, readdir, closedir
#include <errno.h>
#include <unistd.h> //close

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "server -b/--bind <ip_address:port> -d/--dir <path> [-v/--version] [-h/--help]\n\n";

    std::cout << "--bind, -b <ip_address:port> - specifies local IP address and port where the server will work;\n";
    std::cout << "--dir, -d <path> - specifies the working directory for files on the server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}

void sendError(Socket& s1, std::string error_message)
{
    // send error code to open file for writing
    sendUint8(s1, ERROR);

    sendString(s1, error_message);
}

void sendList(Socket& s1, const std::string& path)
{
    // getting file list
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
        throw Socket::Error("Error opendir.");
    std::string list;

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name))
            list = list + entry->d_name + "\n";
    }
    closedir(dir);

    // sending code to send file list
    sendUint8(s1, SEND_LIST);

    sendString(s1, list);
}

void sendFile(Socket& s1, const std::string& path)
{
    std::string file_name;
    receiveString(s1, file_name);

    std::string path_file = path + "/" + file_name;

    // file existence check
    struct stat st;
    if (stat(path_file.c_str(), &st) < 0)
    {
        sendError(s1, "File does not exists or does not have access.");

        std::cerr << "File does not exists or does not have access: " << path_file << "\n";
    }

    // open file
    std::ifstream fin(path_file);
    if (!fin)
    {
        sendError(s1, "File failed to open.");

        std::cerr << path_file << " File not open.\n";
    }

    // send code to send file
    sendUint8(s1, SEND_FILE);

    // file length determination
    uint32_t filesize = st.st_size;

    // sending file length
    sendUint32(s1, filesize);

    // read file to buffer
    char buff[1024] = {0};
    while (!fin.eof())
    {
        fin.read(buff, 1024);

    // sending buffer contents
        if (fin.gcount() > 0)
        {
            if (s1.send(buff, fin.gcount(), 0) != (size_t)fin.gcount())
                std::cerr << "Send call error buff. " << strerror(errno) << "\n";
        }
    }
}

void receiveFile(Socket& s1, const std::string& path)
{
    std::string file_name;
    receiveString(s1, file_name);

    std::string path_file = path + "/" + file_name;

    // file existence check
    struct stat st;
    if (stat(path_file.c_str(), &st) == 0)
    {
        sendError(s1, "Such file already exists.");

        std::cerr << "Such file already exists: " << path_file << "\n";
    }

    // open file for writing
    std::ofstream fout(path_file);
    if (!fout)
    {
        sendError(s1, "File failed to open.");

        std::cerr << path_file << "File not open.\n";
    }

    sendUint8(s1, SUCCESS);

    // getting file length from client
    uint32_t filesize;
    receiveUint32(s1, filesize);

    // getting buffer contents from client
    size_t bytes_recv = 0;
    int bytes_write = 0;

    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        int res = s1.recv(buff, sizeof(buff), 0);
        bytes_recv += res;

    // write file
        if (!fout.write(buff, res))
        {
            bytes_write = -1;
            break;
        }
    }

    // send the file write status code
    if (bytes_write < 0)
        sendError(s1, "File write error.");

    sendUint8(s1, SUCCESS);
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

    try
    {
        Socket serverSocket;

        serverSocket.bind(address);

        serverSocket.listen(5);

        while (true)
        {
            struct sockaddr_in peer;
            socklen_t peerlen = sizeof(peer);

            Socket s1 = serverSocket.accept(peer, peerlen);

            uint8_t com;
            receiveUint8(s1, com);

            if (com == GET)
            {
                sendFile(s1, path);
            }
            else if (com == PUT)
            {
                receiveFile(s1, path);
            }
            else if (com == LIST)
            {
                sendList(s1, path);
            }
        }
    }

    catch (const Socket::Error &exception)
    {
        std::cerr << "Error: " << exception.what() << "\n";
    }
    return 0;
}
