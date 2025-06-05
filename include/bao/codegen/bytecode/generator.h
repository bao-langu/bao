#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H
#include <bao/mir/mir.h>
#include <cstdint>
#include <unordered_map>

typedef uint8_t BYTE;

namespace bao::bytecode {
    class Generator {
        bao::mir::Module mir_module;
        std::vector<BYTE> bytecode;
    public:
        Generator(bao::mir::Module&& mir_module);
        void generate();
        void print_source();
        int create_bytecode(const std::string& filename);
    private:
    };
}

#endif