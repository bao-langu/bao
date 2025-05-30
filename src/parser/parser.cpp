//
// Created by đỗ quyên on 14/5/25.
//
#include "bao/lexer/token.h"
#include <bao/parser/parser.h>
#include <bao/utils.h>
#include <bao/types.h>
#include <bao/parser/ast.h>
#include <memory>
#include <unordered_map>

using std::out_of_range;

std::unordered_map<std::string, int> precedences{
    {"hoặc",    3},
    {"và",      6},
    {"=",       12},
    {"!=",      12},
    {"<=",      25},
    {"<",       25},
    {">=",      25},
    {">",       25},
    {"+",       50},
    {"-",       50},
    {"*",       100},
    {"/",       100}
};

bao::Parser :: Parser(const string &filename, const string &directory, const vector<Token> &tokens) : tokens(tokens) {
    this->filename = filename;
    this->directory = directory;
    this->it = 0;
}

auto
bao::Parser :: parse_program() -> bao::ast::Program {
    vector<ast::FuncNode> functions;
    vector<exception_ptr> exceptions;
    while (this->current().type != TokenType::EndOfFile) {
        try {
            // Skip newlines
            this->skip_newlines();
            if (this->current().type == TokenType::EndOfFile) {
                break;
            }
            switch (this->current().type) {
                case TokenType::Keyword:
                    utils::match(this->current().value, {
                                     // Case "thủ tục" or "hàm"
                                     {
                                         "hàm", [&functions, this]() {
                                             functions.emplace_back(this->parse_function());
                                         }
                                     },
                                     {
                                         "thủ tục", [&functions, this]() {
                                             functions.emplace_back(this->parse_procedure());
                                         }
                                     }
                                 },
                                 // Default case
                                 [this]() {
                                     const int line = this->current().line;
                                     const int column = this->current().column;
                                     throw utils::CompilerError::new_error(
                                         this->filename, this->directory, "Ký hiệu không xác định", line, column);
                                 });
                    break;
                default:
                    const int line = this->current().line;
                    const int column = this->current().column;
                    throw utils::CompilerError::new_error(
                        this->filename, this->directory, "Ký hiệu không xác định", line, column);
            }
        } catch (...) {
            exceptions.push_back(std::current_exception());
            if (this->current().type == TokenType::EndOfFile) {
                break;
            }
            this->next();
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }
    return ast::Program(
        this->filename,
        this->directory,
        std::move(functions));
}

auto
bao::Parser :: parse_function() -> bao::ast::FuncNode {
    const int line = this->current().line;
    const int column = this->current().column;
    this->next(); // Consumes "hàm"

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi tên hàm ở vị trí này", this->current().line, this->current().column);
    }
    const string function_name = this->current().value;
    this->next(); // Consumes identifier

    if (this->current().type != TokenType::LParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '(' ở vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '('

    vector<ast::VarNode> params;
    // TODO: Implement parameters parsing

    if (this->current().type != TokenType::RParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi ')' ở vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes ')'

    if (this->current().value != "->") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '->' tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '->'

    std::unique_ptr<Type> type = nullptr;
    try {
        type = parse_type();
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }

    if (this->current().type != TokenType::Newline) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi xuống dòng tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '\n'

    vector<std::unique_ptr<ast::StmtNode>> stmts;
    vector<exception_ptr> exceptions;
    while (this->current().value != "kết thúc" || this->current().type != TokenType::EndOfFile) {
        // Ignore newlines
        this->skip_newlines();
        if (this->current().value == "kết thúc" || this->current().type == TokenType::EndOfFile) {
            break;
        }
        try {
            stmts.push_back(std::move(this->parse_statement()));
        } catch (...) {
            exceptions.push_back(std::current_exception());
            if (this->current().type == TokenType::EndOfFile) {
                break;
            }
            this->next();
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }

    if (this->current().value != "kết thúc") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi từ khoá 'kết thúc' tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes 'kết thúc'

    return {std::move(function_name), std::move(params), std::move(stmts), std::move(type), line, column};
}

auto
bao::Parser :: parse_procedure() -> bao::ast::FuncNode {
    const int line = this->current().line;
    const int column = this->current().column;
    this->next(); // Consumes "thủ tục"

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi tên thủ tục ở vị trí này", this->current().line, this->current().column);
    }
    const string function_name = this->current().value;
    this->next(); // Consumes identifier
    if (this->current().type != TokenType::LParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '(' ở vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '('

    vector<ast::VarNode> params;
    // TODO: Implement parameters parsing

    if (this->current().type != TokenType::RParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi ')' ở vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes ')'

    if (this->current().type != TokenType::Newline) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi xuống dòng tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '\n'

    vector<std::unique_ptr<ast::StmtNode>> stmts;
    vector<exception_ptr> exceptions;
    while (this->current().value != "kết thúc" || this->current().type != TokenType::EndOfFile) {
        // Ignore newlines
        this->skip_newlines();
        if (this->current().value == "kết thúc" || this->current().type == TokenType::EndOfFile) {
            break;
        }
        try {
            stmts.push_back(std::move(this->parse_statement()));
        } catch (...) {
            exceptions.push_back(std::current_exception());
            if (this->current().type == TokenType::EndOfFile) {
                break;
            }
            this->next();
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }

    if (this->current().value != "kết thúc") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory,
            "Mong đợi từ khoá 'kết thúc' tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes 'kết thúc'

    return {std::move(function_name), std::move(params), std::move(stmts), std::move(std::make_unique<PrimitiveType>("rỗng")), line, column};
}

auto
bao::Parser :: current() -> bao::Token {
    return this->tokens[this->it];
}

// --- Helpers ---

void
bao::Parser :: next() {
    this->it++;
    if (this->it == this->tokens.size()) {
        throw out_of_range("Lỗi nội bộ: Không còn token");
    }
}

auto
bao::Parser :: peek() -> bao::Token {
    if (this->tokens[this->it].type == TokenType::EndOfFile) {
        throw out_of_range("Lỗi nội bộ: Không còn token để hé lộ");
    }
    return this->tokens[this->it + 1];
}

void
bao::Parser :: skip_newlines() {
    while (this->current().type == TokenType::Newline && this->current().type != TokenType::EndOfFile) {
        this->next();
    }
}

auto
bao::Parser :: current_precedence() -> int {
    bao::Token current = this->current();
    if (current.type == bao::TokenType::Operator || current.type == bao::TokenType::Keyword) {
        if (precedences.contains(current.value)) {
            return precedences.at(current.value);
        }
    }
    return -1;
}


// --- Statements ---

auto
bao::Parser :: parse_statement() -> std::unique_ptr<bao::ast::StmtNode> {
    std::unique_ptr<ast::StmtNode> stmt;
    try {
        utils::match(this->current().value,{
            {
                "trả về", [&]() {
                    try {
                        stmt = this->parse_retstmt();
                    } catch ([[maybe_unused]] exception& e) {
                        throw utils::CompilerError::new_error(
                            this->filename, this->directory, "Lỗi cú pháp trong câu lệnh trả về", this->current().line, this->current().column
                        );
                    }
                }
            },
            {
                "biến", [&]() {
                    try {
                        stmt = this->parse_vardeclstmt(false);
                    } catch (...) {
                        throw;
                    }
                }
            }, {
                "hằng", [&]() {
                    try {
                        stmt = this->parse_vardeclstmt(true);
                    } catch (...) {
                        throw;
                    }
                }
            }
        }, {
            [&]() {
                    if (this->current().type == TokenType::Identifier) {
                        try {
                            stmt = this->parse_varassignstmt();
                        } catch (...) {
                            throw;
                        }
                    } else {
                        throw utils::CompilerError::new_error(
                            this->filename, this->directory, "Câu lệnh không xác định", this->current().line, this->current().column
                        );
                    }
                }
            }
        );
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
    return stmt;
}

auto
bao::Parser :: parse_retstmt() -> std::unique_ptr<bao::ast::RetStmt> {
    auto line = this->current().line;
    auto column = this->current().column;
    this->next(); // Consumes 'trả về'
    if (this->current().type == TokenType::Newline || this->current().type == TokenType::Semicolon) {
        this->next(); // Consumes '\n' or ';'
        return std::make_unique<ast::RetStmt>(nullptr, line, column);
    }
    try {
        std::unique_ptr<ast::ExprNode> expr = this->parse_expression(0);
        return std::make_unique<ast::RetStmt>(std::move(expr), line, column);
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

auto
bao::Parser :: parse_vardeclstmt(
    bool isConst
) -> std::unique_ptr<bao::ast::VarDeclStmt> {
    auto line = this->current().line;
    auto column = this->current().column;
    this->next(); // Consumes 'hằng' or 'biến'
    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, 
            "Mong đợi tên biến tại đây", 
            this->current().line, this->current().column
        );
    }
    try {
        auto varNode = this->parse_var(isConst);
        std::unique_ptr<bao::ast::ExprNode> val = nullptr;
        if (current().value == ":=") {
            this->next(); // Consumes ':=' token
            val = this->parse_expression(0);
        } else if (isConst) {
            auto temp_line = this->current().line;
            auto temp_column = this->current().column;
            throw utils::CompilerError::new_error(
                this->filename, this->directory,
                "Hằng số phải có giá trị khởi tạo", 
                temp_line, temp_column);
        }
        return std::make_unique<ast::VarDeclStmt>(
            varNode, 
            std::move(val), 
            line, column
        );
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

auto
bao::Parser :: parse_varassignstmt() -> std::unique_ptr<bao::ast::VarAssignStmt> {
    auto line = this->current().line;
    auto column = this->current().column;
    std::string var_name = this->current().value;
    this->next();

    if (this->current().value != ":=") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, 
            "Mong đợi ':=' tại đây",
            this->current().line, this->current().column
        );
    }
    this->next();

    std::unique_ptr<ast::ExprNode> val;
    try {
        val = this->parse_expression(0);
    } catch (...) {
        throw;
    }
    auto var = ast::VarNode(
            var_name,
            std::make_unique<UnknownType>(),
            false,
            line, column
        );
    return std::make_unique<ast::VarAssignStmt>(
        var,
        std::move(val),
        line, column
    );
}

auto
bao::Parser :: parse_var(
    bool isConst
) -> bao::ast::VarNode  {
    auto var_line = this->current().line;
    auto var_column = this->current().column;
    std::string var_name = this->current().value;
    this->next();
    if (this->current().value != "E") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi ký hiệu 'E' tại đây", this->current().line, this->current().column
        );
    }
    this->next();
    try {
        auto type = this->parse_type();
        return ast::VarNode(var_name, std::move(type), isConst, var_line, var_column);
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

auto
bao::Parser :: parse_type() -> std::unique_ptr<bao::Type> {
    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
        this->filename, this->directory, "Kiểu dữ liệu không xác định", this->current().line, this->current().column
        );
    }
    
    // TODO: Implement types other than primitive
    if (!primitive_map.contains(this->current().value)) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi kiểu nguyên thuỷ", this->current().line, this->current().column);
    }
    std::string type_name = this->current().value;
    this->next();
    return std::make_unique<PrimitiveType>(type_name);
}

auto
bao::Parser :: parse_expression(
    int minPrec
) -> std::unique_ptr<bao::ast::ExprNode> {
    try {
        // Expression's position
        auto line = this->current().line;
        auto column = this->current().column;

        // Parse the left-hand side
        auto left = this->parse_primary();

        // Funny loop
        while(true) {
            int prec = this->current_precedence();
            if (prec < minPrec) {
                break;
            }

            const auto current = this->current();
            auto op = current.value; // Binary operator
            this->next(); // Consumes token

            // Parse right-hand side expression with higher precedence for right-associativity
            auto right = this->parse_expression(prec + 1);

            left = std::make_unique<ast::BinExpr>(std::move(left), std::move(op), std::move(right), line, column);
        }

        return std::move(left);
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

auto
bao::Parser :: parse_primary() -> std::unique_ptr<bao::ast::ExprNode>  {
    const auto current = this->current();
    const auto val = current.value;
    auto line = this->current().line;
    auto column = this->current().column;
    this->next(); // Consumes token
    switch (current.type) {
        case TokenType::Identifier:
            return std::make_unique<ast::VarExpr>(
                val, std::make_unique<UnknownType>(),
                line, column
            );
        case TokenType::Literal:
            if (val.contains(".")) {
                return std::make_unique<ast::NumLitExpr>(
                    val, std::make_unique<PrimitiveType>("R64"),
                    line, column);
            }
            return std::make_unique<ast::NumLitExpr>(
                val, std::make_unique<PrimitiveType>("Z64"),
                line, column);

        case TokenType::LParen: {
            auto expr = this->parse_expression(0);
            if (this->current().type != TokenType::RParen) {
                throw utils::CompilerError::new_error(
                    this->filename, this->directory,
                    "Mong đợi ')' tại đây", this->current().line, this->current().column);
            }
            this->next();
            return std::move(expr);
        }
        default:
            throw utils::CompilerError::new_error(
                this->filename, this->directory,
                "Biểu thức không xác định",
                this->current().line, this->current().column);
    }
}