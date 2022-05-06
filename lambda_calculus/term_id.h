#ifndef LAMBDA_TERM_ID_H
#define LAMBDA_TERM_ID_H

#include "utils/stdint.h"
#include <functional> // std::hash

namespace lambda {

//--------------------------------------------------------------------------------------------------
class TermArena;

//--------------------------------------------------------------------------------------------------
class TermId {
public:
    TermId() = default;
    uint32_t value() const { return idx; }

    bool operator==(TermId rhs) const { return idx == rhs.idx; }
    bool operator<(TermId rhs) const { return idx < rhs.idx; }

private:
    friend class TermArena;
    TermId(uint32_t i): idx(i) {}

    uint32_t idx;
};

}

namespace std {

//--------------------------------------------------------------------------------------------------
template <>
struct hash<lambda::TermId> {
    std::size_t operator()(lambda::TermId term_id) const {
        return std::hash<uint32_t>{}(term_id.value());
    }
};

}

#endif
