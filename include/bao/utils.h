//
// Created by doqin on 13/05/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <exception>
#include <bao/lexer/token.h>
#include <utility>
#include <vector>
#include <bao/filereader/reader.h>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <format>
#include <sstream>
#include <llvm/IR/IRBuilder.h>

namespace bao::mir {
    struct Value;
    struct Instruction;
}

namespace bao::mir {
    struct BasicBlock;
}

namespace bao {
    namespace mir {
        struct Function;
        struct Module;
    }

    class Type;
}

namespace bao::ast {
    class NumLitExpr;
    class StmtNode;
    struct Program;
    class FuncNode;
    class ExprNode;
}

namespace fs = std::filesystem;
using std::exception;
using std::exception_ptr;
using std::vector;
using std::ostringstream;

namespace bao::utils {
    // -- Helper functions ---
    /**
     * Helper function to match program argument
     * @param argc Program argument count
     * @param argv Program argument variables
     * @param target C_string target argument
     * @return Boolean value whether it contains it or not
     */
    bool arg_contains(int argc, char *argv[], const char *target);

    /**
     * Helper function to print how to use the compiler
     */
    void print_usage();

    /**
     * Helper function to print tokens
     * @param token The tokens from the lexer
     */
    void print_token(const Token &token);

    namespace ast {
        /**
         * Helper function to print the program
         * @param program The program to print
         */
        void print_program(const bao::ast::Program& program);

        void print_function(const bao::ast::FuncNode& func, const string &padding);

        void print_statement(bao::ast::StmtNode* stmt, const string &padding);

        void print_expression(bao::ast::ExprNode* expr, const string &padding);
    }

    namespace mir {
        void print_module(const bao::mir::Module& module);

        void print_function(const bao::mir::Function& func, const string &padding);

        void print_block(const bao::mir::BasicBlock& block, const string &padding);

        void print_instruction(const bao::mir::Instruction* inst, const string &padding);

        void print_value(const bao::mir::Value& value, const string &padding);
    }

    /**
     * Helper function for matching a value against a set of patterns
     * @param val Value to match
     * @param pattern Pattern to match against
     * @param default_case In case of no match, this function will be called
     */
    void match(
        const string &val,
        const std::unordered_map<string, std::function<void()> > &pattern,
        const std::function<void()> &default_case);

    // --- Error list class ---
    /**
     * Helper class for making a list of errors
     */
    class ErrorList final : public exception {
        vector<exception_ptr> exceptions;
        mutable string message;

    public:
        explicit ErrorList(const vector<exception_ptr> &exceptions) : exceptions(exceptions) {
        }

        [[nodiscard]] const char *what() const noexcept override {
            ostringstream oss;
            // oss << "Số lỗi phát hiện: " << this->exceptions.size() << "\n\n";
            for (const auto & i : exceptions) {
                try {
                    if (i) {
                        std::rethrow_exception(i);
                    }
                } catch (const std::exception& e) {
                    oss << e.what() << "\n\n";
                }
            }
            this->message = oss.str();
            return this->message.c_str();
        }

        [[nodiscard]] const vector<exception_ptr> &get_exceptions() const {
            return this->exceptions;
        }
    };

    // --- Compiler error class ---
    /**
     * Helper class for making a compiler error
     */
    class CompilerError final : public exception {
        string message;
        int line, column;
    public:
        explicit CompilerError(
            string message,
            string filepath,
            const int line,
            const int column
        ): line(line),
           column(column) {
            const Reader reader(std::move(filepath));
            string content = reader.get_line(line);
            string liner(std::ranges::max(column - 1, 0), '~');
            this->message = std::format("{}\n\033[32m{}^\033[0m\n[Dòng {}, Cột {}] {}", content, liner, line, column, message);
        }

        [[nodiscard]] const char* what() const noexcept override {
            return this->message.c_str();
        }

        /**
         *  Helper function for creating new compiler errors
         * @param file Name of source file to preview
         * @param dir Directory of source file
         * @param message Error message
         * @param line Line of source file
         * @param column Column of source file
         * @return A CompilerError object to throw
         */
        static CompilerError new_error(const string &file, const string &dir, string message, const int line,
                                       const int column) {
            const fs::path filename = file;
            const fs::path directory = dir;
            const fs::path fullpath = directory / filename;
            return CompilerError(std::move(message), fullpath.string(), line, column);
        }
    };

    /**
     * Helper function for padding strings
     * @param input String to add padding
     * @param padding Kind of padding you want
     * @return Padded string
     */
    string pad_lines(const string& input, const string& padding);

    /**
     * Helper function for creating an exception_ptr
     * @tparam ExceptionType Exception's type
     * @param ex Exception to create pointer from
     * @return Exception pointer
     */
    template<typename ExceptionType>
    std::exception_ptr make_exception_ptr(const ExceptionType& ex) {
        try {
            throw ex;
        } catch (...) {
            return std::current_exception();
        }
    }

    bool is_literal(bao::ast::ExprNode* expr);
    bool can_cast_literal(const bao::ast::NumLitExpr* expr, const Type* type);
    void cast_literal(bao::ast::NumLitExpr* expr, Type* type);

    llvm::Type* get_llvm_type(llvm::IRBuilder<>& builder, bao::Type* type);
    llvm::Value* get_llvm_value(llvm::IRBuilder<>& builder, bao::mir::Value& mir_value);

    std::string type_to_string(Type* type);

    std::unique_ptr<Type> clone_type(Type* type);
}

#endif //UTILS_H
