//
// Created by doqin on 13/05/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <bao/lexer/token.h>
#include <utility>
#include <vector>
#include <bao/filereader/reader.h>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <bao/parser/ast.h>
#include <format>

namespace fs = std::filesystem;
using std::exception;
using std::vector;

namespace bao::utils {
    // -- Helper functions ---
    bool arg_contains(int argc, char *argv[], const char *target);

    void print_usage();

    /**
     * Helper function to print tokens
     * @param token The tokens from the lexer
     */
    void print_token(const Token &token);

    /**
     * Helper function to print the program
     * @param program The program to print
     */
    void print_program(const Program &program);

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
        mutable string error_buffer;
        vector<exception> errors;

    public:
        explicit ErrorList(const vector<exception> &errors) : errors(errors) {
        }

        [[nodiscard]] const vector<exception> &get_errors() const {
            return this->errors;
        }

        [[nodiscard]] const char *what() const noexcept override {
            for (const auto &error: this->errors) {
                this->error_buffer += string(error.what()) + string("\n");
            }
            return this->error_buffer.c_str();
        }
    };

    // --- Compiler error class ---
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
            this->message = std::format("{}\n[Dòng {}, Cột {}] {}", content, line, column, message);
        }

        [[nodiscard]] const char *what() const noexcept override {
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
            const fs::path directory = file;
            const fs::path filename = dir;
            const fs::path fullpath = directory / filename;
            return CompilerError(std::move(message), fullpath.string(), line, column);
        }
    };
}

#endif //UTILS_H
