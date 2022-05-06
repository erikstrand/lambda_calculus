#include "term_reduction.h"

#include <iostream>

namespace lambda {

//..................................................................................................
TermId copy_and_substitute(
    TermArena& arena,
    TermId root_id,
    TermId new_root_id,
    TermId variable_id,
    TermId argument_id
) {
    struct StackEntry {
        StackEntry(TermId term):
            terms{term, TermId{}}, new_children{}, size{1}, idx{0}
        {}

        StackEntry(TermId left, TermId right):
            terms{left, right}, new_children{}, size{2}, idx{0}
        {}

        std::array<TermId, 2> terms;
        std::array<TermId, 2> new_children;
        uint32_t size;
        uint32_t idx;
    };

    // Initialize the stack for a depth first traversal from root_id.
    std::vector<StackEntry> stack;
    stack.emplace_back(root_id);

    // This stores all terms that have been duplicated. It mpas the original TermId (as used in the
    // the tree beneath root_id) to that of the duplicate.
    std::unordered_map<TermId, TermId> new_terms;

    while (true) {
        StackEntry* context = &stack.back();
        TermId term_id = context->terms[context->idx];
        LambdaTerm* term = &arena[term_id];

        // Enter this term.
        bool const added_terms_to_stack = term->visit(
            [&](Variable) {
                std::cout << "entering variable\n";
                return false;
            },
            [&](Abstraction abstraction) {
                std::cout << "entering abstraction\n";
                if (new_terms.contains(term_id)) {
                    return false;
                }
                stack.emplace_back(abstraction.variable, abstraction.body);
                return true;
            },
            [&](Application application) {
                std::cout << "entering application\n";
                if (new_terms.contains(term_id)) {
                    return false;
                }
                stack.emplace_back(application.left, application.right);
                return true;
            }
        );

        // If this term has children, go down one level.
        if (added_terms_to_stack) {
            continue;
        }

        // Otherwise we need to backtrack.
        while (true) {
            // Leave this term. This is when we create the duplicate node (if needed).
            TermId const new_term_id = [&]() {
                // If this term is the bound variable, perform the substitution.
                if (term_id == variable_id) {
                    std::cout << "substituting\n";
                    return argument_id;
                }

                // If we've already duplicated this term, return the existing copy.
                auto itr = new_terms.find(term_id);
                if (itr != new_terms.end()) {
                    std::cout << "have copy already\n";
                    return itr->second;
                }

                // Otherwise we have to make a new copy.
                TermId new_term_id;
                if (stack.size() > 1) {
                    std::cout << "making copy\n";
                    new_term_id = term->visit(
                        [&](Variable var) {
                            return arena.make_variable(var.name);
                        },
                        [&](Abstraction) {
                            return arena.make_abstraction(
                                context->new_children[0],
                                context->new_children[1]
                            );
                        },
                        [&](Application) {
                            return arena.make_application(
                                context->new_children[0],
                                context->new_children[1]
                            );
                        }
                    );
                } else {
                    std::cout << "making copy (in-place)\n";
                    new_term_id = term->visit(
                        [&](Variable var) {
                            return arena.make_variable(new_root_id, var.name);
                        },
                        [&](Abstraction) {
                            return arena.make_abstraction(
                                new_root_id,
                                context->new_children[0],
                                context->new_children[1]
                            );
                        },
                        [&](Application) {
                            return arena.make_application(
                                new_root_id,
                                context->new_children[0],
                                context->new_children[1]
                            );
                        }
                    );
                }
                new_terms.emplace(term_id, new_term_id);
                return new_term_id;
            }();

            // If there is a previous stack element, store the new term id there.
            if (stack.size() > 1) {
                auto& parent_context = stack[stack.size() - 2];
                std::cout << "saving new term in slot " << context->idx << '\n';
                parent_context.new_children[context->idx] = new_term_id;
            }

            // If this term has more siblings, move to the next one.
            ++context->idx;
            if (context->idx < context->size) {
                break;
            }

            // Otherwise go up a level.
            stack.pop_back();
            if (stack.size() > 0) {
                context = &stack.back();
                term_id = context->terms[context->idx];
                term = &arena[term_id];
            } else {
                // If we've explored all nodes in the tree, return the copy of root_id.
                return new_term_id;
            }
        }
    }
}

//..................................................................................................
void beta_reduce(TermArena& arena, TermId term_id) {
    // Check that the term is an application.
    LambdaTerm& term = arena[term_id];
    if (!term.is_applicaton()) {
        throw std::runtime_error("Only applications can be reduced");
    }
    Application& application = term.get_application();

    // Check that the left term is an abstraction.
    TermId function_id = application.left;
    LambdaTerm& function = arena[function_id];
    if (!function.is_abstraction()) {
        throw std::runtime_error("Only applications of abstractions can be reduced");
    }
    Abstraction& abstraction = function.get_abstraction();

    // Generate the new term.
    // We make a deep copy of the tree (really DAG) underneath body_id. The copy of body_id itself
    // is created in place: it's data (i.e. variable, abstraction, or application info) replaces the
    // application info of term_id, but we leave term_id's parents in place.
    TermId body_id = abstraction.body;
    TermId variable_id = abstraction.variable;
    TermId argument_id = application.right;
    copy_and_substitute(arena, body_id, term_id, variable_id, argument_id);
    function.remove_parent(term_id);
    arena[argument_id].remove_parent(term_id);
}

}
