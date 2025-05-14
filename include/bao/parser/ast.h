//
// Created by đỗ quyên on 14/5/25.
//

#ifndef AST_H
#define AST_H
#include <bao/parser/ast.h>
#include <bao/parser/types.h>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::pair;
using std::vector;

namespace bao {
    // --- AST Node interface ---
    class ASTNode {
    protected:
        string name;
        int line;
        int column;
    public:
        ASTNode(string name, const int line, const int column) : name(std::move(name)), line(line), column(column) {
        }

        virtual ~ASTNode() = default;

        /**
         * Get the line and column of the node
         * @return Node's line and column in the source code
         */
        pair<int, int> pos() const {
            return {line, column};
        }
        string get_name() const {
            return name;
        }
    };


    // --- Program's variables ---
    class VarNode final : public ASTNode {
        Type type;
    public:
        explicit VarNode(
            string name,
            const int line,
            const int column,
            const Type &type
        ):  ASTNode(std::move(name), line, column),
            type(type) {}
    };

    // --- Program's function ---
    class FuncNode final : public ASTNode {
        vector<VarNode> params;
        Type return_type;
    public:
        explicit FuncNode(
            string name,
            const int line, const int column,
            const vector<VarNode> &params,
            const Type &return_type
        ): ASTNode(std::move(name), line, column),
           params(params),
           return_type(return_type) {
        }
        [[nodiscard]] const vector<VarNode> &get_params() const {
            return params;
        }

        [[nodiscard]] const Type &get_return_type() const {
            return return_type;
        }
    };

    // --- Statement Base ---
    class StmtNode : public ASTNode {
    public:
        explicit StmtNode(string name, const int line, const int column): ASTNode(std::move(name), line, column) {
        }
    };

    // --- Expression Base ---
    class ExprNode : public ASTNode {
    public:
        explicit ExprNode(
            string name,
            const int line,
            const int column
        ):  ASTNode(std::move(name),
            line, column) {}
    };

    /*
    // --- Statements ---
    class VarDeclStmt final : public StmtNode {
        VarNode var;
        ExprNode val;
        public:
        explicit VarDeclStmt(
            const VarNode& var,
            const ExprNode &val,
            const int line, const int column
            ):
            StmtNode(line, column),
            var(var), val(val) {}
    };

    class RetStmt final : public StmtNode {
        ExprNode val;
    public:
        explicit RetStmt(
            const ExprNode &val,
            const int line, const int column
            ):
            StmtNode(line, column),
            val(val) {}
    };

    // --- Expressions ---
    class NumLitExpr final : public ExprNode {
        string value;
        Type type;
        public:
        explicit NumLitExpr(
            const string &value,
            const Type &type,
            const int line,
            const int column
            ):
            ExprNode(line, column),
            value(value), type(type) {}
    };
    */

    // --- Final parsed program ---
    struct Program {
        string name;
        string path;
        vector<FuncNode> funcs;
        explicit Program(
            string name, string path,
            const vector<FuncNode> &funcs): name(std::move(name)),
                                            path(std::move(path)),
                                            funcs(funcs) {
        }
    };
}
#endif //AST_H
