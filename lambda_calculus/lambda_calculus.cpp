#include <cstring>
#include <iostream>
#include "utils/utf8.h"
#include "term_arena.h"
#include "term_reduction.h"
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
    auto const lx_x = arena.make_abstraction(var_x, var_x);
    auto term = arena.make_application(lx_x, var_y);

    serialize_term(arena, term, std::cout);
    std::cout << '\n';
    term = reduce_normal_order(arena, term);
    serialize_term(arena, term, std::cout);
    std::cout << '\n';
}
