//
// Created by doqin on 13/05/2025.
//
#include <bao/utils.h>
#include <iostream>
#include <bao/lexer/maps.h>
#include <cstring>

using std::cout;
using std::endl;

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

void bao::utils::print_program(const Program &program) {
    cout << "Nội dung chương trình:" << endl;
    cout << "Tên: " << program.name << endl;
    cout << "Đường dẫn: " << program.path << endl;
    cout << "Số hàm: " << program.funcs.size() << endl;
    for (const auto &func : program.funcs) {
        auto [line, column] = func.pos();
        cout << std::format(
            "Hàm: {} -> {} (Dòng {}, Cột {})\n",
            func.get_name(), func.get_return_type().get_name(),
            line, column);
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

