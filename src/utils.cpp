//
// Created by doqin on 13/05/2025.
//
#include "bao/types.h"
#include <bao/utils.h>
#include <iostream>
#include <bao/lexer/maps.h>
#include <cstring>
#include <bao/parser/ast.h>
#include <bao/mir/mir.h>
#include <memory>
#include <stdexcept>

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

void bao::utils::ast::print_program(const bao::ast::Program &program) {
    cout << "Nội dung chương trình:" << endl;
    cout << "   Tên: " << program.name << endl;
    cout << "   Đường dẫn: " << program.path << endl;
    cout << "   Số hàm: " << program.funcs.size() << endl;
    for (const auto &func : program.funcs) {
        ast::print_function(func, "   ");
    }
}

void bao::utils::ast::print_function(const bao::ast::FuncNode& func, const string &padding) {
    auto [line, column] = func.pos();
    std::string type = type_to_string(func.get_return_type());
    const auto message = std::format(
        "Hàm: {} -> {}: {} (Dòng {}, Cột {})",
        func.get_name(), type, func.get_return_type()->get_name(),
        line, column);
    cout << pad_lines(message, padding) << endl;
    for (const auto& stmt_ptr : func.get_stmts()) {
        ast::print_statement(stmt_ptr.get(), padding + " | ");
    }
}

void bao::utils::ast::print_statement(bao::ast::StmtNode* stmt, const string &padding) {
    if (!stmt) {
        cout << padding + "   Câu lệnh không xác định" << endl;
        return;
    }
    auto [line, column] = stmt->pos();
    const auto message = std::format("{} (Dòng {}, Cột {}):", stmt->get_name(), line, column);
    cout << pad_lines(message, padding) << endl;
    if (const auto ret_stmt = dynamic_cast<bao::ast::RetStmt*>(stmt)) {
        if (ret_stmt->get_val()) {
            ast::print_expression(ret_stmt->get_val(), padding + "   ");
        }
    } else {
        cout << padding + "   Biểu thức không xác định" << endl;
    }
}

void bao::utils::ast::print_expression(bao::ast::ExprNode* expr, const string &padding) {
    auto [line, column] = expr->pos();
    std::string type = type_to_string(expr->get_type());
    if (const auto num_expr = dynamic_cast<bao::ast::NumLitExpr*>(expr)) {

        const auto message = std::format(
            "Biểu thức số: {} ({}: {}) (Dòng {}, Cột {})",
            num_expr->get_value(), type, num_expr->get_type()->get_name(), line, column);
        cout << pad_lines(message, padding) << endl;
    } else {
        cout << padding + "\tBiểu thức không xác định" << endl;
    }
}

void bao::utils::mir::print_module(const bao::mir::Module &module) {
    cout << "Nội dung module:" << endl;
    cout << "   Tên: " << module.name << endl;
    cout << "   Đường dẫn: " << module.path << endl;
    cout << "   Số hàm: " << module.functions.size() << endl;
    for (const auto &func : module.functions) {
        print_function(func, "   ");
    }
}

void bao::utils::mir::print_function(const bao::mir::Function &func, const string &padding) {
    cout << padding + "Hàm: " << func.name << endl;
    cout << padding + "   Kiểu trả về: " << type_to_string(func.return_type.get()) << ": "<< func.return_type->get_name() << endl;
    /*
    cout << padding + "   Số tham số: " << func.parameters.size() << endl;
    for (const auto &param : func.parameters) {
        cout << padding + "   Tham số: " << param.get_name() << endl;
    }
    */
    for (const auto &block : func.blocks) {
        print_block(block, padding + "   ");
    }
}

void bao::utils::mir::print_block(const bao::mir::BasicBlock &block, const string &padding) {
    cout << padding + "Khối: " << block.label << endl;
    for (const auto &inst : block.instructions) {
        print_instruction(inst.get(), padding + " | ");
    }
}

void bao::utils::mir::print_instruction(const bao::mir::Instruction* inst, const string &padding) {
    cout << padding + "Lệnh: ";
    switch (inst->kind) {
        case bao::mir::InstructionKind::Assign:
            cout << "Gán: ";
            if (const auto assign = dynamic_cast<const bao::mir::AssignInst*>(inst)) {
                cout << "Từ: ";
                print_value(assign->src, "");
                cout << "Đến: ";
                print_value(assign->dst, "");
            }
            cout << endl;
            break;
        case bao::mir::InstructionKind::Call:
            cout << "Gọi hàm: ";
            if (const auto call = dynamic_cast<const bao::mir::CallInst*>(inst)) {
                cout << "Tên hàm: " << call->function_name << endl;
                cout << "Tham số: ";
                for (const auto &arg : call->arguments) {
                    print_value(arg, "");
                }
            }
            cout << endl;
            break;
        case bao::mir::InstructionKind::Return:
            cout << "Trả về: ";
            if (const auto ret = dynamic_cast<const bao::mir::ReturnInst*>(inst)) {
                print_value(ret->ret_val, "");
            }
            cout << endl;
            break;
        default:
            cout << "Lệnh không xác định" << endl;
            break;
    }
}

void bao::utils::mir::print_value(const bao::mir::Value &value, const string &padding) {
    cout << padding;
    switch (value.kind) {
        case bao::mir::ValueKind::Constant:
            cout << "Hằng số: " << type_to_string(value.type.get()) << ": " << value.name;
            break;
        case bao::mir::ValueKind::Temporary:
            cout << "Biến tạm thời: " << type_to_string(value.type.get()) << ": " << value.name;
            break;
        default:
            cout << "Giá trị không xác định";
            break;
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

bool bao::utils::is_literal(bao::ast::ExprNode* expr) {
    // Null check
    if (!expr) {
        return false;
    }
    if (dynamic_cast<bao::ast::NumLitExpr*>(expr)) {
        return true;
    }
    return false;
}

void bao::utils::cast_literal(bao::ast::NumLitExpr *expr, Type *type) {
    try {
        expr->set_type(type->clone());
    } catch (...) {
        throw std::runtime_error(std::format("Lỗi nội bộ: Không nhận dạng được kiểu chuyển: {}", type->get_name()));
    }
}

bool bao::utils::can_cast_literal(const bao::ast::NumLitExpr *expr, const Type *type) {
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

llvm::Type* bao::utils::get_llvm_type(llvm::IRBuilder<> &builder, bao::Type* type) {
    if (auto prim = dynamic_cast<bao::PrimitiveType*>(type)) {
        switch (prim->get_type()) {
        // LLVM does not differentiate signed and unsigned types
        case bao::Primitive::N32:
        case bao::Primitive::Z32:
            return builder.getInt32Ty();
        case bao::Primitive::N64:
        case bao::Primitive::Z64:
            return builder.getInt64Ty();
        case bao::Primitive::R32:
            return builder.getFloatTy();
        case bao::Primitive::R64:
            return builder.getDoubleTy();
        case bao::Primitive::Void:
            return builder.getVoidTy();
        case bao::Primitive::Null:
            return nullptr;
        default: // Fallthrough
            throw std::runtime_error(std::format("-> Lỗi nội bộ: Không thể chuyển kiểu: {}\n | Kiểu nguyên thuỷ không xác định", type->get_name()));
        }
    }
    throw std::runtime_error(std::format("-> Lỗi nội bộ: Không thể chuyển kiểu: {}", type->get_name()));
}

llvm::Value* bao::utils::get_llvm_value(llvm::IRBuilder<> &builder, bao::mir::Value &mir_value) {
    try {
        switch (mir_value.kind) {
        case bao::mir::ValueKind::Constant:
            if (auto numlit = dynamic_cast<bao::PrimitiveType*>(mir_value.type.get())) {
                auto type = get_llvm_type(builder, numlit);
                if (type->isIntegerTy(32)) {
                    return builder.getInt32(std::stoi(mir_value.name));
                } else if (type->isIntegerTy(64)) {
                    return builder.getInt64(std::stoi(mir_value.name));
                } else if (type->isFloatTy()) {
                    return llvm::ConstantFP::get(builder.getFloatTy(), std::stof(mir_value.name));
                } else if (type->isDoubleTy()) {
                    return llvm::ConstantFP::get(builder.getDoubleTy(), std::stod(mir_value.name));
                } else if (type->isVoidTy()) {
                    return nullptr;
                }
            }
        case bao::mir::ValueKind::Temporary:
        case bao::mir::ValueKind::Variable:
        default:
            ;
        }
        throw std::runtime_error("Lỗi nội bộ: Không thể tạo giá trị llvm");
    } catch (std::exception& e) {
        throw std::runtime_error("Lỗi nội bộ: Không thể tạo giá trị llvm");
    }
}

std::string bao::utils::type_to_string(Type *type) {
    if (auto prim = dynamic_cast<PrimitiveType*>(type)) {
        return "PrimitiveType";
    } else if (auto unknown = dynamic_cast<UnknownType*>(type)) {
        return "Unknown";
    } else {
        return "__error";
    }
}

/*

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(elf)
LLD_HAS_DRIVER(mingw)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(wasm)

int bao::utils::link_obj(std::vector<const char *> &args) {
    lld::Result res = lld::lldMain(args, llvm::outs(), llvm::errs(), LLD_ALL_DRIVERS);
    return res.retCode;
}

*/