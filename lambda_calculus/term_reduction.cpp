#include "term_reduction.h"
#include "utils/stdint.h"
#include "utils/visit.h"
#include <unordered_set>

#include <iostream>

namespace lambda {

//..................................................................................................
std::variant<TermId, LambdaTerm> substitute(
    TermArena& arena,
    TermId root_id,
    TermId variable_id,
    TermId argument_id
) {
    struct AnnotatedTerm {
        TermId id;
        bool is_new; // true when we made a new node, false when id points to an existing node
    };

    struct StackEntry {
        StackEntry(TermId term):
            terms{term, TermId{}}, new_children{}, size{1}, idx{0}
        {}

        StackEntry(TermId left, TermId right):
            terms{left, right}, new_children{}, size{2}, idx{0}
        {}

        std::array<TermId, 2> terms;
        std::array<AnnotatedTerm, 2> new_children;
        uint32_t size;
        uint32_t idx;
    };

    // Initialize the stack for a depth first traversal from root_id.
    std::vector<StackEntry> stack;
    stack.emplace_back(root_id);

    // This stores all terms that have been duplicated. It mpas the original TermId (as used in the
    // the tree beneath root_id) to that of the duplicate.
    std::unordered_map<TermId, AnnotatedTerm> new_terms;

    // TODO
    // I need two passes. In the first I build an array of all "dirty" nodes (those that will need
    // to be duplicated). I should ignore bound variables; they are basically part of their lambdas.
    // The interesting thing is that this makes them a non-local source of dirtiness. If anything
    // inside a lambda uses the outer bound variable, then the lambda needs to be duplicated, and
    // the new lambda needs its own bound variable. So all structures inside the old lambda that
    // depend on that bound variable are now dirty as well.
    // Hm so I guess really first I'm just detecting dirtiness with respect to the outer bound
    // variable. In the second pass I have to expand the definition to include terms that are dirty
    // with respect to the duplicated inner bound variables.
    //
    // In the second pass I do what I do here. Just I identify dirtiness sources using the
    // unordered_map I built in the first pass, instead of wherever the outer bound variable
    // appears.

    // This helper method traverses the tree.
    auto const traverse = [&]() {
        while (true) {
            StackEntry* context = &stack.back();
            TermId term_id = context->terms[context->idx];
            LambdaTerm* term = &arena[term_id];

            // Enter this term.
            bool const added_terms_to_stack = term->visit(
                [&](Variable var_tmp) {
                    std::cout << "entering variable (";
                    if (!var_tmp.abstraction.has_value()) {
                        std::cout << "un";
                    }
                    std::cout << "bound)\n";
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
                if (stack.size() == 1) {
                    return;
                }

                // Leave this term. This is when we create the duplicate node (if needed).
                AnnotatedTerm const new_term = [&]() {
                    // If this term is the bound variable, perform the substitution.
                    if (term_id == variable_id) {
                        std::cout << "  substituting\n";
                        return AnnotatedTerm{argument_id, true};
                    }

                    // If we've already covered this term, return the existing copy.
                    auto itr = new_terms.find(term_id);
                    if (itr != new_terms.end()) {
                        std::cout << "  have copy (or re-used node) already\n";
                        return itr->second;
                    }

                    // Otherwise we have to make a new copy.
                    AnnotatedTerm new_term = term->visit(
                        [term_id](Variable) {
                            // This isn't the bound variable, so it can't depend on the argument.
                            std::cout << "  re-using variable\n";
                            return AnnotatedTerm{term_id, false};
                        },
                        [&](Abstraction) {
                            AnnotatedTerm child_0 = context->new_children[0];
                            AnnotatedTerm child_1 = context->new_children[1];
                            if (child_0.is_new || child_1.is_new) {
                                std::cout << "  duplicating abstraction\n";
                                return AnnotatedTerm{
                                    arena.make_abstraction(child_0.id, child_1.id),
                                    true
                                };
                            } else {
                                std::cout << "  re-using abstraction\n";
                                return AnnotatedTerm{term_id, false};
                            }
                        },
                        [&](Application) {
                            AnnotatedTerm child_0 = context->new_children[0];
                            AnnotatedTerm child_1 = context->new_children[1];
                            if (child_0.is_new || child_1.is_new) {
                                std::cout << "  duplicating application\n";
                                return AnnotatedTerm{
                                    arena.make_application(child_0.id, child_1.id),
                                    true
                                };
                            } else {
                                std::cout << "  re-using application\n";
                                return AnnotatedTerm{term_id, false};
                            }
                        }
                    );
                    new_terms.emplace(term_id, new_term);
                    return new_term;
                }();

                // Store the resulting term one level up the stack.
                auto& parent_context = stack[stack.size() - 2];
                parent_context.new_children[context->idx] = new_term;

                // If this term has more siblings, move to the next one.
                ++context->idx;
                if (context->idx < context->size) {
                    break;
                }

                // Otherwise go up a level.
                stack.pop_back();
                context = &stack.back();
                term_id = context->terms[context->idx];
                term = &arena[term_id];
            }
        }
    };

    // Do the traverse.
    traverse();

    // Build the final node. This is different from what we do inside traverse(), since if we have
    // to construct a node we don't do it inside the arena.
    auto const context = stack.back();
    return arena[root_id].visit(
        [=](Variable) -> std::variant<TermId, LambdaTerm> {
            std::cout << "  re-using variable (root)\n";
            return (root_id == variable_id) ? argument_id : root_id;
        },
        [=](Abstraction) -> std::variant<TermId, LambdaTerm> {
            AnnotatedTerm child_0 = context.new_children[0];
            AnnotatedTerm child_1 = context.new_children[1];
            if (child_0.is_new || child_1.is_new) {
                std::cout << "  duplicating abstraction (root)\n";
                return LambdaTerm{{}, Abstraction{child_0.id, child_1.id}};
            } else {
                std::cout << "  re-using abstraction (root)\n";
                return root_id;
            }
        },
        [=](Application) -> std::variant<TermId, LambdaTerm> {
            AnnotatedTerm child_0 = context.new_children[0];
            AnnotatedTerm child_1 = context.new_children[1];
            if (child_0.is_new || child_1.is_new) {
                std::cout << "  duplicating application (root)\n";
                return LambdaTerm{{}, Application{child_0.id, child_1.id}};
            } else {
                std::cout << "  re-using application (root)\n";
                return root_id;
            }
        }
    );
}

//..................................................................................................
TermId beta_reduce(TermArena& arena, TermId term_id) {
    // Check that the term is an application.
    LambdaTerm& term = arena[term_id];
    if (!term.is_applicaton()) {
        throw std::runtime_error("Only applications can be reduced");
    }
    Application& application = term.get_application();
    TermId const function_id = application.left;
    TermId const argument_id = application.right;

    // Check that the application's left term is an abstraction.
    LambdaTerm& function = arena[function_id];
    if (!function.is_abstraction()) {
        throw std::runtime_error("Only applications of abstractions can be reduced");
    }
    Abstraction& abstraction = function.get_abstraction();
    TermId body_id = abstraction.body;
    TermId variable_id = abstraction.variable;

    // Perform the substitution and splice in the new node.
    std::variant<TermId, LambdaTerm> new_root =
        substitute(arena, body_id, variable_id, argument_id);
    return lambda::visit(
        new_root,
        [&](TermId new_id) {
            // If the new root is an existing node, we remap the current node to it.
            // Note: if we start tracking memory more closely, we could free term_id here.
            arena.replace_term(term_id, new_id);
            return new_id;
        },
        [&](LambdaTerm new_term) {
            // If the root node is a new node, we reuse the current node.
            arena.replace_application(term_id, new_term);
            return term_id;
        }
    );
}

//..................................................................................................
TermId reduce_normal_order(TermArena& arena, TermId root_id) {
    struct StackEntry {
        StackEntry(TermId term): children{term, TermId{}}, size{1}, idx{0} {}
        StackEntry(TermId left, TermId right): children{left, right}, size{2}, idx{0} {}

        std::array<TermId, 2> children;
        uint32_t size;
        uint32_t idx;
    };

    std::vector<StackEntry> stack;
    stack.emplace_back(root_id);

    auto const get_term_id = [&stack](uint32_t depth) -> TermId {
        return stack[depth].children[stack[depth].idx];
    };

    // This stores all terms that have been fully reduced, so we know not to traverse them again.
    std::unordered_set<TermId> reduced_terms;

    while (true) {
        TermId term_id = get_term_id(stack.size() - 1);

        if (!reduced_terms.contains(term_id)) {
            // Enter this term, and add its children (if any) to the stack.
            bool const modified_stack = arena[term_id].visit(
                [&](Variable) {
                    return false;
                },
                [&](Abstraction abstraction) {
                    stack.emplace_back(abstraction.body);
                    return true;
                },
                [&](Application application) {
                    // If this term is a redex, reduce it.
                    if (arena[application.left].is_abstraction()) {
                        term_id = beta_reduce(arena, term_id);
                        // It's possible that our parent is now a redex.
                        stack.pop_back();
                        if (stack.size() == 0) {
                            stack.emplace_back(term_id);
                        }
                        return true;
                    }

                    // Otherwise, continue walking down.
                    stack.emplace_back(application.left, application.right);
                    return true;
                }
            );

            // If we modified the stack, go down (or up) one level.
            if (modified_stack) {
                continue;
            }
        }

        // Otherwise we need to backtrack.
        while (true) {
            // We have reduced everything below here.
            reduced_terms.insert(term_id);

            // If this term has more siblings, move to the next one.
            ++stack.back().idx;
            if (stack.back().idx < stack.back().size) {
                // This means it was an application.
                // We want a space between the left and right terms.
                break;
            }

            // If this is the last node in the stack, we're done.
            if (stack.size() == 1) {
                return term_id;
            }

            // Otherwise go up a level.
            stack.pop_back();
            term_id = get_term_id(stack.size() - 1);
        }
    }
}

}
