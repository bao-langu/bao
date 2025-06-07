#ifndef VMINST_H
#define VMINST_H

#include <cstdint>
typedef uint8_t BYTE;

namespace bao::bytecode {
    enum VMInst : BYTE{
        CONST_N32,
        CONST_N64,
        CONST_Z32,
        CONST_Z64,
        CONST_R32,
        CONST_R64
    };
}

#endif