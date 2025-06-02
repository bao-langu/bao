//
// Created by đỗ quyên on 17/5/25.
//

#ifndef SYMTABL_H
#define SYMTABL_H
#include <iostream>
#include <unordered_map>
#include <bao/common/types.h>

namespace bao::sema {
    enum class SymbolType {
        Variable,
        Function,
        Type
    };

    struct SymbolInfo {
        SymbolType type;
        Type* datatype;
        bool isConst;
    };

    class SymbolTable {
        std::unordered_map<std::string, SymbolInfo> table;
        SymbolTable* parent;
    public:
        SymbolTable(): parent(nullptr) {}
        SymbolTable(SymbolTable* parent): parent(parent) {}

        bool insert(const std::string& name, const SymbolInfo& info) {
            return table.emplace(name, info).second;
        }

        SymbolInfo* lookup(const std::string& name) {
            const auto it = table.find(name);
            if (it != table.end()) {
                return &it->second;
            }
            if (parent) {
                return parent->lookup(name);
            }
            return nullptr;
        }

        bool remove(const std::string& name) {
            return table.erase(name) > 0;
        }

        void dump() const {
            for (const auto& [name, info] : table) {
                std::cout << "Name: " << name
                        << ", Type: " << static_cast<int>(info.type)
                        << ", DataType: " << info.datatype->get_name();
            }
        }
    };
}
#endif //SYMTABL_H
