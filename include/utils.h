//
// Created by doqin on 13/05/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <lexer/token.h>
#include <vector>

using std::exception;
using std::vector;

namespace bao::utils {
    bool arg_contains(int argc, char *argv[], const char *target);
    void print_usage();
    void print_token(const Token &token);

    class ErrorList final : public exception {
        mutable string error_buffer;
        vector<exception> errors;
    public:
        explicit ErrorList(const vector<exception>& errors) : errors(errors) {}
        [[nodiscard]] const vector<exception>& get_errors() const {
            return this->errors;
        }
        [[nodiscard]] const char* what() const noexcept override {
            for (const auto& error : this->errors) {
                this->error_buffer += string(error.what()) + string("\n");
            }
            return this->error_buffer.c_str();
        }
    };
}

#endif //UTILS_H
