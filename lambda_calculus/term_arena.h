#ifndef LAMBDA_TERM_ARENA_H
#define LAMBDA_TERM_ARENA_H

#include <vector>
#include "term.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
class TermArena {
public:
    void reserve(uint32_t capacity) { pool_.reserve(capacity); }

    TermId make_variable(std::string_view name) {
        return construct(std::vector<TermId>{}, Variable{name});
    }

    TermId make_abstraction(TermId var, TermId body) {
        TermId const idx = construct(std::vector<TermId>{}, Abstraction{var, body});
        LambdaTerm& var_term = operator[](var);
        if (!var_term.is_variable()) {
            throw std::runtime_error("Expected variable");
        }
        var_term.parents.push_back(idx);
        operator[](body).parents.push_back(idx);
        return idx;
    }

    TermId make_application(TermId left, TermId right) {
        TermId const idx = construct(std::vector<TermId>{}, Application{left, right});
        operator[](left).parents.push_back(idx);
        operator[](right).parents.push_back(idx);
        return idx;
    }

    LambdaTerm& operator[](TermId idx) { return pool_[idx.value()]; }
    LambdaTerm const& operator[](TermId idx) const { return pool_[idx.value()]; }

private:
    template <class... Args>
    TermId construct(Args&&... args) {
        auto const idx = static_cast<uint32_t>(pool_.size());
        pool_.emplace_back(std::forward<Args>(args)...);
        return idx;
    }

    std::vector<LambdaTerm> pool_;
};

}

#endif
