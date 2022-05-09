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

    TermId const identity = [&arena]() {
        auto const var_x = arena.make_variable("x");
        return arena.make_abstraction(var_x, var_x);
    }();

    // We represent true as the function of two variables that returns the first argument.
    // λt. λf. t
    TermId const true_combinator = [&arena]() {
        auto const var_t = arena.make_variable("t");
        auto const var_f = arena.make_variable("f");
        auto const lambda_f = arena.make_abstraction(var_f, var_t);
        return arena.make_abstraction(var_t, lambda_f);
    }();

    // We represent false as the function of two variables that returns the second argument.
    // λt. λf. f
    TermId const false_combinator = [&arena]() {
        auto const var_t = arena.make_variable("t");
        auto const var_f = arena.make_variable("f");
        auto const lambda_f = arena.make_abstraction(var_f, var_f);
        return arena.make_abstraction(var_t, lambda_f);
    }();

    // We can implement control flow by passing two different computations to a boolean.
    // true_combinator true_branch false_branch = true_branch
    // false_combinator true_branch false_branch = false_branch
    auto const if_then_else = [&arena](TermId condition, TermId true_branch, TermId false_branch) {
        auto term = arena.make_application(condition, true_branch);
        return arena.make_application(term, false_branch);
    };

    // Similarly, thinking of terms as data, a pair lets us package two terms up as one and select
    // between them later.
    // λt0. λt1. λb. b t0 t1
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
    auto const get_first = [&arena, true_combinator](TermId pair) {
        return arena.make_application(pair, true_combinator);
    };
    auto const get_second = [&arena, false_combinator](TermId pair) {
        return arena.make_application(pair, false_combinator);
    };

    // The natural number n is represented as the function of two arguments that applies the first
    // to the second n times.
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

    // To get the successor of a natural number, we just need to apply s one more time.
    // λn. λs. λz. s (n s z)
    auto const succ_combinator = [&arena]() {
        auto const var_n = arena.make_variable("n");
        auto const var_s = arena.make_variable("s");
        auto const var_z = arena.make_variable("z");
        auto term = arena.make_application(var_n, var_s);
        term = arena.make_application(term, var_z);
        term = arena.make_application(var_s, term);
        term = arena.make_abstraction(var_z, term);
        term = arena.make_abstraction(var_s, term);
        return arena.make_abstraction(var_n, term);
    }();

    // More generally, to add two numbers, we can plug one in as the "zero" of another.
    // λm. λn. λs. λz. (m s) (n s z)
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

    auto const pred_combinator = [=, &arena]() {
        // This function takes a pair of numbers (n, m) and returns a pair (m, m + 1).
        // If we apply our helper function n times starting with (0, 0), we will get (n - 1, n).
        auto const var_p = arena.make_variable("p");
        auto const second = get_second(var_p);
        auto helper_function = arena.make_abstraction(
            var_p,
            make_pair(
                second,
                arena.make_application(succ_combinator, second))
        );

        auto const var_n = arena.make_variable("n");
        auto term = arena.make_application(var_n, helper_function);
        term = arena.make_application(term, make_pair(church_zero, church_zero));
        term = get_first(term);
        return arena.make_abstraction(var_n, term);
    }();

    auto const is_zero_combinator = [&arena, true_combinator, false_combinator]() {
        auto const var_n = arena.make_variable("n");
        auto const var_x = arena.make_variable("x");
        auto term = arena.make_abstraction(var_x, false_combinator);
        term = arena.make_application(var_n, term);
        term = arena.make_application(term, true_combinator);
        return arena.make_abstraction(var_n, term);
    }();

    auto const y_combinator = [&arena]() {
        auto const var_f = arena.make_variable("f");
        auto const var_x = arena.make_variable("x");
        auto term = arena.make_application(var_x, var_x);
        term = arena.make_application(var_f, term);
        term = arena.make_abstraction(var_x, term);
        term = arena.make_application(term, term);
        return arena.make_abstraction(var_f, term);
    }();

    std::cout << "Y combinator: " << TermPrinter(arena, y_combinator) << '\n';

    auto const var_f = arena.make_variable("f");
    auto const var_n = arena.make_variable("n");
    auto term = if_then_else(
        //arena.make_application(is_zero_combinator, var_n),
        true_combinator,
        church_zero,
        //arena.make_application(var_f, arena.make_application(pred_combinator, var_n))
        arena.make_application(var_f, var_n)
    );
    //term = arena.make_abstraction(var_n, term);
    //auto const partial_func = arena.make_abstraction(var_f, term);

    //term = arena.make_application(y_combinator, term);
    //term = arena.make_application(term, make_church_numeral(2));

    //term = arena.make_application(partial_func, identity);
    //term = arena.make_application(term, church_zero);

    //term = arena.make_application(term, church_zero);

    for (uint32_t i = 0; i < 10; ++i) {
        std::cout << "inner func 0: " << TermPrinter(arena, term) << "\n\n";
        term = reduce_normal_order(arena, term, 1);
    }
}
