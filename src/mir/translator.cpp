//
// Created by đỗ quyên on 18/5/25.
//
#include "bao/mir/mir.h"
#include "bao/parser/ast.h"
#include "bao/types.h"
#include "bao/utils.h"
#include <bao/mir/translator.h>
#include <exception>
#include <iostream>
#include <memory>

// ⚠ Warning: This code contains non-standard spacing and formatting.
// ⚠ Readability over orthodoxy. Fight me, ISO committee.

bao::mir::Translator :: Translator(
    ast::Program &&program
) : program(std::move(program)) {
    this->module = Module();
    this->module.name = this->program.name;
    this->module.path = this->program.path;
}

auto
bao::mir::Translator :: translate() -> bao::mir::Module {
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

auto
bao::mir::Translator :: translate_function(
    const ast::FuncNode& func
) -> bao::mir::Function {
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

void 
bao::mir::Translator :: translate_statement(
    Function& func, 
    ast::StmtNode* stmt
) {
    try {
        if (const auto ret_stmt = dynamic_cast<ast::RetStmt*>(stmt)) { // Check if the statement is a return statement
            if (ret_stmt->get_val()) {
                // Translate the return value expression
                func.blocks.back()
                    .instructions.push_back(
                        std::make_unique<ReturnInst>(
                            std::move(
                                this->translate_expression(func, stmt, ret_stmt->get_val())
                            )
                        )
                    );
            } else {
                // Handle the case where the return value is null
                auto inst = std::make_unique<ReturnInst>(
                    Value(ValueKind::Constant, "rỗng", std::make_unique<PrimitiveType>("rỗng")));
                func.blocks.back()
                    .instructions.push_back(std::move(inst));
            }
        } else if (const auto vardecl_stmt = dynamic_cast<ast::VarDeclStmt*>(stmt)) {
            auto& var = vardecl_stmt->get_var();
            Value dst{
                ValueKind::Variable,
                var.get_name(),
                var.get_type()->clone()
            };
            // Create a stack allocated variable
            func.blocks.back()
                .instructions
                .push_back(
                    std::make_unique<AllocInst>(
                        dst
                    )
                );
            // Store a value to it if it has an initializer
            if (vardecl_stmt->get_val()) {
                auto src = 
                    this->translate_expression(
                        func, 
                        stmt, 
                        vardecl_stmt->get_val()
                    );
                func.blocks.back()
                    .instructions
                    .push_back(
                        std::make_unique<StoreInst>(
                            std::move(src),
                            dst
                        )
                    );
            }
        } else {
            // Handle other statement types
            auto [line, column] = stmt->pos();
            std::cout 
                << std::format(
                    "Cảnh báo: Bắt gặp kiểu câu lệnh không xác định (Dòng {}, Cột {})",
                    line, column
                )
                << std::endl;
        }
    } catch ([[maybe_unused]] std::exception& e) {
        throw;
    }
}

auto
bao::mir::Translator :: translate_expression(
    Function& func, 
    ast::StmtNode* stmt, // For some context
    ast::ExprNode* expr
) -> bao::mir::Value {
    // Most basic, number literal
    if (const auto numlitexpr = dynamic_cast<ast::NumLitExpr*>(expr)) {
        Value value {
            ValueKind::Constant,
            numlitexpr->get_val(),
            numlitexpr->get_type()->clone()
        };
        return std::move(value);
    }
    // Extract the value from a var
    if (const auto varexpr = dynamic_cast<ast::VarExpr*>(expr)) {
        Value dst {
            ValueKind::Temporary,
            "__temp" + std::to_string(func.temp_var_count++),
            varexpr->get_type()->clone()
        };
        // Translation will not check for validity as it's checked in Analyzer already
        Value src {
            ValueKind::Variable,
            varexpr->get_name(),
            varexpr->get_type()->clone()
        };
        func.blocks.back().instructions.push_back(
            std::make_unique<LoadInst>(
                dst,
                std::move(src)
            )
        );
        return std::move(dst);
    }
    // Binary expresions
    if (const auto binexpr = dynamic_cast<ast::BinExpr*>(expr)) {
        auto left = translate_expression(func, stmt, binexpr->get_left());
        auto right = translate_expression(func, stmt, binexpr->get_right());
        auto type = binexpr->get_type();
        Value dst {
            ValueKind::Temporary,
            "__temp" + std::to_string(func.temp_var_count++),
            type->clone()
        };
        try {
            utils::match(binexpr->get_op(), {
                {
                    "+", [&] {
                        // Signed add
                        if (utils::is_signed(type)) {
                            if (utils::is_float(type)) {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Add_f,
                                        std::move(right)
                                    )
                                );
                            } else {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Add_s,
                                        std::move(right)
                                    )
                                );
                            }
                        // Unsigned add
                        } else {
                            func.blocks.back().instructions.push_back(
                                std::make_unique<BinInst>(
                                    dst,
                                    std::move(left),
                                    BinaryOp::Add_u,
                                    std::move(right)
                                )
                            );
                        }
                    }
                },
                {
                    "-", [&] {
                        if (utils::is_signed(type)) {
                            if (utils::is_float(type)) {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Sub_f,
                                        std::move(right)
                                    )
                                );
                            } else {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Sub_s,
                                        std::move(right)
                                    )
                                );
                            }
                        } else {
                            func.blocks.back().instructions.push_back(
                                std::make_unique<BinInst>(
                                    dst,
                                    std::move(left),
                                    BinaryOp::Sub_u,
                                    std::move(right)
                                )
                            );
                        }
                    }
                },
                {
                    "*", [&] {
                        if (utils::is_signed(type)) {
                            if (utils::is_float(type)) {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Mul_f,
                                        std::move(right)
                                    )
                                );
                            } else {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Mul_s,
                                        std::move(right)
                                    )
                                );
                            }
                        } else {
                            func.blocks.back().instructions.push_back(
                                std::make_unique<BinInst>(
                                    dst,
                                    std::move(left),
                                    BinaryOp::Mul_u,
                                    std::move(right)
                                )
                            );
                        }
                    }
                },
                {
                    // Special operation due to IEEE-754
                    "/", [&] {
                        if (utils::is_signed(type)) {
                            // Floating point numbers
                            if (utils::is_float(type)) {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Div_f,
                                        std::move(right)
                                    )
                                );
                            // Dividing signed integers
                            } else {
                                func.blocks.back().instructions.push_back(
                                    std::make_unique<BinInst>(
                                        dst,
                                        std::move(left),
                                        BinaryOp::Div_s,
                                        std::move(right)
                                    )
                                );
                            }
                        // Dividing unsigned integers
                        } else {
                            func.blocks.back().instructions.push_back(
                                std::make_unique<BinInst>(
                                    dst,
                                    std::move(left),
                                    BinaryOp::Div_u,
                                    std::move(right)
                                )
                            );
                        }
                    }
                }
            }, [&expr, this] {
                auto [line, column] = expr->pos();
                throw utils::CompilerError::new_error(
                    this->module.name, this->module.path, 
                    "Biểu thức không xác định", line, column);
            });
        } catch ([[maybe_unused]] exception& e) {
            throw;
        }
        return std::move(dst);
    }
    return {};
}