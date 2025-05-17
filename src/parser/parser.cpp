//
// Created by đỗ quyên on 14/5/25.
//
#include <bao/parser/parser.h>
#include <bao/utils.h>
#include <bao/types.h>
#include <bao/parser/ast.h>
#include <filesystem>

namespace fs = std::filesystem;
using std::out_of_range;
using std::runtime_error;

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
            this->next();
            exceptions.push_back(std::current_exception());
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

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi kiểu trả về tại vị trí này", this->current().line, this->current().column);
    }
    // TODO: Implement types other than primitive
    if (!primitive_map.contains(this->current().value)) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi kiểu nguyên thuỷ", this->current().line, this->current().column);
    }
    const string type = this->current().value;
    this->next(); // Consumes type

    if (this->current().type != TokenType::Newline) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi xuống dòng tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes '\n'

    vector<std::unique_ptr<ast::StmtNode>> stmts;
    vector<exception_ptr> exceptions;
    while (this->current().value != "kết thúc") {
        // Ignore newlines
        this->skip_newlines();
        if (this->current().value == "kết thúc") {
            break;
        }
        try {
            stmts.push_back(std::move(this->parse_statement()));
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
    }
    if (!exceptions.empty()) {
        exceptions.insert(exceptions.begin(), utils::make_exception_ptr("Lỗi cú pháp trong câu lệnh:"));
        throw utils::ErrorList(exceptions);
    }

    if (this->current().value != "kết thúc") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi từ khoá 'kết thúc' tại vị trí này", this->current().line, this->current().column);
    }
    this->next(); // Consumes 'kết thúc'

    return {std::move(function_name), std::move(params), std::move(stmts), std::move(std::make_unique<PrimitiveType>(type)), line, column};
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
    while (this->current().value != "kết thúc") {
        // Ignore newlines
        this->skip_newlines();
        if (this->current().value == "kết thúc") {
            break;
        }
        try {
            stmts.push_back(std::move(this->parse_statement()));
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
    }
    if (!exceptions.empty()) {
        exceptions.insert(exceptions.begin(), utils::make_exception_ptr("Lỗi cú pháp trong câu lệnh:"));
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
    while (this->current().type == TokenType::Newline) {
        this->next();
    }
}

std::unique_ptr<bao::ast::StmtNode> bao::Parser::parse_statement() {
    std::unique_ptr<ast::StmtNode> stmt;
    try {
        utils::match(this->current().value,{
            {
                "trả về", [this, &stmt]() {
                    stmt = this->parse_retstmt();
                }
            }
        }, {
            [this, &stmt]() {
                    stmt = nullptr;
                    throw utils::CompilerError::new_error(
                        this->filename, this->directory, "Câu lệnh không xác định", this->current().line, this->current().column);
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
    if (this->current().type == TokenType::Newline || this->current().type == TokenType::Comma) {
        this->next(); // Consumes '\n' or ';'
        return std::make_unique<ast::RetStmt>(nullptr, line, column);
    }
    try {
        std::unique_ptr<ast::ExprNode> expr = this->parse_expression();
        return std::make_unique<ast::RetStmt>(std::move(expr), line, column);
    } catch ([[maybe_unused]] exception& e) {
        throw;
    }
}

std::unique_ptr<bao::ast::ExprNode> bao::Parser::parse_expression() {
    const Token current = this->current();
    const string val = current.value;
    auto line = this->current().line;
    auto column = this->current().column;
    this->next();
    switch (current.type) {
        case TokenType::Literal:

            if (val.contains(".")) {
                return std::make_unique<ast::NumLitExpr>(
                    val, std::move(std::make_unique<PrimitiveType>("R64")),
                    line, column);
            }
            return std::make_unique<ast::NumLitExpr>(
                val, std::move(std::make_unique<PrimitiveType>("Z64")),
                line, column);
        default:
            throw utils::CompilerError::new_error(
                this->filename, this->directory,
                "Biểu thức không xác định",
                this->current().line, this->current().column);
    }
}
