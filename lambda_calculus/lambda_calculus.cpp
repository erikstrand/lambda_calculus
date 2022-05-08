#include <cstring>
#include <iostream>
#include "utils/utf8.h"
#include "term_arena.h"
#include "term_reduction.h"
#include "term_serialization.h"

using namespace lambda;

//--------------------------------------------------------------------------------------------------
int main() {
    /*
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
    std::cout << "length (code points): " << length << "\n\n";
    */

    TermArena arena;
    arena.reserve(2048);

    auto const true_combinator = [&arena]() {
        auto const var_t = arena.make_variable("t");
        auto const var_f = arena.make_variable("f");
        auto const lambda_f = arena.make_abstraction(var_f, var_t);
        return arena.make_abstraction(var_t, lambda_f);
    }();

    auto const false_combinator = [&arena]() {
        auto const var_t = arena.make_variable("t");
        auto const var_f = arena.make_variable("f");
        auto const lambda_f = arena.make_abstraction(var_f, var_f);
        return arena.make_abstraction(var_t, lambda_f);
    }();

    auto const make_church_numeral = [&arena](uint32_t n) {
        auto const var_s = arena.make_variable("s");
        auto const var_z = arena.make_variable("z");

        // Apply s to z n times.
        auto term = var_z;
        for (uint32_t i = 0; i < n; ++i) {
            term = arena.make_application(var_s, term);
        }

        term = arena.make_abstraction(var_z, term);
        return arena.make_abstraction(var_s, term);
    };

    auto const church_zero = make_church_numeral(0);
    auto const church_one = make_church_numeral(1);

    auto const plus_combinator = [&arena]() {
        auto const var_m = arena.make_variable("m");
        auto const var_n = arena.make_variable("n");
        auto const var_s = arena.make_variable("s");
        auto const var_z = arena.make_variable("z");
        auto left_term = arena.make_application(var_m, var_s);
        auto right_term = arena.make_application(var_n, var_s);
        right_term = arena.make_application(right_term, var_z);
        auto term = arena.make_application(left_term, right_term);
        term = arena.make_abstraction(var_z, term);
        term = arena.make_abstraction(var_s, term);
        term = arena.make_abstraction(var_n, term);
        return arena.make_abstraction(var_m, term);
    }();

    auto const church_sum = [&arena, plus_combinator](TermId m, TermId n) {
        auto term = arena.make_application(plus_combinator, m);
        return arena.make_application(term, n);
    };

    //auto const var_x = arena.make_variable("x");
    auto term = church_sum(church_one, church_one);

    serialize_term(arena, term, std::cout);
    std::cout << '\n';

    term = reduce_normal_order(arena, term);
    serialize_term(arena, term, std::cout);
    std::cout << '\n';
}
