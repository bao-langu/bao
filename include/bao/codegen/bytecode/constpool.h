#ifndef CONSTPOOL_H
#define CONSTPOOL_H

#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

typedef uint8_t BYTE;

namespace bao::bytecode {
    struct PoolEntry {
        std::variant<uint32_t, int32_t, uint64_t, int64_t, float, double> value;

        bool operator==(const PoolEntry& other) {
            return this->value == other.value;
        }
    };
}

namespace std {
    template<>
    struct hash<bao::bytecode::PoolEntry> {
        std::size_t operator()(const bao::bytecode::PoolEntry& p) const {
            return std::visit([](const auto& v) -> std::size_t {
                return std::hash<std::decay_t<decltype(v)>>{}(v);
            }, p.value);
        }
    };
}

namespace bao::bytecode {
    class ConstPool {
    private:
        std::vector<std::vector<BYTE>> bytecode;
        std::unordered_map<PoolEntry, int> map;
        int index;
    public:
        ConstPool();
        uint16_t insert_const(auto& value);
        void insert_type(auto& value);
        void look_up(std::string value);
        std::vector<BYTE> get_bytecode();
    };
}

#endif