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

        // Statements
        void analyze_statement(sema::SymbolTable& parentTable, ast::StmtNode* stmt, Type* return_type);
        void analyze_retstmt(sema::SymbolTable& parentTable, ast::RetStmt* stmt, Type* return_type);
        void analyze_vardeclstmt(sema::SymbolTable& parentTable, ast::VarDeclStmt* stmt);
        void analyze_varassignstmt(sema::SymbolTable& parentTable, ast::VarAssignStmt* stmt);

        // Expressions
        void analyze_expression(sema::SymbolTable& parentTable, ast::ExprNode* expr);

        // Helpers
        void analyze_type(ast::ExprNode* val, Type* type);
    };
}
#endif //ANALYZER_H
