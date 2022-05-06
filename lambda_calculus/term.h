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

    bool is_variable() const {
        return visit([](Variable) { return true; }, [](auto) { return false; });
    }

    // Throws an exception if this isn't a variable.
    std::string_view get_name() const {
        return visit(
            [&](Variable variable) { return variable.name; },
            [](auto) -> std::string_view { throw std::runtime_error("Expected variable"); }
        );
    }
};

}

#endif
