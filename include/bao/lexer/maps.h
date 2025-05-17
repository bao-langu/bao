//
// Created by doqin on 14/05/2025.
//

#ifndef MAPS_H
#define MAPS_H

#include <unordered_map>
#include <string>

using std::string;

namespace bao {
    struct Token;
    enum class Primitive;
    enum class TokenType;
    extern const std::unordered_map<TokenType, string> token_type_map;
    extern const std::unordered_map<string, Token> token_map;
    extern const std::unordered_map<string, Primitive> primitive_map;
}

#endif //MAPS_H
