#ifndef LAMBDA_TERM_REDUCTION_H
#define LAMBDA_TERM_REDUCTION_H

#include "term_arena.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Remaps the parents of old_id so they point to new_id (wherever they used to point to old_id).
// Only works if old_id is an abstraction or application. (If it were a variable we'd also have to
// consider its owning abstraction as a parent.)
void remap_parents(TermArena& arena, TermId old_id, TermId new_id);

//--------------------------------------------------------------------------------------------------
// Walks down the tree starting at root_id, substituting all occurences of variable_id with
// argument_id. Portions of the tree that don't depend on variable_id are re-used; those that do are
// duplicated.
std::variant<TermId, LambdaTerm> substitute(
    TermArena& arena,
    TermId root_id,
    TermId variable_id,
    TermId argument_id
);

//--------------------------------------------------------------------------------------------------
TermId beta_reduce(TermArena& arena, TermId term_id);

}

#endif
