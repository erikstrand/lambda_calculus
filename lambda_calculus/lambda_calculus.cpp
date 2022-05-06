#include <bit>
#include <cstring>
#include <iostream>

//--------------------------------------------------------------------------------------------------
char8_t const* next_utf8_codepoint(char8_t const* pos) {
    auto const n_leading_ones = std::countl_one(*reinterpret_cast<unsigned char const*>(pos));
    switch (n_leading_ones) {
        case 0:
            // single byte
            return pos + 1;
        case 1:
            // invalid
            return nullptr;
        case 2:
            // two bytes
            if ((*(pos + 1) & 0xC0) != 0x80) {
                return nullptr;
            }
            return pos + 2;
        case 3:
            // three bytes
            if ((*(pos + 1) & 0xC0) != 0x80) {
                return nullptr;
            }
            if ((*(pos + 2) & 0xC0) != 0x80) {
                return nullptr;
            }
            return pos + 3;
        case 4:
            // four bytes
            if ((*(pos + 1) & 0xC0) != 0x80) {
                return nullptr;
            }
            if ((*(pos + 2) & 0xC0) != 0x80) {
                return nullptr;
            }
            if ((*(pos + 3) & 0xC0) != 0x80) {
                return nullptr;
            }
            return pos + 4;
        default:
            // invalid (or end of string)
            return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
int main() {
    char const* source = "λx.x";
    std::cout << source << '\n';

    auto length = std::strlen(source);
    std::cout << "length (bytes): " << length << '\n';

    length = 0;
    char8_t const* pos = reinterpret_cast<char8_t const*>(source);
    while (pos && *pos) {
        pos = next_utf8_codepoint(pos);
        ++length;
    }
    std::cout << "length (code points): " << length << '\n';
}
