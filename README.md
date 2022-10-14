# Lambda Calculus

This is a C++20 implementation of the untyped lambda calculus with no external dependencies.

For efficiency, lambda terms are represented as graphs instead of trees. So if you reduce an
application of an abstraction that uses its argument twice, both instances will point to the same
`LambdaTerm`.

The code assumes that `stdout` accepts the `UTF-8` format, so that we can serialize lambda terms
using the `λ` character. This is by default true on almost all Linux and Mac systems, but is not
true on Windows machines. If you're on Windows, you probably just want to change `λ` to some ASCII
character in [term_serialization.cpp](lambda_calculus/term_serialization.cpp).

Build it using `cmake`.

```
mkdir build; cd build
cmake ..
make -j
```

This will produce an executable called `lambda_calculus` (in the directory `build/lambda_calculus`).
