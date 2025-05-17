//
// Created by đỗ quyên on 17/5/25.
//
#include <bao/sema/analyzer.h>

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
        const std::string errorMessage = std::format("@{}/{}:\n{}",
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
            analyze_statement(localTable, *stmt, func.get_return_type());
        } catch (...) {
            exceptions.emplace_back(std::current_exception());
        }
    }

    if (!exceptions.empty()) {
        auto [line, column] = func.pos();
        const std::string errorMessage = std::format("@{} -> {} (Dòng {}, Cột {}):\n{}",
                                                func.get_name(),
                                                func.get_return_type()->get_name(),
                                                line,
                                                column,
                                                utils::pad_lines(utils::ErrorList(exceptions).what(), " | "));
        throw std::runtime_error(errorMessage);
    }
}

void bao::Analyzer::analyze_statement(sema::SymbolTable &parentTable, ast::StmtNode &stmt, const Type* return_type) {
    if (const auto* retStmt = dynamic_cast<ast::RetStmt*>(&stmt)) {
        // Analyze return statement
        if (retStmt->get_val()) {
            try {
                analyze_expression(parentTable, *retStmt->get_val());
            } catch (...) {
                auto [line, column] = retStmt->get_val()->pos();
                throw utils::CompilerError::new_error(program.name, program.path, "Biểu thức không xác định", line, column);
            }
        }
        // Check if the return type matches the function's return type
        if (return_type->get_name() != "rỗng") {
            // Check if the return type matches the function's return type
            if (retStmt->get_val()) {
                // For now check only the type name
                if (return_type->get_name() == retStmt->get_val()->get_type()->get_name()) {
                    return;
                }
                // If the return type does not match, check if it can be cast
                if (!utils::is_literal(retStmt->get_val())) {
                    auto [line, column] = retStmt->get_val()->pos();
                    throw utils::CompilerError::new_error(program.name, program.path, "Kiểu trả về khác kiểu trả về của hàm", line, column);
                }
                const auto numExpr = dynamic_cast<ast::NumLitExpr*>(retStmt->get_val());
                if (!utils::can_cast_literal(numExpr, return_type)) {
                    auto [line, column] = numExpr->pos();
                    throw utils::CompilerError::new_error(program.name, program.path, "Kiểu trả về khác kiểu trả về của hàm", line, column);
                }
                // Cast the expression to the function's return type
                utils::cast_literal(numExpr, return_type);
            }
        } else if (retStmt->get_val()) {
            // If the function return type is "rỗng", but a value is returned
            auto [line, column] = retStmt->pos();
            throw utils::CompilerError::new_error(program.name, program.path, "Hàm thủ tục không thể trả về một giá trị", line, column);
        }
    } else {
        // Handle other statement types
        auto [line, column] = stmt.pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Câu lệnh không xác định", line, column);
    }
}

void bao::Analyzer::analyze_expression(sema::SymbolTable &parentTable, ast::ExprNode &expr) {
    if (dynamic_cast<ast::NumLitExpr*>(&expr)) {
        // Analyze number literal expression
        // No action needed for number literals
    } else {
        // Handle other expression types
        throw std::runtime_error("Unsupported expression type");
    }
}