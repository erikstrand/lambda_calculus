#ifndef LAMBDA_UTILS_VISIT_H
#define LAMBDA_UTILS_VISIT_H

#include "overloaded.h"
#include <variant>

namespace lambda {

//--------------------------------------------------------------------------------------------------
// This is a visitor application method that takes one variant and any number of callable objects.
// Contrast with std::visit, which takes one callable object and any number of variants.
template <typename Variant, typename... Visitors>
constexpr decltype(auto) visit(
    Variant&& variant,
    Visitors&&... visitors
) {
    return std::visit(
        overloaded{std::forward<Visitors>(visitors)...},
        std::forward<Variant>(variant)
    );
}

}

#endif
