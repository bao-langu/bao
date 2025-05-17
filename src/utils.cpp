//
// Created by doqin on 13/05/2025.
//
#include <bao/utils.h>
#include <iostream>
#include <bao/lexer/maps.h>
#include <cstring>
#include <bao/parser/ast.h>

using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;

bool bao::utils::arg_contains(const int argc, char *argv[], const char *target) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], target) == 0) {
            return true;
        }
    }
    return false;
}

void bao::utils::print_usage() {
    cout << "Cú pháp: baoc [--test] [--huong-dan]" << endl;
    cout << "--test: Chạy tests" << endl;
    cout << "--huong-dan: Hiện thông tin về cách sử dụng" << endl;
}

void bao::utils::print_token(const Token &token) {
    cout << "Token: " << token.value << ", Loại: " << token_type_map.at(token.type) << ", Dòng: " << token.line << ", Cột: " << token.column << endl;
}

void bao::utils::print_program(const ast::Program &program) {
    cout << "Nội dung chương trình:" << endl;
    cout << "\tTên: " << program.name << endl;
    cout << "\tĐường dẫn: " << program.path << endl;
    cout << "\tSố hàm: " << program.funcs.size() << endl;
    for (const auto &func : program.funcs) {
        print_function(func, "\t");
    }
}

void bao::utils::print_function(const ast::FuncNode& func, const string &padding) {
    auto [line, column] = func.pos();
    const auto message = std::format(
        "Hàm: {} -> {} (Dòng {}, Cột {})",
        func.get_name(), func.get_return_type()->get_name(),
        line, column);
    cout << pad_lines(message, padding) << endl;
    for (const auto& stmt_ptr : func.get_stmts()) {
        print_statement(stmt_ptr.get(), padding + "\t");
    }
}

void bao::utils::print_statement(ast::StmtNode* stmt, const string &padding) {
    if (!stmt) {
        cout << padding + "\tCâu lệnh không xác định" << endl;
        return;
    }
    auto [line, column] = stmt->pos();
    const auto message = std::format("{} (Dòng {}, Cột {}):", stmt->get_name(), line, column);
    cout << pad_lines(message, padding) << endl;
    if (const auto ret_stmt = dynamic_cast<ast::RetStmt*>(stmt)) {
        print_expression(ret_stmt->get_val(), padding + "\t");
    } else {
        cout << padding + "\tBiểu thức không xác định" << endl;
    }
}

void bao::utils::print_expression(ast::ExprNode* expr, const string &padding) {
    auto [line, column] = expr->pos();
    if (const auto num_expr = dynamic_cast<ast::NumLitExpr*>(expr)) {
        const auto message = std::format(
            "Biểu thức số: {} ({}) (Dòng {}, Cột {})",
            num_expr->get_value(), num_expr->get_type()->get_name(), line, column);
        cout << pad_lines(message, padding) << endl;
    } else {
        cout << padding + "\tBiểu thức không xác định" << endl;
    }
}

void bao::utils::match(
    const string& val,
    const std::unordered_map<string, std::function<void()>>& pattern,
    const std::function<void()>& default_case) {
    if (pattern.contains(val)) {
        pattern.at(val)();
    } else {
       default_case();
    }
}

string bao::utils::pad_lines(const string &input, const string &padding) {
    istringstream iss(input);
    ostringstream oss;
    string line;
    bool first = true;
    while (std::getline(iss, line)) {
        if (!first) oss << "\n";
        oss << padding << line;
        first = false;
    }

    return oss.str();
}

bool bao::utils::is_literal(ast::ExprNode* expr) {
    // Null check
    if (!expr) {
        return false;
    }
    if (dynamic_cast<ast::NumLitExpr*>(expr)) {
        return true;
    }
    return false;
}

void bao::utils::cast_literal(ast::NumLitExpr *expr, const Type *type) {
    expr->set_type(std::move(std::make_unique<Type>(*type)));
}

bool bao::utils::can_cast_literal(const ast::NumLitExpr *expr, const Type *type) {
    if (!expr->get_type()) {
        return false; // FIXME: Handle this case
    }
    const auto prim = dynamic_cast<PrimitiveType*>(expr->get_type());
    if (!prim) {
        return false; // FIXME: Handle this case
    }
    switch (prim->get_type()) {
        case Primitive::N64:
        case Primitive::N32:
            if (type->get_name() == "N32") {
                return true;
            }
            return false;
        case Primitive::Z64:
        case Primitive::Z32:
            if (type->get_name() == "Z32") {
                return true;
            }
            return false;
        case Primitive::R64:
        case Primitive::R32:
            if (type->get_name() == "R32") {
                return true;
            }
            return false;
        default:
            return false;
    }
}