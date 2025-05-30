//
// Created by đỗ quyên on 14/5/25.
//

#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <string>
#include <utility>

#include "lexer/maps.h"

using std::string;

namespace bao {
    // --- Type base class ---
    class Type {
        string name;
    public:
        explicit Type(string name) : name(std::move(name)) {}
        virtual ~Type() = default;
        [[nodiscard]] string get_name() const { return name; }
        virtual std::unique_ptr<Type> clone() const = 0;
    };

    enum class Primitive {
        N32, // 32-bit unsigned integer
        N64, // 64-bit unsigned integer
        Z32, // 32-bit signed integer
        Z64, // 64-bit signed integer
        R32, // 32-bit float
        R64, // 64-bit float
        Void, // Void
        Null, // Null type
    };

    // --- Primitive type ---
    class PrimitiveType final : public Type {
        Primitive type;
    public:
        explicit PrimitiveType(const string &type) : Type(type), type(primitive_map.at(type)) {}
        [[nodiscard]] Primitive get_type() const { return type; }
        std::unique_ptr<Type> clone() const {
            return std::make_unique<PrimitiveType>(*this);
        }
    };

    class UnknownType final : public Type {
    public:
        explicit UnknownType() : Type("unknown") {}
        std::unique_ptr<Type> clone() const {
            return std::make_unique<UnknownType>(*this);
            // throw std::runtime_error("Không thể clone kiểu Unknown");
        }
    };
}
#endif //TYPES_H
