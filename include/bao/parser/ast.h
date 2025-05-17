//
// Created by đỗ quyên on 14/5/25.
//

#ifndef AST_H
#define AST_H
#include <bao/types.h>
#include <bao/utils.h>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::pair;
using std::vector;

namespace bao::ast {
    // --- AST Node interface ---
    class ASTNode {
    protected:
        string name;
        int line;
        int column;
    public:
        ASTNode(string name, const int line, const int column)
            : name(std::move(name)), line(line), column(column) {
        }

        virtual ~ASTNode() = default;

        /**
         * Get the line and column of the node
         * @return Node's line and column in the source code
         */
        [[nodiscard]] pair<int, int> pos() const {
            return {line, column};
        }

        /**
         * Get a node's name
         * @return Node's name
         */
        [[nodiscard]] string get_name() const {
            return name;
        }
    };


    /**
     * --- Statement base ---
     */

    class StmtNode : public ASTNode {
    public:
        StmtNode(): ASTNode("unknown", 0, 0) {}

        StmtNode(
            string name,
            const int line,
            const int column
        ):  ASTNode(std::move(name),
            line, column) {}

        ~StmtNode() override = default;
    };

    /**
     * --- Expression Base ---
     */

    class ExprNode : public ASTNode {
        std::unique_ptr<Type> type;
    public:
        ExprNode(): ASTNode("unknown", 0, 0), type(std::move(std::make_unique<UnknownType>())) {}
        ExprNode(
            string name,
            std::unique_ptr<Type>&& type,
            const int line,
            const int column
        ):  ASTNode(std::move(name),
            line, column), type(std::move(type)) {}

        ~ExprNode() override = default;

        [[nodiscard]] Type* get_type() const {
            return type.get();
        }

        void set_type(std::unique_ptr<Type>&& new_type) {
            type = std::move(new_type);
        }
    };

    // --- Program's variables ---
    class VarNode final : public ASTNode {
        std::unique_ptr<Type> type;
    public:
        VarNode(
            string name,
            const int line,
            const int column,
            std::unique_ptr<Type>&& type
        ):  ASTNode(std::move(name), line, column),
            type(std::move(type)) {}

        [[nodiscard]] Type* get_type() const {
            return type.get();
        }
    };

    // --- Program's function ---
    class FuncNode final : public ASTNode {
        vector<VarNode> params;
        vector<std::unique_ptr<StmtNode>> stmts;
        std::unique_ptr<Type> return_type;
    public:
        FuncNode(const FuncNode&) = delete;
        FuncNode& operator=(const FuncNode&) = delete;
        FuncNode(FuncNode&&) = default;
        FuncNode& operator=(FuncNode&&) = default;

        FuncNode(
            string name,
            vector<VarNode>&& params,
            vector<std::unique_ptr<StmtNode>>&& stmts,
            std::unique_ptr<Type>&& return_type,
            const int line, const int column
        ):  ASTNode(std::move(name), line, column),
            params(std::move(params)),
            stmts(std::move(stmts)),
            return_type(std::move(return_type)) {
        }
        [[nodiscard]] const vector<VarNode> &get_params() const {
            return params;
        }

        [[nodiscard]] Type* get_return_type() const {
            return return_type.get();
        }

        [[nodiscard]] const vector<std::unique_ptr<StmtNode>> &get_stmts() const {
            return stmts;
        }
    };

    // --- Statements ---

    class VarDeclStmt final : public StmtNode {
        VarNode var;
        std::unique_ptr<ExprNode> val;
        public:
        explicit VarDeclStmt(
            VarNode  var,
            std::unique_ptr<ExprNode>&& val,
            const int line, const int column
            ):
            StmtNode("vardeclstmt", line, column),
            var(std::move(var)), val(std::move(val)) {}

        [[nodiscard]] const VarNode &get_var() const {
            return var;
        }

        [[nodiscard]] const ExprNode *get_val() const {
            return val.get();
        }
    };

    class RetStmt final : public StmtNode {
        std::unique_ptr<ExprNode> val;
    public:
        explicit RetStmt(
            std::unique_ptr<ExprNode> val,
            const int line, const int column
            ):
            StmtNode("retstmt", line, column),
            val(std::move(val)) {}

        [[nodiscard]] ExprNode* get_val() const {
            return val.get();
        }
    };

    // --- Expressions ---

    class NumLitExpr final : public ExprNode {
        string value;
    public:
        explicit NumLitExpr(
            string value,
            std::unique_ptr<Type> &&type,
            const int line,
            const int column
            ):
            ExprNode("numlitexpr", std::move(type), line, column),
            value(std::move(value)) {}

        [[nodiscard]] string get_value() const {
            return value;
        }
    };

    // --- Final parsed program ---
    struct Program {
        string name;
        string path;
        vector<FuncNode> funcs;
        explicit Program(
            string name, string path,
            vector<FuncNode>&& funcs)
        :   name(std::move(name)),
            path(std::move(path)),
            funcs(std::move(funcs)) {}
    };
}
#endif //AST_H
