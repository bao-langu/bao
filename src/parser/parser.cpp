//
// Created by đỗ quyên on 14/5/25.
//
#include <bao/parser/parser.h>
#include <bao/utils.h>
#include <bao/parser/types.h>
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

bao::Program bao::Parser::parse_program() {
    vector<FuncNode> functions;
    vector<exception> exceptions;
    while (this->current().type != TokenType::EndOfFile) {
        try {
            // Skip newlines
            while (this->current().type == TokenType::Newline) {
                this->next();
            }
            switch (this->current().type) {
                case TokenType::Keyword:
                    utils::match(this->current().value, {
                                     // Case "thủ tục" or "hàm"
                                     {
                                         "hàm", [&functions, this]() {
                                             functions.push_back(this->parse_function());
                                         }
                                     },
                                     {
                                         "thủ tục", [&functions, this]() {
                                             functions.push_back(this->parse_procedure());
                                         }
                                     }
                                 },
                                 // Default case
                                 [this]() {
                                     const int line = this->current().line;
                                     const int column = this->current().column;
                                     this->next();
                                     throw utils::CompilerError::new_error(
                                         this->filename, this->directory, "Ký hiệu không xác định", line, column);
                                 });
                    break;
                default:
                    const int line = this->current().line;
                    const int column = this->current().column;
                    this->next();
                    throw utils::CompilerError::new_error(
                        this->filename, this->directory, "Ký hiệu không xác định", line, column);
            }
        } catch (exception &e) {
            this->next();
            exceptions.push_back(e);
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }
    return Program(
        this->filename,
        this->directory,
        functions);
}

bao::FuncNode bao::Parser::parse_function() {
    const int line = this->current().line;
    const int column = this->current().column;
    this->next(); // Consumes "hàm"

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi tên hàm ở vị trí này", line, column);
    }
    const string function_name = this->current().value;
    this->next(); // Consumes identifier

    if (this->current().type != TokenType::LParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '(' ở vị trí này", line, column);
    }
    this->next(); // Consumes '('

    vector<VarNode> params;
    // TODO: Implement parameters parsing

    if (this->current().type != TokenType::RParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi ')' ở vị trí này", line, column);
    }
    this->next(); // Consumes ')'

    if (this->current().value != "->") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '->' tại vị trí này", line, column);
    }
    this->next(); // Consumes '->'

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi kiểu trả về tại vị trí này", line, column);
    }
    const string type = this->current().value;
    this->next(); // Consumes type

    if (this->current().type != TokenType::Newline) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi xuống dòng tại vị trí này", line, column);
    }
    this->next(); // Consumes '\n'

    vector<StmtNode> stmts;
    // TODO: Implement statement parsing

    if (this->current().value != "kết thúc") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi từ khoá 'kết thúc' tại vị trí này", line, column);
    }
    this->next(); // Consumes 'kết thúc'

    return FuncNode(function_name, line, column, params, PrimitiveType(type));
}

bao::FuncNode bao::Parser::parse_procedure() {
    const int line = this->current().line;
    const int column = this->current().column;
    this->next(); // Consumes "thủ tục"

    if (this->current().type != TokenType::Identifier) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi tên thủ tục ở vị trí này", line, column);
    }
    const string function_name = this->current().value;
    this->next(); // Consumes identifier
    if (this->current().type != TokenType::LParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi '(' ở vị trí này", line, column);
    }
    this->next(); // Consumes '('

    vector<VarNode> params;
    // TODO: Implement parameters parsing

    if (this->current().type != TokenType::RParen) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi ')' ở vị trí này", line, column);
    }
    this->next(); // Consumes ')'

    if (this->current().type != TokenType::Newline) {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi xuống dòng tại vị trí này", line, column);
    }
    this->next(); // Consumes '\n'

    vector<StmtNode> stmts;
    // TODO: Implement statement parsing

    if (this->current().value != "kết thúc") {
        throw utils::CompilerError::new_error(
            this->filename, this->directory, "Mong đợi từ khoá 'kết thúc' tại vị trí này", line, column);
    }
    this->next(); // Consumes 'kết thúc'

    return FuncNode(function_name, line, column, params, PrimitiveType("rỗng"));
}

bao::Token bao::Parser::current() {
    return this->tokens[this->it];
}

void bao::Parser::next() {
    this->it++;
    if (this->it == this->tokens.size()) {
        throw out_of_range("Lỗi nội bộ: không còn token");
    }
}

bao::Token bao::Parser::peek() {
    if (this->tokens[this->it].type == TokenType::EndOfFile) {
        throw out_of_range("Lỗi nội bộ: không còn token để hé");
    }
    return this->tokens[this->it + 1];
}
