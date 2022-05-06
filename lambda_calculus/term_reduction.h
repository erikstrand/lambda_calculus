#ifndef LAMBDA_TERM_REDUCTION_H
#define LAMBDA_TERM_REDUCTION_H

#include "term_arena.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Like the method below, except the copy of the root is placed in an existing node (new_root_id).
TermId copy_and_substitute(
    TermArena& arena,
    TermId root_id,
    TermId new_root_id,
    TermId variable_id,
    TermId argument_id
);

//--------------------------------------------------------------------------------------------------
// Makes a deep copy of the tree starting at root_id, substituting all occurences of the variable
// variable_id with argument_id.
inline TermId copy_and_substitute(
    TermArena& arena,
    TermId root_id,
    TermId variable_id,
    TermId argument_id
) {
    return copy_and_substitute(
        arena,
        root_id,
        arena.make_variable("tmp"),
        variable_id,
        argument_id
    );
}

}

#endif
