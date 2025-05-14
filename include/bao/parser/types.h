//
// Created by đỗ quyên on 14/5/25.
//

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <utility>

using std::string;

namespace bao {
    class Type {
        string name;
    public:
        explicit Type(string name) : name(std::move(name)) {}
        virtual ~Type() = default;
        [[nodiscard]] string get_name() const { return name; }
    };

    class PrimitiveType final : public Type {
    public:
        explicit PrimitiveType(const string &name) : Type(name) {}
    };
}
#endif //TYPES_H
