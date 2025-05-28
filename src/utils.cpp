//
// Created by doqin on 13/05/2025.
//
#include <bao/types.h>
#include <bao/utils.h>
#include <iostream>
#include <bao/lexer/maps.h>
#include <cstring>
#include <bao/parser/ast.h>
#include <bao/mir/mir.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <stdexcept>
#include <algorithm>

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
            ast::print_expression(ret_stmt->get_val(), padding);
            std::cout << std::endl;
        }
    } else if (const auto vardecl_stmt = dynamic_cast<bao::ast::VarDeclStmt*>(stmt)) {
        auto& var = vardecl_stmt->get_var();
        auto type = var.get_type();
        auto [var_line, var_column] = var.pos();
        std::string var_name = "Biến";
        if (var.is_const()) {
            var_name = "Hằng";
        }
        cout << padding + " $ " + var_name + ": " << std::format("{} ({}: {}) (Dòng {}, Cột {})",
                vardecl_stmt->get_var().get_name(),
                utils::type_to_string(type), type->get_name(),
                var_line, var_column
            );
        if (vardecl_stmt->get_val()) {
            cout << " := " << endl;
            ast::print_expression(vardecl_stmt->get_val(), padding + "   ");
        }
        std::cout << std::endl;
    } else {
        cout << padding + " ? Biểu thức không xác định";
        std::cout << std::endl;
    }
    std::cout << padding << std::endl;
}

void bao::utils::ast::print_expression(bao::ast::ExprNode* expr, const string &padding) {
    auto [line, column] = expr->pos();
    std::string type = type_to_string(expr->get_type());
    if (const auto num_expr = dynamic_cast<bao::ast::NumLitExpr*>(expr)) {
        const auto message = std::format(
            " $ Biểu thức số: {} ({}: {}) (Dòng {}, Cột {})",
            num_expr->get_val(), type, num_expr->get_type()->get_name(), line, column
        );
        cout << pad_lines(message, padding);
    } else if (const auto var_expr = dynamic_cast<bao::ast::VarExpr*>(expr)) {
        const auto message = std::format(
            " $ Biểu thức biến: {} ({}: {}) (Dòng {}, Cột {})",
            var_expr->get_name(), type, var_expr->get_type()->get_name(), line, column
        );
        cout << pad_lines(message, padding);
    } else if (const auto bin_expr = dynamic_cast<bao::ast::BinExpr*>(expr)){
        const auto message = std::format(
            " $ Biểu thức nhị phân ({}: {}) (Dòng {}, Cột {}):",
            type, bin_expr->get_type()->get_name(), line, column
        );
        std::cout << pad_lines(message, padding) << std::endl;
        print_expression(bin_expr->get_left(), padding + "   ");
        std::cout << std::endl;
        cout << padding + "      Phép toán: " + bin_expr->get_op() << std::endl;
        print_expression(bin_expr->get_right(), padding + "   ");
    } else {
        cout << padding + " ? Biểu thức không xác định";
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
    cout << padding;    
    if (const auto assign = dynamic_cast<const bao::mir::AssignInst*>(inst)) {
        cout << "assigninst: ";
        print_value(assign->dst, "");
        cout << " = ";
        print_value(assign->src, "");
    } else if (const auto call = dynamic_cast<const bao::mir::CallInst*>(inst)) {
        cout << "callinst: ";
        cout << "Tên hàm: " << call->function_name << endl;
        cout << "Tham số: ";
        for (const auto &arg : call->arguments) {
            print_value(arg, "");
        }
    } else if (const auto ret = dynamic_cast<const bao::mir::ReturnInst*>(inst)) {
        cout << "retinst: ";
        print_value(ret->ret_val, "");
    } else if (const auto bin = dynamic_cast<const bao::mir::BinInst*>(inst)) {
        cout << "bininst: ";
        cout << bin->dst.name << " = ";
        switch(bin->op) {
        case bao::mir::BinaryOp::Add_f:
            cout << "add_f: ";
            break;
        case bao::mir::BinaryOp::Add_s:
            cout << "add_c: ";
            break;
        case bao::mir::BinaryOp::Add_u:
            cout << "add_u: ";
            break;
        case bao::mir::BinaryOp::Sub_f:
            cout << "sub_f: ";
            break;
        case bao::mir::BinaryOp::Sub_s:
            cout << "sub_c: ";
            break;
        case bao::mir::BinaryOp::Sub_u:
            cout << "sub_u: ";
            break;
        case bao::mir::BinaryOp::Mul_f:
            cout << "mul_f: ";
            break;
        case bao::mir::BinaryOp::Mul_s:
            cout << "mul_c: ";
            break;
        case bao::mir::BinaryOp::Mul_u:
            cout << "mul_u: ";
            break;
        case bao::mir::BinaryOp::Div_s:
            cout << "div_s: ";
            break;
        case bao::mir::BinaryOp::Div_u:
            cout << "div_u: ";
            break;
        case bao::mir::BinaryOp::Div_f:
            cout << "div_f: ";
            break;
        case bao::mir::BinaryOp::Rem_s:
            cout << "rem_s: ";
            break;
        case bao::mir::BinaryOp::Rem_u:
            cout << "rem_u: ";
            break;
        case bao::mir::BinaryOp::Lt_s:
            cout << "lt_s: ";
            break;
        case bao::mir::BinaryOp::Lt_u:
            cout << "lt_u: ";
            break;
        }
        print_value(bin->left, "");
        cout << ", ";
        print_value(bin->right, "");
    } else {
        cout << "Lệnh không xác định";
    }
    cout << endl;
}

void bao::utils::mir::print_value(const bao::mir::Value &value, const string &padding) {
    cout << padding;
    switch (value.kind) {
        case bao::mir::ValueKind::Constant:
            cout << "const(" << type_to_string(value.type.get()) << ": " << value.name << ")";
            break;
        case bao::mir::ValueKind::Temporary:
            cout << "temp(" << type_to_string(value.type.get()) << ": " << value.name << ")";
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
    if (auto bin_expr = dynamic_cast<bao::ast::BinExpr*>(expr)) {
        return is_literal(bin_expr->get_left()) && is_literal(bin_expr->get_right());
    }
    return false;
}

void bao::utils::cast_literal(bao::ast::ExprNode *expr, Type *type) {
    try {
        expr->set_type(type->clone());
        if (auto bin_expr = dynamic_cast<bao::ast::BinExpr*>(expr)) {
            bao::utils::cast_literal(bin_expr->get_left(), type);
            bao::utils::cast_literal(bin_expr->get_right(), type);
        }
    } catch (...) {
        throw std::runtime_error(std::format("Lỗi nội bộ: Không nhận dạng được kiểu chuyển: {}", type->get_name()));
    }
}

bool bao::utils::can_cast_literal(const bao::ast::ExprNode *expr, const Type *type) {
    if (!expr->get_type()) {
        return false;
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

std::string bao::utils::type_to_string(Type *type) {
    if (auto prim = dynamic_cast<PrimitiveType*>(type)) {
        return "PrimitiveType";
    } else if (auto unknown = dynamic_cast<UnknownType*>(type)) {
        return "Unknown";
    } else {
        return "__error";
    }
}

// Helper function guide Linux into the program
int bao::utils::generate_start() {
    // LLVM module
    llvm::LLVMContext context;
    llvm::Module module("_start", context);
    llvm::IRBuilder<> builder(context);

    // Declare the main function to call
    llvm::FunctionType* mainType = 
        llvm::FunctionType::get(
            builder.getInt32Ty(),
            false);
    llvm::Function* main = 
        llvm::Function::Create(
            mainType, 
            llvm::Function::ExternalLinkage, 
            "main",
            module);

    // Declare the exit() from cstdlib
    llvm::FunctionType* exitType = 
        llvm::FunctionType::get(
            builder.getVoidTy(), 
            {builder.getInt32Ty()}, 
            false);
    llvm::Function* exit =
        llvm::Function::Create(
            exitType,
            llvm::Function::ExternalLinkage,
            "exit",
            module
        );

    // Linux's starting function
    llvm::FunctionType* startType =
        llvm::FunctionType::get(
            builder.getVoidTy(), 
            false);
    llvm::Function* start =
        llvm::Function::Create(
            startType,
            llvm::Function::ExternalLinkage,
            "_start",
            module
        );
    
    llvm::BasicBlock* entryBlock = 
        llvm::BasicBlock::Create(
            context, 
            "entry",
            start);

    // Let's go instruct the entry block
    builder.SetInsertPoint(entryBlock);

    // Run the main function
    llvm::Value* ret = builder.CreateCall(main);
    
    // Evaluate the return value
    builder.CreateCall(exit, ret);

    // Done
    builder.CreateUnreachable();

    // Print for debugging
    #ifndef NDEBUG
        module.print(llvm::outs(), nullptr);
    #endif

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();

    auto targetTriple = 
        llvm::sys::getDefaultTargetTriple();

    std::string error;
    const auto* target = 
        llvm::TargetRegistry::lookupTarget(
            targetTriple, 
            error);

    if (!error.empty()) {
        std::cerr << error << "Lỗi nội bộ: \n";
        return 1;
    }

    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto targetMachine =
        target->createTargetMachine(
            targetTriple, 
            "generic", 
            "",
            opt, 
            RM);

    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(targetTriple);

    std::error_code EC;
    llvm::raw_fd_ostream dest("_start.o", EC, llvm::sys::fs::OF_None);
    //               Linux obj stuff ^
    if (EC) {
        llvm::errs() << "Lỗi nội bộ: Gặp sự cố mở tệp _start.o: " << EC.message() << "\n";
        return 1;
    }
    llvm::legacy::PassManager pass;
    targetMachine->addPassesToEmitFile(
        pass, 
        dest, 
        nullptr, 
        llvm::CodeGenFileType::ObjectFile);
    pass.run(module);
    return 0;
}

void bao::utils::trim(std::string& str) {
    while (!str.empty() && (str.back() == '\n' || str.back() == '\r')) {
        str.pop_back();
    }
}

bool bao::utils::is_signed(bao::Type* type) {
    const auto type_name = type->get_name();
    std::vector<std::string> valid = {"Z32", "Z64", "R32", "R64"};
    if (std::any_of(valid.begin(), valid.end(), 
        [&](const std::string& s) { return s == type_name; })) {
        return true;
    }
    return false;
}

bool bao::utils::is_float(bao::Type* type) {
    const auto type_name = type->get_name();
    std::vector<std::string> valid = {"R32", "R64"};
    if (std::any_of(valid.begin(), valid.end(), 
        [&](const std::string& s) { return s == type_name; })) {
        return true;
    }
    return false;
}