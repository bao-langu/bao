#ifndef BYTECODE_TAG_H
#define BYTECODE_TAG_H
#include <cstdint>

typedef uint8_t BYTE;

namespace bao::bytecode {
    enum ConstTag : BYTE {
        N32 = 0x00,
        N64 = 0x01,
        Z32 = 0x02,
        Z64 = 0x03,
        R32 = 0x04,
        R64 = 0x05,
    };

    enum OpTag : BYTE {
        FUNC_DECL = 0x00,
        FUNC_END = 0x01,
        RETURN = 0x02,
    };
}

#endif