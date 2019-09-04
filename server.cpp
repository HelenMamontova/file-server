#include "protocol.h"
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
    s1.sendUint8(ERROR);

    s1.sendString(error_message);
}

void sendList(Socket& s1, const std::string& path)
{
    // getting file list
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
        throw Socket::Error("Directory failed to open.");
    std::string list;

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name))
            list = list + entry->d_name + "\n";
    }
    closedir(dir);

    // sending code to send file list
    s1.sendUint8(SEND_LIST);

    s1.sendString(list);
}

void sendFile(Socket& s1, const std::string& path)
{
    std::string file_name = s1.receiveString();

    std::string path_file = path + "/" + file_name;

    // file existence check
    struct stat st;
    if (stat(path_file.c_str(), &st) < 0)
    {
        sendError(s1, "File does not exist or does not have access.");

        std::cerr << "File does not exist or does not have access: " << path_file << "\n";
        return;
    }

    // open file
    std::ifstream fin(path_file);
    if (!fin)
    {
        sendError(s1, "File failed to open.");

        std::cerr << path_file << " File failed to open.\n";
        return;
    }

    // send code to send file
    s1.sendUint8(SEND_FILE);

    // file length determination
    uint32_t filesize = st.st_size;

    // sending file length
    s1.sendUint32(filesize);

    // read file to buffer
    char buff[1024] = {0};
    while (!fin.eof())
    {
        fin.read(buff, 1024);

    // sending buffer contents
        std::streamsize len = fin.gcount();
        if (len > 0)
        {
            if (s1.send(buff, len, 0) != (size_t)len)
            {
                std::cerr << "File failed to send. " << strerror(errno) << "\n";
                return;
            }
        }
    }
}

void receiveFile(Socket& s1, const std::string& path)
{
    std::string file_name = s1.receiveString();

    std::string path_file = path + "/" + file_name;

    // file existence check
    struct stat st;
    if (stat(path_file.c_str(), &st) == 0)
    {
        sendError(s1, "Such file already exists.");

        std::cerr << "Such file already exists: " << path_file << "\n";
        return;
    }

    // open file for writing
    std::ofstream fout(path_file);
    if (!fout)
    {
        sendError(s1, "File failed to open.");

        std::cerr << path_file << " File failed to open.\n";
        return;
    }

    s1.sendUint8(SUCCESS);

    // getting file length from client
    uint32_t filesize = s1.receiveUint32();

    // getting buffer contents from client
    size_t bytes_recv = 0;

    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        int res = s1.recv(buff, sizeof(buff), 0);
        bytes_recv += res;

    // write file
        if (!fout.write(buff, res))
        {
            sendError(s1, "File failed to write.");

            std::cerr << path_file << " File failed to write.\n";
            return;
        }
    }
    s1.sendUint8(SUCCESS);
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
            Socket s1 = serverSocket.accept(peer);
            try
            {
                uint8_t com = s1.receiveUint8();

                if (com == GET)
                    sendFile(s1, path);
                else if (com == PUT)
                    receiveFile(s1, path);
                else if (com == LIST)
                    sendList(s1, path);
            }
            catch (const Socket::Error &exception)
            {
                std::cerr << "Error: " << exception.what() << "\n";
            }
        }
    }
    catch (const Socket::Error &exception)
    {
        std::cerr << "Error: " << exception.what() << "\n";
    }
    return 0;
}
