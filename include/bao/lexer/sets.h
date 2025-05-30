//
// Created by doqin on 14/05/2025.
//

#ifndef SYMBOLS_H
#define SYMBOLS_H
#include <unordered_set>
#include <string>

using std::string;

namespace bao {
    inline std::unordered_set<string> operators = {
        "+", "-", "*", "/", "=", "!=", ":=", ">", "<", ">=", "<=", "..", "->", ":"
    };
    inline std::unordered_set<string> keywords = {
        "hàm", "thủ tục", "nếu", "thì", "không thì", "và", "hoặc", "kết thúc", "trả về"
    };
}

#endif //SYMBOLS_H
