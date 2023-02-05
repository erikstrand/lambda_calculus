#ifndef LAMBDA_TERM_SERIALIZATION_H
#define LAMBDA_TERM_SERIALIZATION_H

#include "term_arena.h"
#include <ostream>

namespace lambda {

//--------------------------------------------------------------------------------------------------
// Assumes the stream expects UTF-8 text.
void serialize_term(TermArena const& arena, TermId root, std::ostream& stream);

//--------------------------------------------------------------------------------------------------
struct TermPrinter {
    TermPrinter(TermArena const& arena, TermId root) : arena(arena), root(root) {}

    TermArena const& arena;
    TermId root;
};

//--------------------------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& stream, TermPrinter const& printer) {
    serialize_term(printer.arena, printer.root, stream);
    return stream;
}

//--------------------------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& stream, TermId term_id) {
    return stream << term_id.value();
}

}

#endif
