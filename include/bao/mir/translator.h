//
// Created by đỗ quyên on 18/5/25.
//

#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <bao/parser/ast.h>
#include <bao/mir/mir.h>

namespace bao::mir {
    class Translator {
        Module module;
        ast::Module program;
    public:
        explicit Translator(ast::Module&& program);
        Module translate();
    private:
        Function translate_function(const ast::FuncNode& func);
        void translate_statement(Function& func, ast::StmtNode* stmt);
        Value translate_expression(Function& func, ast::StmtNode* stmt, ast::ExprNode* expr);
    };
}
#endif //TRANSLATOR_H
