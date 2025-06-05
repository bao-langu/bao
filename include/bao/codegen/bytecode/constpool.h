#ifndef BYTECODE_CONSTPOOL_H
#define BYTECODE_CONSTPOOL_H

#include <bao/codegen/bytecode/tags.h>
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>

namespace bao::bytecode {
    struct ConstValue {
        bao::bytecode::ConstTag tag;
        std::variant<uint32_t, uint64_t, int32_t, int64_t, float, double, std::string> value;

        bool operator==(const ConstValue& other) const {
            return this->tag == other.tag && this->value == other.value;
        }
    };
}

namespace std {
    template<>
    struct hash<bao::bytecode::ConstValue> {
        size_t operator()(const bao::bytecode::ConstValue& cv) const {
            size_t h1 = std::hash<int>()(static_cast<int>(cv.tag));
            size_t h2 = 
                std::visit(
                    [](auto&& v) { 
                        return std::hash<std::decay_t<decltype(v)>>()(v); 
                    }, 
                    cv.value
                );
            return h1 ^ (h2 << 1);
        }
    };
}

namespace bao::bytecode {
    class ConstPool {
    public:
        int getOrAdd(ConstValue value) {
            auto it = indexMap.find(value);
            if (it != indexMap.end()) {
                return it->second;
            }
            int index = pool.size();
            pool.push_back(value);
            indexMap[value] = index;
            return index;
        }

        const std::vector<ConstValue>& getAll() const {
            return pool;
        }
    private:
        std::vector<ConstValue> pool;
        std::unordered_map<ConstValue, int> indexMap;
    };
}

#endif