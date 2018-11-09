#include <iostream>
#include <cstring>
#include <string>

int main(int argc, char * argv[])
{
    std::string version = "1.0";

    if (argc > 1) {
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")){
            std::cout << "Version " << version << "\n";
        } else {
            if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
                std::cout << "Usage:\n";
                std::cout << "server [-v/--version] [-h/--help]\n\n";

                std::cout << "--version, -v - server version;\n";
                std::cout << "--help, -h - show this text.\n";
            } else {
                std::cout << "Unknown command line argument\n";
                return -1;
            }
        }
    } else {
        std::cout << "No arguments\n";
        return -1;
    }

    return 0;
}
