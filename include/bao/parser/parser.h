//
// Created by đỗ quyên on 14/5/25.
//

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <bao/lexer/token.h>
#include <bao/parser/ast.h>

using std::vector;

namespace bao {
    class Parser {
        string filename;
        string directory;
        vector<Token> tokens;
        int it;

    public:
        /**
         *
         * @param filename Source file's name
         * @param directory Path to source file
         * @param tokens Provide the tokens to parse
         */
        explicit Parser(
            const string &filename,
            const string &directory,
            const vector<Token> &tokens
        );

        Program parse_program();

    private:
        FuncNode parse_function();

        FuncNode parse_procedure();

        Token current();

        void next();

        Token peek();

        void skip_newlines();
    };
}
#endif //PARSER_H
