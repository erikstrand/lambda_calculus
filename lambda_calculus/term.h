#ifndef LAMBDA_TERM_H
#define LAMBDA_TERM_H

#include <string_view>
#include <variant>
#include "term_id.h"

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
};

}

#endif
