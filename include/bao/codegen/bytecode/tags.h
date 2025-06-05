#ifndef BYTECODE_TAG_H
#define BYTECODE_TAG_H
#include <cstdint>

typedef uint8_t BYTE;

namespace bao::bytecode {
    enum class ConstTag : BYTE {
        N32,
        N64,
        Z32,
        Z64,
        R32,
        R64
    };
}

#endif