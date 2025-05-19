//
// Created by đỗ quyên on 17/5/25.
//

#ifndef ANALYZER_H
#define ANALYZER_H
#include <bao/sema/symtabl.h>
#include <bao/parser/ast.h>

namespace bao {
    class Analyzer {
        sema::SymbolTable symbolTable;
        ast::Program program;
    public:
        explicit Analyzer(ast::Program&& program);
        ast::Program analyze_program();
    private:
        void analyze_function(const ast::FuncNode& func);
        void analyze_statement(sema::SymbolTable& parentTable, ast::StmtNode* stmt, Type* return_type);
        void analyze_expression(sema::SymbolTable& parentTable, ast::ExprNode* expr);
    };

}
#endif //ANALYZER_H
