#include "term_arena.h"

namespace lambda {

//..................................................................................................
void TermArena::replace_term(TermId old_id, TermId new_id) {
    LambdaTerm& old_term = operator[](old_id);
    uint32_t const n_parents = old_term.parents.size();
    if (n_parents == 0) {
        return;
    }

    // This helper method remaps a single parent.
    auto const remap_parent = [this, old_id, new_id](TermId parent_id) {
        LambdaTerm& parent = operator[](parent_id);
        parent.visit(
            [](Variable) {
                throw std::runtime_error("A variable cannot be a parent");
            },
            [old_id, new_id](Abstraction& abstraction) {
                if (abstraction.body == old_id) {
                    abstraction.body = new_id;
                }
            },
            [old_id, new_id](Application& application) {
                if (application.left == old_id) {
                    application.left = new_id;
                }
                if (application.right == old_id) {
                    application.right = new_id;
                }
            }
        );
    };

    // Note: if there are duplicate parents here we could skip them.
    // This would be easy if we kept our parent lists sorted.
    for (TermId parent_id : old_term.parents) {
        remap_parent(parent_id);
    }

    // Donate the old_term's parents to the new term.
    LambdaTerm& new_term = operator[](new_id);
    if (new_term.parents.size() == 0) {
        new_term.parents = std::move(old_term.parents);
    } else {
        new_term.parents.insert(
            new_term.parents.end(),
            old_term.parents.begin(),
            old_term.parents.end()
        );
    }
    old_term.parents.clear();

    // If the old term is a bound variable, we also have to update its abstraction.
    if (old_term.is_variable()) {
        if (!new_term.is_variable()) {
            throw std::runtime_error("Bound variables can only be replaced with other variables");
        }

        Variable& old_variable = old_term.get_variable();
        if (old_variable.abstraction.has_value()) {
            TermId abstraction_id = old_variable.abstraction.value();
            Abstraction& abstraction = operator[](abstraction_id).get_abstraction();
            if (abstraction.variable != old_id) {
                throw std::runtime_error("Can't replace corrupt bound variable");
            }
            old_variable.abstraction.reset();
            abstraction.variable = new_id;
        }
    }
}

//..................................................................................................
void TermArena::replace_application(TermId old_id, LambdaTerm new_term) {
    LambdaTerm& term = operator[](old_id);

    // Remove the current children.
    term.visit(
        [this, old_id](Application& application) {
            operator[](application.left).remove_parent(old_id);
            operator[](application.right).remove_parent(old_id);
        },
        [](auto) {
            throw std::runtime_error("Expected application");
        }
    );

    // Attach the new children.
    term.data = new_term.data;

    // The children of new_term didn't know about their parent yet, since it didn't have an id.
    term.visit(
        [](Variable) {},
        [&](Abstraction& abstraction) {
            bind_variable(abstraction.variable, old_id);
            operator[](abstraction.body).add_parent(old_id);
        },
        [&](Application& application) {
            operator[](application.left).add_parent(old_id);
            operator[](application.right).add_parent(old_id);
        }
    );
}

}
