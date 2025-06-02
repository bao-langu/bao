//
// Created by đỗ quyên on 17/5/25.
//

#include <bao/parser/ast.h>
#include <bao/sema/symtabl.h>
#include <bao/common/utils.h>
#include <bao/sema/analyzer.h>
#include <exception>

bao::Analyzer :: Analyzer(
    ast::Module &&program
) : program(std::move(program)) {
    this->symbolTable = sema::SymbolTable();
}

auto
bao::Analyzer :: analyze_program() -> bao::ast::Module {
    // Forward declaration of functions
    for (auto& func : program.funcs) {
        // Insert the function into the symbol table
        sema::SymbolInfo info {
            sema::SymbolType::Function,
            func.get_return_type()
        };
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

void 
bao::Analyzer :: analyze_function(
    const ast::FuncNode &func
) {
    sema::SymbolTable localTable(&this->symbolTable);
    // Insert param into the local symbol table
    for (auto& param : func.get_params()) {
        // Insert the parameter into the local symbol table
        sema::SymbolInfo info{};
        info.type = sema::SymbolType::Variable;
        info.datatype = param.get_type();
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
        const std::string errorMessage = 
            std::format("\033[34m@{}\033[0m -> {} (Dòng {}, Cột {}):\n{}",
            func.get_name(),
            func.get_return_type()->get_name(),
            line,
            column,
            utils::pad_lines(utils::ErrorList(exceptions).what(), " | "));
        throw std::runtime_error(errorMessage);
    }
}

void 
bao::Analyzer :: analyze_statement(
    sema::SymbolTable &parentTable, 
    ast::StmtNode* stmt, 
    Type* return_type
) {
    if (auto retStmt = dynamic_cast<ast::RetStmt*>(stmt)) {
        try {
            this->analyze_retstmt(parentTable, retStmt, return_type);
        } catch (...) {
            throw;
        }
    } else if (auto varDeclStmt = dynamic_cast<ast::VarDeclStmt*>(stmt)) {
        try {
            this->analyze_vardeclstmt(parentTable, varDeclStmt);
        } catch (...) {
            throw;
        }
    } else if (auto varAssignStmt = dynamic_cast<ast::VarAssignStmt*>(stmt)) {
        try {
            this->analyze_varassignstmt(parentTable, varAssignStmt);
        } catch (...) {
            throw;
        }
    } else {
        // Handle other statement types
        auto [line, column] = stmt->pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Câu lệnh không xác định", line, column);
    }
}

void 
bao::Analyzer :: analyze_retstmt(
    bao::sema::SymbolTable& parentTable, 
    bao::ast::RetStmt* stmt, 
    Type* return_type
) {
    // Analyze return statement
    if (stmt->get_val()) {
        try {
            analyze_expression(parentTable, stmt->get_val());
        } catch (...) {
            throw;
        }
    }
    // Check if the return type matches the function's return type
    if (return_type->get_name() != "rỗng") {
        // Check if the return type matches the function's return type
        if (!stmt->get_val()) return;
        try {
            this->analyze_type(stmt->get_val(), return_type);
        } catch (...) {
            auto [line, column] = stmt->get_val()->pos();
            throw utils::CompilerError::new_error(this->program.name, this->program.path, 
                "Kiểu dữ liệu khác kiểu trả về của hàm", 
                line, column);
        }
    } else if (stmt->get_val()) {
        // If the function return type is "rỗng", but a value is returned
        auto [line, column] = stmt->pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Hàm thủ tục không thể trả về một giá trị", line, column);
    }
}

void
bao::Analyzer :: analyze_vardeclstmt(
    bao::sema::SymbolTable& parentTable, 
    bao::ast::VarDeclStmt* stmt
) {
    if (!parentTable.insert(
        stmt->get_var().get_name(), 
        {
            sema::SymbolType::Variable,
            stmt->get_var().get_type(),
            stmt->get_var().is_const()
        }
    )) {
        auto [line, column] = stmt->get_var().pos();
        throw utils::CompilerError::new_error(
            this->program.name, this->program.path, 
            "Biến '" + stmt->get_var().get_name() + "' đã được khai báo rồi",
            line, column);
    }
    if (!stmt->get_val()) return;
    try {
        this->analyze_expression(parentTable, stmt->get_val());
    } catch (...) {
        throw;
    }
    try {
        this->analyze_type(stmt->get_val(), stmt->get_var().get_type());
    } catch (...) {
        auto [line, column] = stmt->get_val()->pos();
        throw utils::CompilerError::new_error(this->program.name, this->program.path, 
            std::format(
                "Kiểu dữ liệu ({}) khác kiểu dữ liệu của biến ({})",
                stmt->get_val()->get_type()->get_name(),
                stmt->get_var().get_type()->get_name()
            ), 
            line, column);
    }
}

void
bao::Analyzer :: analyze_varassignstmt(
    sema::SymbolTable& parentTable, 
    ast::VarAssignStmt* stmt
) {
    auto symbol = 
        parentTable.lookup(
            stmt->get_var().get_name()
        );
    if (!symbol || symbol->type != sema::SymbolType::Variable) {
        auto [line, column] = stmt->pos();
        throw utils::CompilerError::new_error(
            this->program.name, this->program.path,
            "Biến không xác định, chưa khai báo hoặc trùng tên với hàm", 
            line, column
        );
    }

    // Resolve type
    stmt->get_var().set_type(symbol->datatype->clone());

    // Constants cannot be reassigned
    if (symbol->isConst) {
        auto [line, column] = stmt->pos();
        throw utils::CompilerError::new_error(
            this->program.name, this->program.path,
            "Hằng số không thể gán lại giá trị", 
            line, column
        );
    }

    try {
        this->analyze_expression(parentTable, stmt->get_val());
    } catch (...) {
        throw;
    }
    try {
        this->analyze_type(stmt->get_val(), symbol->datatype);
    } catch (...) {
        auto [line, column] = stmt->get_val()->pos();
        throw utils::CompilerError::new_error(this->program.name, this->program.path, 
            std::format(
                "Kiểu dữ liệu ({}) khác kiểu dữ liệu của biến ({})",
                stmt->get_val()->get_type()->get_name(),
                symbol->datatype->get_name()
            ), 
            line, column);
    }
}

void 
bao::Analyzer :: analyze_expression(
    sema::SymbolTable &parentTable, 
    ast::ExprNode* expr
) {
    if (dynamic_cast<ast::NumLitExpr*>(expr)) {
        // Analyze number literal expression
        // No action needed for number literals
    } else if (auto var = dynamic_cast<ast::VarExpr*>(expr)) {
        auto symbol = parentTable.lookup(var->get_name());
        if (!symbol || symbol->type != sema::SymbolType::Variable) {
            auto [line, column] = var->pos();
            throw utils::CompilerError::new_error(
                this->program.name, this->program.path,
                "Biến không xác định, chưa khai báo hoặc trùng tên với hàm", 
                line, column
            );
        }
        var->set_type(symbol->datatype->clone());
    } else if (auto bin_expr = dynamic_cast<ast::BinExpr*>(expr)) {
        auto left = bin_expr->get_left();
        auto right = bin_expr->get_right();

        // Resolve left and right's type if they're binary expressions (Unknown by default)
        std::vector<std::exception_ptr> exceptions;

        try {
            analyze_expression(parentTable, left);
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
        try {
            analyze_expression(parentTable, right);
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
        auto [lline, lcolumn] = left->pos();
        auto [rline, rcolumn] = right->pos();
        if (!utils::is_literal(left) && !utils::is_literal(right)) {
            auto [line, column] = left->pos();
            throw utils::CompilerError::new_error(
                program.name, program.path, 
                std::format("Kiểu dữ liệu của hai biểu thức khác nhau {} {} {}",
                                    left->get_type()->get_name(),
                                    bin_expr->get_op(),
                                    right->get_type()->get_name()),
                lline, lcolumn, rcolumn - lcolumn);
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

void 
bao::Analyzer :: analyze_type(
    ast::ExprNode* val, 
    Type* type
) {
    // TODO: For now check only the type name, check it better later
    if (val->get_type()->get_name() == type->get_name()) {
        return;
    }

    // If the type does not match, check if it can be literal cast
    if (!utils::is_literal(val)) {
        auto [line, column] = val->pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Lỗi nội bộ: Không thể chuyển kiểu", line, column);
    }
    if (!utils::can_cast_literal(val, type)) {
        auto [line, column] = val->pos();
        throw utils::CompilerError::new_error(program.name, program.path, "Lỗi nội bộ: Không thể chuyển kiểu", line, column);
    }

    // Cast the expression to the type
    try {
        utils::cast_literal(val, type);
    } catch ([[maybe_unused]] std::exception& e) {
        throw;
    }
}