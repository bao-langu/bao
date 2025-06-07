#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H
#include <bao/codegen/bytecode/constpool.h>
#include <bao/mir/mir.h>
#include <cstdint>


typedef uint8_t BYTE;

namespace bao::bytecode {
    class Generator {
    private:
        bao::mir::Module mir_module;
        std::vector<std::vector<BYTE>> insts;
        std::vector<BYTE> bytecode;
        ConstPool const_pool;
    public:
        Generator(bao::mir::Module&& mir_module);
        void generate();
        void print_bytecode();
        int create_file(const std::string& filename);
        std::vector<BYTE> dump_bytecode();
    private:
        // Main methods
        void generate_function(bao::mir::Function& func);
        void generate_block(bao::mir::BasicBlock& block);
        void generate_instruction(bao::mir::Instruction* inst);
        uint16_t generate_value(bao::mir::Value& val);

        // Instruction methods
        void generate_retinst(bao::mir::ReturnInst* retinst);
    };
}

#endif