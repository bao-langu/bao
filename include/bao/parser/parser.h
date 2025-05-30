//
// Created by đỗ quyên on 14/5/25.
//

#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>
#include <bao/lexer/token.h>
#include <bao/parser/ast.h>
#include <bao/utils.h>

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

        ast::Program parse_program();
    private:
        // Highest priority
        ast::FuncNode parse_function();
        ast::FuncNode parse_procedure();

        // Parsing helpers
        Token current();
        void next();
        Token peek();
        void skip_newlines();
        int current_precedence();

        ast::VarNode parse_var(bool isConst);
        std::unique_ptr<Type> parse_type();

        // Statements
        std::unique_ptr<ast::StmtNode> parse_statement();
        std::unique_ptr<ast::RetStmt> parse_retstmt();
        std::unique_ptr<ast::VarDeclStmt> parse_vardeclstmt(bool isConst);
        std::unique_ptr<ast::VarAssignStmt> parse_varassignstmt();

        // Expressions
        std::unique_ptr<ast::ExprNode> parse_expression(int minPrec);
        std::unique_ptr<ast::ExprNode> parse_primary();
    };
}
#endif //PARSER_H
