#ifndef LAMBDA_TERM_SERIALIZATION_H
#define LAMBDA_TERM_SERIALIZATION_H

#include "term_arena.h"
#include <ostream>

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Assumes the stream expects UTF-8 text.
void serialize_term(TermArena const& arena, TermId root, std::ostream& stream);

}

#endif
