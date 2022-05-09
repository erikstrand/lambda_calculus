#ifndef LAMBDA_TERM_ARENA_H
#define LAMBDA_TERM_ARENA_H

#include <vector>
#include "term.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
class TermArena {
public:
    void reserve(std::size_t capacity) { pool_.reserve(capacity); }
    std::size_t size() const { return pool_.size(); }

    TermId make_variable(std::string_view name) {
        return construct(std::vector<TermId>{}, Variable{name});
    }

    TermId make_abstraction(TermId var, TermId body) {
        TermId const idx = construct(std::vector<TermId>{}, Abstraction{var, body});
        bind_variable(var, idx);
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

    // Replaces old_id with new_id. Specifically, all the parents of old_id are remapped to point to
    // new_id, and new_id has its parent list updated accordingly. The parent list of old_id is
    // cleared. This method works for any type of term.
    void replace_term(TermId old_id, TermId new_id);

    // Replaces old_id with new_id. Specifically, all the children of old_id are cut loose, and
    // replaced with those of new_term. The parents of old_id remain intact. This method expects
    // old_id to be an application, since I only use this during beta reduction.
    void replace_application(TermId old_id, LambdaTerm new_term);

private:
    template <class... Args>
    TermId construct(Args&&... args) {
        auto const idx = static_cast<uint32_t>(pool_.size());
        pool_.emplace_back(std::forward<Args>(args)...);
        return idx;
    }

    void bind_variable(TermId variable_id, TermId abstraction_id) {
        LambdaTerm& variable_term = operator[](variable_id);
        variable_term.visit(
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
