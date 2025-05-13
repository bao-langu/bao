//
// Created by doqin on 13/05/2025.
//

#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <vector>
#include <unicode/unistr.h>
#include <unicode/schriter.h>
#include <lexer/token.h>

using std::string;
using std::vector;
using std::exception;
using icu::UnicodeString;
using icu::StringCharacterIterator;

namespace bao {
    class Lexer {
        vector<exception> errors;

        UnicodeString source;
        StringCharacterIterator it;
        int code_point_index;
        vector<Token> tokens;
        int current_line;
        int current_column;
    public:
        explicit Lexer(const string& source);
        void tokenize();
        [[nodiscard]] const vector<Token>& get_tokens() const;
    private:
        [[nodiscard]] string current_utf8() const;
        [[nodiscard]] UChar32 current_code_point() const;
        [[nodiscard]] string peek() const;
        void next();
        void skip_whitespace();
        void seek(int code_point_index);

        Token handle_identifier();
        Token handle_number();
        Token handle_symbols();
    };
}
#endif //LEXER_H
