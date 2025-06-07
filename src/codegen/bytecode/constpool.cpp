#include <bao/codegen/bytecode/vminst.h>
#include <bao/codegen/bytecode/constpool.h>
#include <bao/common/utils.h>
#include <variant>

template<class... Ts> struct overloaded : Ts... {using Ts::operator()...;};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

bao::bytecode::ConstPool :: ConstPool() : index(0) {}

auto
bao::bytecode::ConstPool :: insert_const(
    auto& value
) -> uint16_t {
    // Return const value index if it exists in the map
    if (this->map.contains(PoolEntry(value))) {
        return this->map.at(PoolEntry(value));
    }
    auto opcode = std::visit(overloaded{
        [&](uint32_t val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_N32;
        },
        [&](uint64_t val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_N64;
        },
        [&](int32_t val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_Z32;
        },
        [&](int64_t val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_Z64;
        },
        [&](float val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_R32;
        },
        [&](double val) {
            this->map[PoolEntry(val)] = this->index;
            return bao::bytecode::VMInst::CONST_R64;
        }
    }, value);
    std::vector<BYTE> inst = {
        opcode
    };
    auto hexval = bao::utils::convert_to_bytes(value);
    for (auto& hexcode : hexval) {
        inst.push_back(
            hexcode
        );
    }
    this->bytecode.push_back(inst);
    return this->index++;
}