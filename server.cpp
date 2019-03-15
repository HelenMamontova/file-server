#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint> //uint8_t, uint32_t
#include <sys/stat.h> //stat, struct stat
#include <sys/types.h> //socket, bind
#include <sys/socket.h> //socket, bind, accept, listen
#include <netinet/in.h> //struct sockaddr_in
#include <dirent.h> //struct dirent, opendir, readdir, closedir
#include <errno.h>
#include <unistd.h> //close

#include "utils.h"

void reference()
{
    std::cout << "Usage:\n";
    std::cout << "server -b/--bind <ip_address:port> -d/--dir <path> [-v/--version] [-h/--help]\n\n";

    std::cout << "--bind, -b <ip_address:port> - specifies local IP address and port where the server will work;\n";
    std::cout << "--dir, -d <path> - specifies the working directory for files on the server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}

int sendError(int s1, std::string error_message)
{
//отправка клиенту кода ошибки открытия файла для записи
    if (sendUint8(s1, 128))
    {
        std::cerr << "Send command 128 error.\n";
        return 1;
    }

    if (sendString(s1, error_message))
    {
        std::cerr << "Send string error_message error.\n";
        return 1;
    }
    return 0;
}

int sendList(int s1, const std::string& path)
{
//отправка клиенту кода команды отправки списка файлов
    if (sendUint8(s1, 131))
    {
        std::cerr << "Send command 131 error.\n";
        return 1;
    }

//получение списка файлов
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
    {
        std::cerr << "Opendir call error. " << strerror(errno) << "\n";
        return 1;
    }
    std::string list;

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name))
            list = list + entry->d_name + "\n";
    }
    closedir(dir);

    if (sendString(s1, list))
    {
        std::cerr << "Send string list error.\n";
        return 1;
    }

    return 0;
}

int sendFile(int s1, const std::string& path)
{
    std::string file_name;
    if (receiveString(s1, file_name))
    {
        std::cerr << "Receive string file_name error.\n";
        return 1;
    }
    std::string path_file = path + "/" + file_name;

// проверка существования файла
    struct stat st;
    if (stat(path_file.c_str(), &st) < 0)
    {
        if (sendError(s1, "File does not exists or does not have access."))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }

        std::cerr << "File does not exists or does not have access: " << path_file << "\n";
        return 1;
    }

//открытие сервером файла
    std::ifstream fin(path_file);
    if (!fin)
    {
        if (sendError(s1, "File failed to open."))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
        std::cerr << path_file << "File not open.\n";
        return 1;
    }

//отправка клиенту кода команды отправки файла
    if (sendUint8(s1, 130))
    {
        std::cerr << "Send command 130 error.\n";
        return 1;
    }

// определение длины файла
    uint32_t filesize = st.st_size;

//отправка клиенту длины файла
    if (sendUint32(s1, filesize))
    {
        std::cerr << "Send file length error.\n";
        return 1;
    }

// чтение файла в буфер
    char buff[1024] = {0};
    while (!fin.eof())
    {
        fin.read(buff, 1024);

// отправка содержимого буфера клиенту
        if (fin.gcount() > 0)
        {
            int res = send(s1, buff, fin.gcount(), 0);
            if (res < 0 || res != (int)fin.gcount())
            {
                std::cerr << "Send call error buff. " << strerror(errno) << "\n";
                return 1;
            }
        }
    }
    return 0;
}

int receiveFile(int s1, const std::string& path)
{
    std::string file_name;
    if (receiveString(s1, file_name))
    {
        std::cerr << "Receive string file_name error.\n";
        return 1;
    }
    std::string path_file = path + "/" + file_name;

// проверка существования файла
    struct stat st;
    if (stat(path_file.c_str(), &st) == 0)
    {
        if (sendError(s1, "Such file already exists."))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }

        std::cerr << "Such file already exists: " << path_file << "\n";
        return 1;
    }

//открытие файла для записи
    std::ofstream fout(path_file);
    if (!fout)
    {
        if (sendError(s1, "File failed to open."))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
        std::cerr << path_file << "File not open.\n";
        return 1;
    }

//отправка клиенту кода команды успешной записи файла
    if (sendUint8(s1, 129))
    {
        std::cerr << "Send command 129 error.\n";
        return 1;
    }

// получение длины файла от клиента
    uint32_t filesize;
    if (receiveUint32(s1, filesize))
    {
        std::cerr << "Receive file length error.\n";
        return 1;
    }

// получение содержимого буфера от клиента
    size_t bytes_recv = 0;
    int bytes_write = 0;
    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        int res = recv(s1, buff, sizeof(buff), 0);
        bytes_recv += res;
        if (res < 0)
        {
            std::cerr << "Recv call error buff. " << strerror(errno) << "\n";
            return 1;
        }

// запись файла
        if (!fout.write(buff, res))
        {
            bytes_write = -1;
            break;
        }
    }

// отправка клиенту сообщения о состоянии записи файла
// ошибка
    if (bytes_write < 0)
    {
        if (sendError(s1, "File write error."))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
    }

// успех
    if (sendUint8(s1, 129))
    {
        std::cerr << "Send command 129 error.\n";
        return 1;
    }
    return 0;
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
        if (receiveUint8(s1, com))
        {
            std::cerr << "Receive com error.\n";
            return 1;
        }

        if (com == 0)
        {
            if (sendFile(s1, path))
            {
                std::cerr << "Send file error.\n";
                return 1;
            }
        }
        else if (com == 1)
        {
            if (receiveFile(s1, path))
            {
                std::cerr << "Receive file error.\n";
                return 1;
            }
        }
        else if (com == 2)
        {
            if (sendList(s1, path))
            {
                std::cerr << "Send list error.\n";
                return 1;
            }
        }
        close(s1);
    }
    return 0;
}
