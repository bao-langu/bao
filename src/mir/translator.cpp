//
// Created by đỗ quyên on 18/5/25.
//
#include <bao/mir/translator.h>
bao::mir::Translator::Translator(ast::Program &&program): program(std::move(program)) {
    this->module = Module();
}

bao::mir::Module bao::mir::Translator::translate() {
    // Iterate through all functions in the program
    for (ast::FuncNode& func : program.funcs) {
        module.functions.push_back(this->translate_function(func));
    }
    return std::move(module);
}

bao::mir::Function bao::mir::Translator::translate_function(const ast::FuncNode& func) {
    Function function;
    function.name = func.get_name();
    function.return_type = std::make_unique<Type>(*func.get_return_type());
    function.blocks.push_back(std::move(BasicBlock("entry")));
    // Initialize the temporary variable count for this function
    for (const auto& stmt : func.get_stmts()) {
        // Translate each statement
        this->translate_statement(function, stmt.get());
    }
    return std::move(function);
}

void bao::mir::Translator::translate_statement(Function& func, ast::StmtNode* stmt) {
    if (const auto ret_stmt = dynamic_cast<ast::RetStmt*>(stmt)) { // Check if the statement is a return statement
        if (ret_stmt->get_val()) {
            // Translate the return value expression
            auto inst = std::make_unique<ReturnInst>(
                std::move(this->translate_expression(func, stmt, ret_stmt->get_val())));
            func.blocks.back()
                .instructions.push_back(std::move(inst));
        } else {
            // Handle the case where the return value is null
            auto inst = std::make_unique<ReturnInst>(Value(ValueKind::Constant, "null", std::make_unique<PrimitiveType>("null")));
            func.blocks.back()
                .instructions.push_back(std::move(inst));
        }
    } else {
        // Handle other statement types
    }
}

bao::mir::Value bao::mir::Translator::translate_expression(Function& func, ast::StmtNode* stmt, ast::ExprNode* expr) {
    if (const auto numlitexpr = dynamic_cast<ast::NumLitExpr*>(expr)) {
        Value value;
        value.kind = ValueKind::Constant;
        value.name = numlitexpr->get_value();
        value.type = std::make_unique<Type>(*numlitexpr->get_type());
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