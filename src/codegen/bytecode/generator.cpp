#include "bao/codegen/bytecode/tags.h"
#include "bao/common/types.h"
#include "bao/common/utils.h"
#include "bao/mir/mir.h"
#include <bao/codegen/bytecode/generator.h>

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

void
bao::bytecode::Generator :: generate() {
    for (auto& func : this->mir_module.functions) {
        this->generate_function(func);
    }
}

void
bao::bytecode::Generator :: generate_function(
    bao::mir::Function& func
) {
    for (auto& block : func.blocks) {
        this->generate_block(block);
    }
}

void
bao::bytecode::Generator :: generate_block(
    bao::mir::BasicBlock& block
) {
    for (auto& inst : block.instructions) {
        this->generate_instruction(inst.get());
    }
}

void
bao::bytecode::Generator :: generate_instruction(
    bao::mir::Instruction* inst
) {
    if (auto retinst = dynamic_cast<mir::ReturnInst*>(inst)) {
        this->generate_retinst(retinst);
        return;
    }
    LOG_WARNING("Internal error: unknown instruction");
}

auto
bao::bytecode::Generator :: generate_value(
    bao::mir::Value& val
) -> uint16_t {
    if (val.kind == bao::mir::ValueKind::Constant) {
        if (auto prim = dynamic_cast<PrimitiveType*>(val.type.get())) {
            auto type = prim->get_type();
            if (type == bao::Primitive::N32) {
                uint32_t signed_integer = std::stoi(val.name);
                return this->const_pool.insert_const(signed_integer);
            }
            if (type == bao::Primitive::N64) {
                uint64_t signed_integer = std::stoi(val.name);
                return this->const_pool.insert_const(signed_integer);
            }
            if (type == bao::Primitive::Z32) {
                int32_t unsigned_integer = std::stoi(val.name);
                return this->const_pool.insert_const(unsigned_integer);
            }
            if (type == bao::Primitive::Z64) {
                int64_t unsigned_integer = std::stoi(val.name);
                return this->const_pool.insert_const(unsigned_integer);
            }
            if (type == bao::Primitive::R32) {
                float float_32 = std::stof(val.name);
                return this->const_pool.insert_const(float_32);
            }
            if (type == bao::Primitive::R64) {
                double float_64 = std::stod(val.name);
                return this->const_pool.insert_const(float_64);
            }
            LOG_WARNING("Internal error: unknown primitive");
            return 0;
        }
    }
    LOG_WARNING("Internal error: unknown value");
    return 0;
}

void
bao::bytecode::Generator :: generate_retinst(
    bao::mir::ReturnInst* retinst
) {
    // TODO: Handle null value
    auto val = retinst->ret_val;
    std::vector<BYTE> inst = { bao::bytecode::OpTag::RETURN };
    auto index = this->generate_value(val);
    auto index_hex = bao::utils::convert_to_bytes(index);
    for (auto& hex : index_hex) {
        inst.push_back(hex);
    }
    this->insts.push_back(inst);
}

auto
bao::bytecode::Generator :: dump_bytecode() -> std::vector<BYTE> {
    return this->bytecode;
}