#include <bao/utils.h>
#include <bao/mir/mir.h>
#include <bao/codegen/generator.h>
#include <exception>
#include <stdexcept>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Intrinsics.h>
#include <iostream>

bao::Generator :: Generator(
    bao::mir::Module&& mir_module
) : mir_module(std::move(mir_module)), 
    llvm_module(this->mir_module.name, this->context),
    ir_builder(this->context) {}

void
bao::Generator :: generate() {
    std::vector<std::exception_ptr> exceptions;
    for (auto& func : this->mir_module.functions) {
        try {
            this->generate_function(func);
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }
}

void
bao::Generator :: print_source() {
    this->llvm_module.print(llvm::outs(), nullptr);
}

auto
bao::Generator :: create_object(
    const std::string& filename
) -> int {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    const auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!error.empty()) {
        std::cerr << error << "\n";
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

    // Fix on Windows, not sure why it didn't need it on macOS and Linux
    this->llvm_module.setDataLayout(targetMachine->createDataLayout());
    this->llvm_module.setTargetTriple(targetTriple);

    std::error_code EC;
    std::cout << "Đã lưu tại: " << filename << std::endl;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Gặp sự cố mở tệp: " << EC.message() << "\n";
        return 1;
    }
    llvm::legacy::PassManager pass;
    targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile);
    pass.run(this->llvm_module);
    dest.flush();
    // llvm::WriteBitcodeToFile(this->llvm_module, outFile);
    return 0;
}

void
bao::Generator :: generate_function(
    mir::Function& mir_func
) {
    this->current_context = {};
    try {
        // Get function type - Can be thrown an error
        llvm::FunctionType *funcType = 
            llvm::FunctionType::get(
                utils::get_llvm_type(
                    this->ir_builder, 
                    mir_func.return_type.get()), 
                false);
        
        // Creating the function
        llvm::Function *ir_func = 
            llvm::Function::Create(
                funcType, 
                llvm::Function::ExternalLinkage, 
                mir_func.name, 
                this->llvm_module);

        // Generating the blocks
        std::vector<exception_ptr> exceptions;
        for (auto& mir_block : mir_func.blocks) {    
            try {
                llvm::BasicBlock *ir_block = 
                    llvm::BasicBlock::Create(
                        this->context, 
                        mir_block.label, 
                        ir_func);
                this->generate_block(ir_block, mir_block);
            } catch (...) {
                exceptions.push_back(std::current_exception());
            }
        }
        if (!exceptions.empty()) {
            throw utils::ErrorList(exceptions);
        }

    } catch (std::runtime_error& e) {
        throw utils::CompilerError::new_error(
            this->mir_module.name, this->mir_module.path, 
            std::format("Không thể xác định được kiểu trả về của hàm này:\n{}", e.what()), 
            mir_func.line, mir_func.column);
    } catch ([[maybe_unused]] std::exception& e) {
        throw;
    }
}

void
bao::Generator :: generate_block(
    llvm::BasicBlock* ir_block, 
    bao::mir::BasicBlock& mir_block
) {
    this->ir_builder.SetInsertPoint(ir_block);
    std::vector<std::exception_ptr> exceptions;
    for (auto& mir_inst : mir_block.instructions) {
        try {
            this->generate_instruction(mir_inst.get());
        } catch (...) {
            exceptions.push_back(std::current_exception());
        }
    }
    if (!exceptions.empty()) {
        throw utils::ErrorList(exceptions);
    }
}

void
bao::Generator :: generate_instruction(
    bao::mir::Instruction* mir_inst
) {
    try {
        // Return instruction
        if (auto retInst = dynamic_cast<mir::ReturnInst*>(mir_inst)) {
            auto val = get_llvm_value(retInst->ret_val);
            if (val) {
                this->ir_builder.CreateRet(val);
            } else {
                this->ir_builder.CreateRetVoid();
            }
            return;
        }

        // Stack allocation instruction
        if (auto allocInst = dynamic_cast<mir::AllocInst*>(mir_inst)) {
            auto alloca = this->ir_builder.CreateAlloca(
                utils::get_llvm_type(
                    this->ir_builder, 
                    allocInst->dst.type.get()
                )
            );
            alloca->setName(allocInst->dst.name);
            current_context[allocInst->dst.name] = alloca;
            return;
        }

        // Store to a pointer
        if (auto storeInst = dynamic_cast<mir::StoreInst*>(mir_inst)) {
            auto src = this->get_llvm_value(storeInst->src);
            auto dst = this->get_llvm_value(storeInst->dst);
            // TODO: Handle volatility
            this->ir_builder.CreateStore(src, dst, false);
            return;
        }

        // Load from an alloca
        if (auto loadInst = dynamic_cast<mir::LoadInst*>(mir_inst)) {
            auto src = this->get_llvm_value(loadInst->src);

            auto load = this->ir_builder.CreateLoad(
                utils::get_llvm_type(
                    this->ir_builder,
                    loadInst->dst.type.get()
                ),
                src
            );
            load->setName(loadInst->dst.name);
            current_context[loadInst->dst.name] = load;
            return;
        }

        // Arithmatic instructions
        if (auto binInst = dynamic_cast<mir::BinInst*>(mir_inst)) {
            auto left = get_llvm_value(binInst->left);
            auto right = get_llvm_value(binInst->right);
            llvm::Value* dst = nullptr;
            switch (binInst->op) {
            case mir::BinaryOp::Add_f: {
                dst = this->ir_builder.CreateFAdd(left, right, binInst->dst.name);
            }
            break;
            case mir::BinaryOp::Add_s: {
                llvm::Function *saddFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::sadd_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    saddFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                // Not really doing anything for now
                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Add_u: {
                llvm::Function *uaddFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::uadd_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    uaddFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Sub_f: {
                dst = this->ir_builder.CreateFSub(left, right, binInst->dst.name);
            }
            break;
            case mir::BinaryOp::Sub_s: {
                llvm::Function *ssubFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::ssub_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    ssubFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Sub_u: {
                llvm::Function *usubFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::usub_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    usubFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Mul_f: {
                dst = this->ir_builder.CreateFMul(left, right, binInst->dst.name);
            }
            break;
            case mir::BinaryOp::Mul_s: {
                llvm::Function *smulFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::smul_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    smulFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Mul_u: {
                llvm::Function *umulFunc = llvm::Intrinsic::getOrInsertDeclaration(
                    &this->llvm_module, 
                    llvm::Intrinsic::umul_with_overflow,
                    {bao::utils::get_llvm_type(
                        this->ir_builder,
                        binInst->dst.type.get()
                    )}
                );

                llvm::Value *resStruct = this->ir_builder.CreateCall(
                    umulFunc, {left, right}
                );

                dst = this->ir_builder.CreateExtractValue(
                    resStruct, 0
                );

                llvm::Value *overflow = this->ir_builder.CreateExtractValue(
                    resStruct, 1
                );
            }
            break;
            case mir::BinaryOp::Div_s: {
                dst = this->ir_builder.CreateSDiv(left, right, binInst->dst.name);
            }
            break;
            case mir::BinaryOp::Div_u: {
                dst = this->ir_builder.CreateUDiv(left, right, binInst->dst.name);
            }
            break;
            case mir::BinaryOp::Div_f: {
                dst = this->ir_builder.CreateFDiv(left, right, binInst->dst.name);
            }
            break;
            // TODO: Later
            case mir::BinaryOp::Rem_s:
            case mir::BinaryOp::Rem_u:
            case mir::BinaryOp::Lt_s:
            case mir::BinaryOp::Lt_u:
                throw std::runtime_error("Lỗi nội bộ: Chưa hỗ trợ lấy số dư hoặc so sánh");
                break;
            }
            dst->setName(binInst->dst.name);
            current_context[binInst->dst.name] = dst;
            return;
        }
        throw std::runtime_error("Lỗi nội bộ: Không xác định được kiểu câu lệnh");
    } catch (std::exception& e) {
        throw; // FIXME: Rethrow for now
    }
}

auto
bao::Generator :: get_llvm_value(
    bao::mir::Value &mir_value
) -> llvm::Value* {
    try {
        switch (mir_value.kind) {
        case bao::mir::ValueKind::Constant:
            if (auto numlit = dynamic_cast<bao::PrimitiveType*>(mir_value.type.get())) {
                auto type = bao::utils::get_llvm_type(ir_builder, numlit);
                if (type->isIntegerTy(32)) {
                    return ir_builder.getInt32(std::stoi(mir_value.name));
                } else if (type->isIntegerTy(64)) {
                    return ir_builder.getInt64(std::stoi(mir_value.name));
                } else if (type->isFloatTy()) {
                    return llvm::ConstantFP::get(ir_builder.getFloatTy(), std::stof(mir_value.name));
                } else if (type->isDoubleTy()) {
                    return llvm::ConstantFP::get(ir_builder.getDoubleTy(), std::stod(mir_value.name));
                } else if (type->isVoidTy()) {
                    return nullptr;
                }
            }
        case bao::mir::ValueKind::Temporary:
        case bao::mir::ValueKind::Variable:
            // In faith I trust this won't break (pls don't break)
            return current_context.at(mir_value.name);
        default:
            ;
        }
        throw std::runtime_error("Lỗi nội bộ: Không thể tạo giá trị llvm");
    } catch (std::exception& e) {
        throw std::runtime_error("Lỗi nội bộ: Không thể tạo giá trị llvm");
    }
}