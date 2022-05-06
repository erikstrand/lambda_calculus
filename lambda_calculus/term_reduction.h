#ifndef LAMBDA_TERM_REDUCTION_H
#define LAMBDA_TERM_REDUCTION_H

#include "term_arena.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Makes a deep copy of the tree starting at root_id, substituting all occurences of the variable
// variable_id with argument_id.
TermId copy_and_substitute(
    TermArena& arena,
    TermId root_id,
    TermId variable_id,
    TermId argument_id
);

}

#endif
