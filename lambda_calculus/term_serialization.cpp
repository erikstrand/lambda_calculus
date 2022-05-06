#include "term_serialization.h"
#include <array>

namespace lambda {

//..................................................................................................
void serialize_term(TermArena const& arena, TermId root, std::ostream& stream) {
    struct StackEntry {
        StackEntry(TermId term): children{term, TermId{}}, size{1}, idx{0} {}
        StackEntry(TermId left, TermId right): children{left, right}, size{2}, idx{0} {}

        std::array<TermId, 2> children;
        uint32_t size;
        uint32_t idx;
    };

    std::vector<StackEntry> stack;
    stack.emplace_back(root);

    while (true) {
        TermId term_id = stack.back().children[stack.back().idx];
        LambdaTerm const& term = arena[term_id];

        // Enter this term, and add its children (if any) to the stack.
        bool const added_terms_to_stack = term.visit(
            [&](Variable variable) {
                stream << variable.name;
                return false;
            },
            [&](Abstraction abstraction) {
                stream << "(λ" << arena[abstraction.variable].get_name() << '.';
                stack.emplace_back(abstraction.body);
                return true;
            },
            [&](Application application) {
                stream << '(';
                stack.emplace_back(application.left, application.right);
                return true;
            }
        );

        // If this term has children, go down one level.
        if (added_terms_to_stack) {
            continue;
        }

        // Otherwise we need to backtrack.
        term.visit(
            [](Variable) {},
            [&](auto) { stream << ')'; }
        );
        while (true) {
            // If this term has more siblings, move to the next one.
            ++stack.back().idx;
            if (stack.back().idx < stack.back().size) {
                // This means it was an application.
                // We want a space between the left and right terms.
                stream << ' ';
                break;
            }

            // Otherwise go up a level.
            stack.pop_back();
            if (stack.size() > 0) {
                term_id = stack.back().children[stack.back().idx];
                arena[term_id].visit(
                    [](Variable) {},
                    [&](auto) { stream << ')'; }
                );
            } else {
                return;
            }
        }
    }
}

}
