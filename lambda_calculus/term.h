#ifndef LAMBDA_TERM_H
#define LAMBDA_TERM_H

#include <string_view>
#include <variant>
#include <stdexcept>
#include "term_id.h"
#include "utils/overloaded.h"

namespace lambda {

//--------------------------------------------------------------------------------------------------
struct Variable {
    Variable(std::string_view n): name(n) {}
    bool is_bound() const { return abstraction.has_value(); }

    // if this is a bound variable, this refers to the owning abstraction
    std::optional<TermId> abstraction;
    std::string_view name;
};

//--------------------------------------------------------------------------------------------------
struct Abstraction {
    TermId variable;
    TermId body;
};

//--------------------------------------------------------------------------------------------------
struct Application {
    TermId left;
    TermId right;
};

//--------------------------------------------------------------------------------------------------
using LambdaVariant = std::variant<Variable, Abstraction, Application>;

//--------------------------------------------------------------------------------------------------
struct LambdaTerm {
    std::vector<TermId> parents;
    LambdaVariant data;

    template <typename... Visitors>
    constexpr decltype(auto) visit(Visitors&&... visitors) const {
        return std::visit(overloaded{std::forward<Visitors>(visitors)...}, data);
    }

    template <typename... Visitors>
    constexpr decltype(auto) visit(Visitors&&... visitors) {
        return std::visit(overloaded{std::forward<Visitors>(visitors)...}, data);
    }

    bool is_variable() const {
        return visit([](Variable) { return true; }, [](auto) { return false; });
    }
    bool is_abstraction() const {
        return visit([](Abstraction) { return true; }, [](auto) { return false; });
    }
    bool is_applicaton() const {
        return visit([](Application) { return true; }, [](auto) { return false; });
    }

    Variable& get_variable() { return std::get<Variable>(data); }
    Abstraction& get_abstraction() { return std::get<Abstraction>(data); }
    Application& get_application() { return std::get<Application>(data); }

    // Throws an exception if this isn't a variable.
    std::string_view get_name() const {
        return visit(
            [&](Variable variable) { return variable.name; },
            [](auto) -> std::string_view { throw std::runtime_error("Expected variable"); }
        );
    }

    void add_parent(TermId parent_id) { parents.push_back(parent_id); }
    void remove_parent(TermId parent_id) {
        // This is linear time, but very cache friendly.
        std::remove(parents.begin(), parents.end(), parent_id);
        // Note that we only remove one occurence of parent_id. Others may remain.
        parents.pop_back();
    }
};

}

#endif
