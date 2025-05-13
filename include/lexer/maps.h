//
// Created by doqin on 14/05/2025.
//

#ifndef MAPS_H
#define MAPS_H

#include <unordered_map>
#include <lexer/token.h>
#include <string>

using std::string;

namespace bao {
    inline const std::unordered_map<TokenType, string> token_type_map = {
        {TokenType::Operator, "Operator"},
        {TokenType::Identifier, "Identifier"},
        {TokenType::Keyword, "Keyword"},
        {TokenType::Literal, "Literal"},
        {TokenType::String, "String"},
        {TokenType::Semicolon, "Semicolon"},
        {TokenType::Comma, "Comma"},
        {TokenType::LParen, "LParen"},
        {TokenType::RParen, "RParen"},
        {TokenType::LBracket, "LBracket"},
        {TokenType::RBracket, "RBracket"},
        {TokenType::LBrace, "LBrace"},
        {TokenType::RBrace, "RBrace"},
        {TokenType::Newline, "Newline"},
        {TokenType::EndOfFile, "EndOfFile"},
        {TokenType::Unknown, "Unknown"}
        };
}

#endif //MAPS_H
