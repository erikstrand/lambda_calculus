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

    auto const pair_combinator = [&arena]() {
        auto const var_t0 = arena.make_variable("t0");
        auto const var_t1 = arena.make_variable("t1");
        auto const var_b = arena.make_variable("b");
        auto term = arena.make_application(var_b, var_t0);
        term = arena.make_application(term, var_t1);
        term = arena.make_abstraction(var_b, term);
        term = arena.make_abstraction(var_t1, term);
        return arena.make_abstraction(var_t0, term);
    }();

    auto const make_pair = [&arena, pair_combinator](TermId first, TermId second) {
        auto term = arena.make_application(pair_combinator, first);
        return arena.make_application(term, second);
    };

    auto const first_combinator = [&arena, true_combinator]() {
        auto const var_p = arena.make_variable("p");
        auto term = arena.make_application(var_p, true_combinator);
        return arena.make_abstraction(var_p, term);
    }();

    auto const second_combinator = [&arena, false_combinator]() {
        auto const var_p = arena.make_variable("p");
        auto term = arena.make_application(var_p, false_combinator);
        return arena.make_abstraction(var_p, term);
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

    auto const addition_combinator = [&arena]() {
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

    auto const church_sum = [&arena, addition_combinator](TermId m, TermId n) {
        auto term = arena.make_application(addition_combinator, m);
        return arena.make_application(term, n);
    };

    auto const multiplication_combinator = [&arena, addition_combinator, church_zero]() {
        auto const var_m = arena.make_variable("m");
        auto const var_n = arena.make_variable("n");
        auto term = arena.make_application(addition_combinator, var_n);
        term = arena.make_application(var_m, term);
        term = arena.make_application(term, church_zero);
        term = arena.make_abstraction(var_n, term);
        return arena.make_abstraction(var_m, term);
    }();

    auto const church_product = [&arena, multiplication_combinator](TermId m, TermId n) {
        auto term = arena.make_application(multiplication_combinator, m);
        return arena.make_application(term, n);
    };

    auto const is_zero_combinator = [&arena, true_combinator, false_combinator]() {
        auto const var_n = arena.make_variable("n");
        auto const var_x = arena.make_variable("x");
        auto term = arena.make_abstraction(var_x, false_combinator);
        term = arena.make_application(var_n, term);
        term = arena.make_application(term, true_combinator);
        return arena.make_abstraction(var_n, term);
    }();

    auto pair = make_pair(make_church_numeral(2), make_church_numeral(3));
    std::cout << "pair: " << TermPrinter(arena, pair) << '\n';
    pair = reduce_normal_order(arena, pair);
    std::cout << "pair: " << TermPrinter(arena, pair) << '\n';

    auto term = arena.make_application(first_combinator, pair);
    std::cout << "first: " << TermPrinter(arena, term) << '\n';
    term = reduce_normal_order(arena, term);
    std::cout << "first: " << TermPrinter(arena, term) << '\n';

    term = arena.make_application(second_combinator, pair);
    std::cout << "second: " << TermPrinter(arena, term) << '\n';
    term = reduce_normal_order(arena, term);
    std::cout << "second: " << TermPrinter(arena, term) << '\n';
}
