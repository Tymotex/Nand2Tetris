#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        std::cerr << "Insufficient arguments. Please supply a file with filename *.asm.\n";
    } else if (argc >= 3) {
        std::cerr << "Too many arguments. Please supply exactly one file with filename *.asm.\n";
    }

    // TODO: regex check the filename is valid.

    return 0;
}
