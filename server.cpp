#include <iostream>
#include <cstring>

int main(int argc, char * argv[])
{
    float version = 1.0;

    if (argc > 1) {
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")){
            std::cout << "Version " << version << "\n";
        } else {
            std::cout << "Argument is not version\n";
            return -1;
        }
    } else {
        std::cout << "Not arguments\n";
        return -1;
    }

    return 0;
}
