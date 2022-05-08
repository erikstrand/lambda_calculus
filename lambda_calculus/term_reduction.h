#ifndef LAMBDA_TERM_REDUCTION_H
#define LAMBDA_TERM_REDUCTION_H

#include "term_arena.h"
#include <unordered_set>

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Walks down the tree starting at root_id, collecting all terms that directly depend on
// variable_id. Bound variables of inner lambdas are not added along with their abstractions, so a
// second pass is needed to propagate up these indirect dependencies.
std::unordered_set<TermId> find_terms_with_bound_variable(
    TermArena const& arena,
    TermId root_id,
    TermId variable_id
);

//--------------------------------------------------------------------------------------------------
// Walks down the tree starting at root_id, substituting all occurences of variable_id with
// argument_id. Portions of the tree that don't depend on variable_id are re-used; those that do are
// duplicated. Note that if the returned result is a LambdaTerm, its children don't know that it is
// their parent (since it doesn't yet have an id).
std::variant<TermId, LambdaTerm> substitute(
    TermArena& arena,
    TermId root_id,
    TermId variable_id,
    TermId argument_id
);

//--------------------------------------------------------------------------------------------------
TermId beta_reduce(TermArena& arena, TermId term_id);

//--------------------------------------------------------------------------------------------------
TermId reduce_normal_order(TermArena& arena, TermId root_id);

}

#endif
