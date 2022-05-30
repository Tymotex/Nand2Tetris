#include "Colouriser.h"
#include <iostream>

Colour::Mod::Mod(Code mod_code) 
    : code(mod_code) {}

std::ostream& Colour::operator<<(std::ostream& os, const Colour::Mod& mod) {
    int colour_code = static_cast<uint32_t>(mod.code);
    return os << "\033[" << mod.code << "m";
}
