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
    TermId make_variable(TermId placement, std::string_view name) {
        operator[](placement).data = Variable{name};
        return placement;
    }

    TermId make_abstraction(TermId var, TermId body) {
        TermId const idx = construct(std::vector<TermId>{}, Abstraction{var, body});
        bind_variable(var, idx);
        operator[](body).parents.push_back(idx);
        return idx;
    }
    TermId make_abstraction(TermId placement, TermId var, TermId body) {
        operator[](placement).data = Abstraction{var, body};
        bind_variable(var, placement);
        operator[](body).parents.push_back(placement);
        return placement;
    }

    TermId make_application(TermId left, TermId right) {
        TermId const idx = construct(std::vector<TermId>{}, Application{left, right});
        operator[](left).parents.push_back(idx);
        operator[](right).parents.push_back(idx);
        return idx;
    }
    TermId make_application(TermId placement, TermId left, TermId right) {
        operator[](placement).data = Application{left, right};
        operator[](left).parents.push_back(placement);
        operator[](right).parents.push_back(placement);
        return placement;
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

    void bind_variable(TermId variable_id, TermId abstraction_id) {
        LambdaTerm& var_term = operator[](variable_id);
        var_term.visit(
            [abstraction_id](Variable& variable) {
                if (variable.abstraction.has_value()) {
                    throw std::runtime_error("Variable is already bound");
                }
                variable.abstraction = abstraction_id;
            },
            [](auto) { throw std::runtime_error("Expected variable"); }
        );
    }

    std::vector<LambdaTerm> pool_;
};

}

#endif
