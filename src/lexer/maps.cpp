//
// Created by đỗ quyên on 17/5/25.
//
#include <bao/lexer/maps.h>
#include <bao/lexer/token.h>
#include <bao/common/types.h>

namespace bao {
    const std::unordered_map<TokenType, string> token_type_map = {
        {TokenType::Operator,   "Operator"},
        {TokenType::Identifier, "Identifier"},
        {TokenType::Keyword,    "Keyword"},
        {TokenType::Literal,    "Literal"},
        {TokenType::String,     "String"},
        {TokenType::Semicolon,  "Semicolon"},
        {TokenType::Comma,      "Comma"},
        {TokenType::LParen,     "LParen"},
        {TokenType::RParen,     "RParen"},
        {TokenType::LBracket,   "LBracket"},
        {TokenType::RBracket,   "RBracket"},
        {TokenType::LBrace,     "LBrace"},
        {TokenType::RBrace,     "RBrace"},
        {TokenType::Newline,    "Newline"},
        {TokenType::EndOfFile,  "EndOfFile"},
        {TokenType::Unknown,    "Unknown"}
    };

    const std::unordered_map<string, Token> token_map = {
        {"(", Token{TokenType::LParen,      "("}},
        {")", Token{TokenType::RParen,      ")"}},
        {"[", Token{TokenType::LBracket,    "["}},
        {"]", Token{TokenType::RBracket,    "]"}},
        {";", Token{TokenType::Semicolon,   ";"}},
        {",", Token{TokenType::Comma,       ","}},
    };

    const std::unordered_map<string, Primitive> primitive_map = {
        {"N32", Primitive::N32},
        {"N64", Primitive::N64},
        {"Z32", Primitive::Z32},
        {"Z64", Primitive::Z64},
        {"R32", Primitive::R32},
        {"R64", Primitive::R64},
        {"rỗng", Primitive::Void},
        {"null", Primitive::Null}
    };
}
