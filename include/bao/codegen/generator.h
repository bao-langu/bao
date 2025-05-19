#ifndef GENERATOR_H
#define GENERATOR_H

#include <bao/mir/mir.h>
#include <bao/parser/ast.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace bao {
    class Generator {
        bao::mir::Module mir_module;
        llvm::LLVMContext context;
        llvm::Module llvm_module;
        llvm::IRBuilder<> ir_builder;
    public:
        Generator(bao::mir::Module&& mir_module);
        void generate();
        void print_source();
        int create_object(const std::string& filename);
    private:
        void generate_function(bao::mir::Function& mir_func);
        void generate_block(llvm::BasicBlock* ir_block, bao::mir::BasicBlock& mir_block);
        void generate_instruction(bao::mir::Instruction* mir_inst);
    };
}
#endif // GENERATOR_H