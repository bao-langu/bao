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
#include <iostream>

bao::Generator::Generator(bao::mir::Module&& mir_module
    ) : mir_module(std::move(mir_module)), 
    llvm_module(this->mir_module.name, this->context),
    ir_builder(this->context) {}

void bao::Generator::generate() {
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

void bao::Generator::print_source() {
    this->llvm_module.print(llvm::outs(), nullptr);
}

int bao::Generator::create_object(const std::string& filename) {
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

void bao::Generator::generate_function(mir::Function& mir_func) {
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

void bao::Generator::generate_block(llvm::BasicBlock* ir_block, bao::mir::BasicBlock& mir_block) {
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

void bao::Generator::generate_instruction(bao::mir::Instruction* mir_inst) {
    try {
        if (auto retInst = dynamic_cast<mir::ReturnInst*>(mir_inst)) {
            llvm::Value* val = utils::get_llvm_value(this->ir_builder, retInst->ret_val);
            if (val) {
                this->ir_builder.CreateRet(val);
            } else {
                this->ir_builder.CreateRetVoid();
            }
            return;
        }
        throw std::runtime_error("Không xác định được kiểu câu lệnh");
    } catch (std::exception& e) {
        throw; // FIXME: Rethrow for now
    }
}