#include <cstring>
#include <iostream>
#include "utils/utf8.h"
#include "term_arena.h"
#include "term_serialization.h"

using namespace lambda;

//--------------------------------------------------------------------------------------------------
int main() {
    char const* source = "λx.x";
    std::cout << source << '\n';

    auto length = std::strlen(source);
    std::cout << "length (bytes): " << length << '\n';

    length = 0;
    char8_t const* pos = reinterpret_cast<char8_t const*>(source);
    while (pos && *pos) {
        pos = next_utf8_codepoint(pos);
        ++length;
    }
    std::cout << "length (code points): " << length << '\n';

    TermArena arena;
    arena.reserve(2048);

    auto const var_x = arena.make_variable("x");
    auto const var_y = arena.make_variable("y");
    auto const xy = arena.make_application(var_x, var_y);
    auto const lx_xy = arena.make_abstraction(var_x, xy);

    serialize_term(arena, lx_xy, std::cout);
    std::cout << '\n';
}
