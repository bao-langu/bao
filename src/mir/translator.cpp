//
// Created by đỗ quyên on 18/5/25.
//
#include "bao/types.h"
#include "bao/utils.h"
#include <bao/mir/translator.h>
#include <exception>
#include <memory>

bao::mir::Translator::Translator(ast::Program &&program): program(std::move(program)) {
    this->module = Module();
    this->module.name = this->program.name;
    this->module.path = this->program.path;
}

bao::mir::Module bao::mir::Translator::translate() {
    // Iterate through all functions in the program
    std::vector<std::exception_ptr> exceptions;
    try {
        for (ast::FuncNode& func : program.funcs) {
            module.functions.push_back(this->translate_function(func));
        }
    } catch (...) {
        exceptions.push_back(std::current_exception());
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }
    return std::move(module);
}

bao::mir::Function bao::mir::Translator::translate_function(const ast::FuncNode& func) {
    Function function;
    std::string main_sym = "main"; // For most platforms
    function.name = func.get_name() == "chính" ? main_sym : func.get_name();
    try {
        function.return_type = func.get_return_type()->clone();
    } catch ([[maybe_unused]] std::exception& e) {
        throw;
    }
    auto [line, column] = func.pos();
    function.line = line;
    function.column = column;
    // Fall back in case of wrong main semantics
    if (function.name == main_sym && function.return_type->get_name() != "Z32") {
        auto [line, column] = func.pos();
        throw utils::CompilerError::new_error(
            program.name, program.path, 
            "Hàm chính phải có kiểu trả về là Z32", 
            line, column);
    }
    function.blocks.push_back(std::move(BasicBlock("entry")));
    // Initialize the temporary variable count for this function
    for (const auto& stmt : func.get_stmts()) {
        try {
            // Translate each statement
            this->translate_statement(function, stmt.get());
        } catch ([[maybe_unused]] std::exception& e) {
            throw;
        }
    }
    return std::move(function);
}

void bao::mir::Translator::translate_statement(Function& func, ast::StmtNode* stmt) {
    try {
        if (const auto ret_stmt = dynamic_cast<ast::RetStmt*>(stmt)) { // Check if the statement is a return statement
            if (ret_stmt->get_val()) {
                // Translate the return value expression
                auto inst = std::make_unique<ReturnInst>(
                    std::move(this->translate_expression(func, stmt, ret_stmt->get_val())));
                func.blocks.back()
                    .instructions.push_back(std::move(inst));
            } else {
                // Handle the case where the return value is null
                auto inst = std::make_unique<ReturnInst>(
                    Value(ValueKind::Constant, "rỗng", std::make_unique<PrimitiveType>("rỗng")));
                func.blocks.back()
                    .instructions.push_back(std::move(inst));
            }
        } else {
            // Handle other statement types
        }
    } catch ([[maybe_unused]] std::exception& e) {
        throw;
    }
}

bao::mir::Value bao::mir::Translator::translate_expression(Function& func, ast::StmtNode* stmt, ast::ExprNode* expr) {
    if (const auto numlitexpr = dynamic_cast<ast::NumLitExpr*>(expr)) {
        Value value;
        value.kind = ValueKind::Constant;
        value.name = numlitexpr->get_val();
        value.type = numlitexpr->get_type()->clone();
        return std::move(value);
    }
    /* For other expression types that need loading
    Value value;
    value.kind = ValueKind::Temporary;
    value.name = "__temp_var_" + std::to_string(this->temp_var_count[func]++);
    value.type = std::make_unique<Type>(*func.return_type.get());
    return std::move(value);
    */
    return {};
}