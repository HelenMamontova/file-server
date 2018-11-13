#include <iostream>
#include <cstring>
#include <string>

void reference();

int main(int argc, char * argv[])
{
    std::string version = "1.0";
    std::string ip_address;
    std::string port;
    std::string path;

    bool flag_b = false;
    bool flag_d = false;
    bool flag_v = false;
    bool flag_h = false;

    if (argc < 2) {
        reference();
        return 1;
    } else {
        for (int i = 1; i < argc; ++i) {
            if ((!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bind")) && (i + 2 < argc)) {
                    ip_address = argv[i + 1];
                    port = argv[i + 2];
                    flag_b = true;
    std::cout << "addr " << ip_address << "\n";
    std::cout << "port " << port << "\n";
            }
            if ((!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir")) && (i + 1 < argc)) {
                    path = argv[i + 1];
                    flag_d = true;
    std::cout << "path " << path << "\n";
            }
            if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")){
                std::cout << "Version " << version << "\n";
                flag_v = true;
                if (i + 1 == argc)
                    return 0;
            }
            if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
                flag_h = true;
    std::cout << "Help " << "\n";
                reference();
                if (i + 1 == argc)
                    return 0;
            }
            if (!flag_b && !flag_d && !flag_v && !flag_h) {
                std::cout << "Unknown command line argument\n";
                return 1;
            }
        }
        if (ip_address.empty() || port.empty() || path.empty()) {
            std::cout << "The required parameters are not specified.\n";
            if (!flag_h)
                reference();
        }
    }

    return 0;
}

void reference() {
    std::cout << "Usage:\n";
    std::cout << "server -b/--bind <ip_address> <port> -d/--dir <path> [-v/--version] [-h/--help]\n\n";

    std::cout << "--bind, -b <ip_address> <port> - specifies local IP address and port where the server will work;\n";
    std::cout << "--dir, -d <path> - specified the working directory for files on the server;\n";
    std::cout << "--version, -v - server version;\n";
    std::cout << "--help, -h - show this text.\n";
}
