add_library(shared_settings INTERFACE)

# Speed flags
target_compile_options(shared_settings INTERFACE -march=native)

# Warning flags
target_compile_options(shared_settings INTERFACE
    -Wall
    -Wcast-align
    -Wcast-qual
    -Wextra
    -Wundef
    -Wzero-as-null-pointer-constant
    -pedantic
)

target_compile_features(shared_settings INTERFACE cxx_std_20)
