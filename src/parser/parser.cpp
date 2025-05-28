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

bao::Parser::Parser(const string &filename, const string &directory, const vector<Token> &tokens) : tokens(tokens) {
    this->filename = filename;
    this->directory = directory;
    this->it = 0;
}

bao::ast::Program bao::Parser::parse_program() {
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

bao::ast::FuncNode bao::Parser::parse_function() {
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

bao::ast::FuncNode bao::Parser::parse_procedure() {
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

bao::Token bao::Parser::current() {
    return this->tokens[this->it];
}

// --- Helpers ---

void bao::Parser::next() {
    this->it++;
    if (this->it == this->tokens.size()) {
        throw out_of_range("Lỗi nội bộ: Không còn token");
    }
}

bao::Token bao::Parser::peek() {
    if (this->tokens[this->it].type == TokenType::EndOfFile) {
        throw out_of_range("Lỗi nội bộ: Không còn token để hé lộ");
    }
    return this->tokens[this->it + 1];
}

void bao::Parser::skip_newlines() {
    while (this->current().type == TokenType::Newline && this->current().type != TokenType::EndOfFile) {
        this->next();
    }
}

int bao::Parser::current_precedence() {
    bao::Token current = this->current();
    if (current.type == bao::TokenType::Operator || current.type == bao::TokenType::Keyword) {
        if (precedences.contains(current.value)) {
            return precedences.at(current.value);
        }
    }
    return -1;
}


// --- Statements ---

std::unique_ptr<bao::ast::StmtNode> bao::Parser::parse_statement() {
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
            [this, &stmt]() {
                    stmt = nullptr;
                    throw utils::CompilerError::new_error(
                        this->filename, this->directory, "Câu lệnh không xác định", this->current().line, this->current().column
                    );
                }
            }
        );
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
    return stmt;
}

std::unique_ptr<bao::ast::RetStmt> bao::Parser::parse_retstmt() {
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

std::unique_ptr<bao::ast::VarDeclStmt> bao::Parser::parse_vardeclstmt(bool isConst) {
    auto line = this->current().line;
    auto column = this->current().column;
    this->next(); // Consumes 'hằng' or 'biến'
    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi tên biến tại đây", this->current().line, this->current().column
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
        return std::make_unique<ast::VarDeclStmt>(varNode, std::move(val), line, column);
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

bao::ast::VarNode bao::Parser::parse_var(bool isConst) {
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

std::unique_ptr<bao::Type> bao::Parser::parse_type() {
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

std::unique_ptr<bao::ast::ExprNode> bao::Parser::parse_expression(int minPrec) {
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

std::unique_ptr<bao::ast::ExprNode> bao::Parser::parse_primary() {
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