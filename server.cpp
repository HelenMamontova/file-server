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

int sendSuccess(int s1)
{
//отправка клиенту кода команды успешной записи файла
    uint8_t command_success = 129;
    int res = send(s1, &command_success, sizeof(command_success), 0);
    if (res < 0 || res != sizeof(command_success))
    {
        std::cerr << "Send call error command success. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

int sendError(int s1, std::string error_message)
{
//отправка клиенту кода ошибки открытия файла для записи
    uint8_t command_send = 128;
    int res = send(s1, &command_send, sizeof(command_send), 0);
    if (res < 0 || res != sizeof(command_send))
    {
        std::cerr << "Send call error command. " << strerror(errno) << "\n";
        return 1;
    }

// отправка клиенту длины сообщения
    uint32_t error_message_len = error_message.length();
    res = send(s1, &error_message_len, sizeof(error_message_len), 0);
    if (res < 0 || res != sizeof(error_message_len))
    {
        std::cerr << "Send call error error message length. " << strerror(errno) << "\n";
        return 1;
    }

// отправка клиенту ообщения об ошибке
    res = send(s1, error_message.c_str(), error_message.length(), 0);
    if (res < 0 || res != (int)error_message.length())
    {
        std::cerr << "Send call error error message. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

int sendList(int s1, const std::string& path)
{
//отправка клиенту кода команды отправки списка файлов
    uint8_t command_send = 131;
    int res = send(s1, &command_send, sizeof(command_send), 0);
    if (res < 0 || res != sizeof(command_send))
    {
        std::cerr << "Send call error command. " << strerror(errno) << "\n";
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

// отправка клиенту длины списка файлов
    uint32_t list_len = list.length();
    res = send(s1, &list_len, sizeof(list_len), 0);
    if (res < 0 || res != sizeof(list_len))
    {
        std::cerr << "Send call error list length. " << strerror(errno) << "\n";
        return 1;
    }

// отправка клиенту списка файлов
    res = send(s1, list.c_str(), list.length(), 0);
    if (res < 0 || res != (int)list.length())
    {
        std::cerr << "Send call error file list. " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

std::string receiveFileName(int s1)
{
        uint32_t file_name_len;
        int res = recv(s1, &file_name_len, sizeof(file_name_len), 0);
        if (res < 0 || res != sizeof(file_name_len))
            return "Recv call error file name length. " + std::string( strerror(errno)) + "\n";

// получение сервером имени файла
        std::vector <char> file_name(file_name_len);
        res = recv(s1, file_name.data(), file_name_len, 0);
        if (res < 0 || res != (int)file_name_len)
            return "Recv call error file name. " + std::string( strerror(errno)) + "\n";

        std::string name(file_name.begin(), file_name.end());

        return name;
}

int sendFile(int s1, const std::string& path)
{
    std::string file_name = receiveFileName(s1);
    std::string path_file = path + "/" + file_name;

// проверка существования файла
    struct stat st;
    if (stat(path_file.c_str(), &st) < 0)
    {
        std::string error_message = "File does not exists or does not have access.";
        if (sendError(s1, error_message))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }

        std::cerr << "File does not exists or does not have access: " << path_file << "\n";
        return 1;
    }

// определение длины файла
    struct stat st_buff;
    int res = stat(path_file.c_str(), &st_buff);
    if (res < 0)
    {
        std::cerr << "Stat call error. " << strerror(errno) << "\n";
        return 1;
    }

    uint32_t filesize = st_buff.st_size;

//открытие сервером файла
    std::ifstream fin(path_file);
    if (!fin)
    {
        std::string error_message = "File failed to open.";
        if (sendError(s1, error_message))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
        std::cerr << path_file << "File not open.\n";
        return 1;
    }

//отправка клиенту кода команды отправки файла
    uint8_t command_send = 130;
    res = send(s1, &command_send, sizeof(command_send), 0);
    if (res < 0 || res != sizeof(command_send))
    {
        std::cerr << "Send call error command. " << strerror(errno) << "\n";
        return 1;
    }

//отправка клиенту длины файла
    res = send(s1, &filesize, sizeof(filesize), 0);
    if (res < 0 || res != sizeof(filesize))
    {
        std::cerr << "Send call error file size. " << strerror(errno) << "\n";
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
            res = send(s1, buff, fin.gcount(), 0);
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
    std::string file_name = receiveFileName(s1);
    std::string path_file = path + "/" + file_name;

// проверка существования файла
    struct stat st;
    if (stat(path_file.c_str(), &st) == 0)
    {
        std::string error_message = "Such file already exists.";
        if (sendError(s1, error_message))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }

        std::cerr << "Such file already exists: " << path_file << "\n";
        return 1;
    }

    if (sendSuccess(s1))
    {
        std::cerr << "Send error success.\n";
        return 1;
    }

// получение длины файла от клиента
    uint32_t filesize;
    int res = recv(s1, &filesize, sizeof(filesize), 0);
    if (res < 0 || res != sizeof(filesize))
    {
        std::cerr << "Recv call error filesize. " << strerror(errno) << "\n";
        return 1;
    }

//открытие файла для записи
    std::ofstream fout(path_file);
    if (!fout)
    {
        std::string error_message = "File failed to open.";
        if (sendError(s1, error_message))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
        std::cerr << path_file << "File not open.\n";
        return 1;
    }

// получение содержимого буфера от клиента
    size_t bytes_recv = 0;
    int bytes_write = 0;
    while (bytes_recv < filesize)
    {
        char buff[1024] = {0};
        res = recv(s1, buff, sizeof(buff), 0);
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
    if (bytes_write < 0)
    {
        std::string error_message = "File write error.";
        if (sendError(s1, error_message))
        {
            std::cerr << "Send error message error.\n";
            return 1;
        }
    }

    if (sendSuccess(s1))
    {
        std::cerr << "Send error success.\n";
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
        int res =recv(s1, &com, sizeof(com), 0);
        if (res < 0 || res != sizeof(com))
        {
            std::cerr << "Recv call error command main. " << strerror(errno) << "\n";
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
