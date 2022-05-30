#ifndef COLOURISER_H
#define COLOURISER_H

// Sourced from https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal.
#include <fstream>

namespace Colour {

// ANSI colour codes. See: https://en.wikipedia.org/wiki/ANSI_escape_code.
enum Code {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_YELLOW   = 33,
    FG_BLUE     = 34,
    FG_MAGENTA  = 35,
    FG_DEFAULT  = 39,
};

// Embodies a colourised modification to the stdout stream.
struct Mod {
    Code code;
    explicit Mod(Code mod_code);
};
std::ostream& operator<<(std::ostream& os, const Mod& mod);

// Standard colours.
static Mod RED = Mod(Code::FG_RED);
static Mod GREEN = Mod(Code::FG_GREEN);
static Mod YELLOW = Mod(Code::FG_YELLOW);
static Mod BLUE = Mod(Code::FG_BLUE);
static Mod MAGENTA = Mod(Code::FG_MAGENTA);
static Mod RESET = Mod(Code::FG_DEFAULT);

}

#endif