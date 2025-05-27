//
// Created by đỗ quyên on 17/5/25.
//

#include "bao/parser/ast.h"
#include "bao/utils.h"
#include <bao/sema/analyzer.h>
#include <exception>

bao::Analyzer::Analyzer(ast::Program &&program) : program(std::move(program)) {
    this->symbolTable = sema::SymbolTable();
}

bao::ast::Program bao::Analyzer::analyze_program() {
    // Forward declaration of functions
    for (auto& func : program.funcs) {
        // Insert the function into the symbol table
        sema::SymbolInfo info{};
        info.type = sema::SymbolType::Function;
        info.datatype = func.get_return_type();
        info.scopeLevel = 0; // Global scope
        this->symbolTable.insert(func.get_name(), info);
    }

    // Iterate through all functions in the program
    std::vector<exception_ptr> exceptions;
    for (auto& func : program.funcs) {
        try {
            analyze_function(func);
        } catch (...) {
            exceptions.emplace_back(std::current_exception());
        }
    }
    if (!exceptions.empty()) {
        const std::string errorMessage = std::format("\033[34m@{}/{}:\033[0m\n{}",
                                                program.path,
                                                program.name,
                                                utils::pad_lines(utils::ErrorList(exceptions).what(), "   "));
        throw std::runtime_error(errorMessage);
    }
    return std::move(program);
}

void bao::Analyzer::analyze_function(const ast::FuncNode &func) {
    sema::SymbolTable localTable{};
    // Insert param into the local symbol table
    for (auto& param : func.get_params()) {
        // Insert the parameter into the local symbol table
        sema::SymbolInfo info{};
        info.type = sema::SymbolType::Variable;
        info.datatype = param.get_type();
        info.scopeLevel = 1; // Local scope
        localTable.insert(param.get_name(), info);
    }

    std::vector<exception_ptr> exceptions;
    for (auto& stmt : func.get_stmts()) {
        try {
            analyze_statement(localTable, stmt.get(), func.get_return_type());
        } catch (...) {
            exceptions.emplace_back(std::current_exception());
        }
    }

    if (!exceptions.empty()) {
        auto [line, column] = func.pos();
        const std::string errorMessage = std::format("\033[34m@{}\033[0m -> {} (Dòng {}, Cột {}):\n{}",
                                                func.get_name(),
                                                func.get_return_type()->get_name(),
                                                line,
                                                column,
                                                utils::pad_lines(utils::ErrorList(exceptions).what(), " | "));
        throw std::runtime_error(errorMessage);
    }
}

void bao::Analyzer::analyze_statement(sema::SymbolTable &parentTable, ast::StmtNode* stmt, Type* return_type) {
    if (const auto* retStmt = dynamic_cast<ast::RetStmt*>(stmt)) {
        // Analyze return statement
        if (retStmt->get_val()) {
            try {
                analyze_expression(parentTable, retStmt->get_val());
            } catch (...) {
                throw;
            }
        }
        // Check if the return type matches the function's return type
        if (return_type->get_name() != "rỗng") {
            // Check if the return type matches the function's return type
            if (retStmt->get_val()) {

                // TODO: For now check only the type name, check it better later
                if (return_type->get_name() == retStmt->get_val()->get_type()->get_name()) {
                    return;
                }

                auto val = retStmt->get_val();
                // If the return type does not match, check if it can be literal cast
                if (!utils::is_literal(val)) {
                    auto [line, column] = val->pos();
                    throw utils::CompilerError::new_error(program.name, program.path, "Kiểu trả về khác kiểu trả về của hàm", line, column);
                }
                if (!utils::can_cast_literal(retStmt->get_val(), return_type)) {
                    auto [line, column] = val->pos();
                    throw utils::CompilerError::new_error(program.name, program.path, "Kiểu trả về khác kiểu trả về của hàm", line, column);
                }

                // Cast the expression to the function's return type
                try {
                    utils::cast_literal(val, return_type);
                } catch ([[maybe_unused]] std::exception& e) {
                    throw;
                }
            }
        } else if (retStmt->get_val()) {
            // If the function return type is "rỗng", but a value is returned
            auto [line, column] = retStmt->pos();
            throw utils::CompilerError::new_error(program.name, program.path, "Hàm thủ tục không thể trả về một giá trị", line, column);
        }
    } else {
        // Handle other statement types
        auto [line, column] = stmt->pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Câu lệnh không xác định", line, column);
    }
}

void bao::Analyzer::analyze_expression(sema::SymbolTable &parentTable, ast::ExprNode* expr) {
    if (dynamic_cast<ast::NumLitExpr*>(expr)) {
        // Analyze number literal expression
        // No action needed for number literals
    } else if (auto bin_expr = dynamic_cast<ast::BinExpr*>(expr)) {
        auto left = bin_expr->get_left();
        auto right = bin_expr->get_right();

        // Resolve left and right's type if they're binary expressions (Unknown by default)
        std::vector<std::exception_ptr> exceptions;
        try {
            if (dynamic_cast<ast::BinExpr*>(left)) {
                analyze_expression(parentTable, left);
            }
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
        try {
            if (dynamic_cast<ast::BinExpr*>(right)) {
                analyze_expression(parentTable, right);
            }
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
        if (!exceptions.empty()) {
            throw bao::utils::ErrorList(exceptions);
        }

        // TODO: Implement proper type checking later
        if (left->get_type()->get_name() == right->get_type()->get_name()) {
            expr->set_type(left->get_type()->clone());
            return;
        }

        // Check if can literal cast
        if (!utils::is_literal(left) && !utils::is_literal(right)) {
            auto [line, column] = left->pos();
            throw utils::CompilerError::new_error(
                program.name, program.path, 
                std::format("Kiểu dữ liệu của hai biểu thức khác nhau {} {} {}",
                                    left->get_type()->get_name(),
                                    bin_expr->get_op(),
                                    right->get_type()->get_name()),
                line, column);
        }

        // Left is a number literal
        if (utils::is_literal(left) && !utils::is_literal(right)) {
            auto num_left = dynamic_cast<ast::NumLitExpr*>(left);
            if (utils::can_cast_literal(num_left, right->get_type())) {
                try {
                    utils::cast_literal(num_left, right->get_type());
                    expr->set_type(right->get_type()->clone());
                    return;
                } catch ([[maybe_unused]] exception& e) {
                    throw;
                }
            }
        }
        
        // Right is a number literal
        if (utils::is_literal(right) && !utils::is_literal(left)) {
            auto num_right = dynamic_cast<ast::NumLitExpr*>(right);
            if (utils::can_cast_literal(num_right, left->get_type())) {
                try {
                    utils::cast_literal(num_right, left->get_type());
                    expr->set_type(left->get_type()->clone());
                    return;
                } catch ([[maybe_unused]] exception& e) {
                    throw;
                }
            }
        }
        
        // Fallback
        auto [lline, lcolumn] = left->pos();
        auto [rline, rcolumn] = right->pos();
        throw utils::CompilerError::new_error(
            program.name, 
            program.path, 
            std::format("Kiểu dữ liệu của hai biểu thức khác nhau: {} {} {}",
                                    left->get_type()->get_name(),
                                    bin_expr->get_op(),
                                    right->get_type()->get_name()),
            lline, lcolumn, rcolumn - lcolumn);
    } else {
        // Handle other expression types
        throw std::runtime_error("Kiểu biểu thức không hỗ trợ");
    }
}