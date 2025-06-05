#include <bao/codegen/bytecode/generator.h>
#include <ranges>

bao::bytecode::Generator :: Generator(
    bao::mir::Module&& mir_module
) : mir_module(std::move(mir_module)) {
    // Magic numbers
    this->bytecode = { 0xBA, 0x0B, 0xA0, 0xBA };
    
    // Append the version number
        for (int i = 0; i < 4; i++) {
        this->bytecode.push_back((BAO_VERSION >> (24 - i * 8)) & 0xFF);
    }
}