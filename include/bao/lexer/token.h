//
// Created by doqin on 13/05/2025.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <string>
using std::string;
namespace bao {
    enum class TokenType {
        Operator,
        Identifier,
        Keyword,
        Literal,
        String,
        Semicolon,
        Comma,
        LParen,
        RParen,
        LBracket,
        RBracket,
        LBrace,
        RBrace,
        Newline,
        EndOfFile,
        Unknown
    };

    struct Token {
        TokenType type;
        string value;
        int line;
        int column;
    };
}
#endif //TOKEN_H
