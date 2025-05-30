//
// Created by doqin on 13/05/2025.
//

#include <string>
#include <stdexcept>
#include <bao/lexer/lexer.h>
#include <bao/lexer/sets.h>
#include <unicode/uchar.h>
#include <bao/utils.h>
#include <bao/lexer/maps.h>

#define U_SENTINEL 0xFFFF

using std::out_of_range;

// -- Lexer's constructor --
bao::Lexer :: Lexer(
    const string &source
) : it(UnicodeString::fromUTF8(source)) {
    this->source = UnicodeString::fromUTF8(source);
    this->it.setToStart();
    this->current_line = 1;
    this->current_column = 1;
    this->code_point_index = 0;
}

// -- Lexer's methods --
auto 
is_new_line(
    UChar32 cp
) -> bool;

// Tokenize the source code
void 
bao::Lexer :: tokenize() {
    while (this->current_code_point() != U_SENTINEL) {
        try {
            this->skip_whitespace(); // Skip whitespace characters
            const int line = this->current_line;
            const int column = this->current_column;
            // const int current_code_point_index = this->code_point_index; // Anchor
            const UChar32 current_code_point = this->current_code_point();
            const string current_utf8 = this->current_utf8();
            if (current_code_point == U_SENTINEL) {
                break;
            }

            // Handle identifier
            if (u_isalpha(current_code_point)) {
                this->tokens.push_back(handle_identifier());
                continue;
            }

            // Handle numbers
            if (u_isdigit(current_code_point)) {
                this->tokens.push_back(handle_number());
                continue;
            }

            // Handles symbols
            if (operators.contains(current_utf8)) {
                this->tokens.push_back(handle_symbols());
                continue;
            }

            // Specific tokens
            if (token_map.contains(this->current_utf8())) {
                Token token = token_map.at(this->current_utf8());
                this->next();
                token.line = line;
                token.column = column;
                this->tokens.push_back(token);
                continue;
            }

            // Fall back to the default case
            this->next();
            this->tokens.push_back(Token{TokenType::Unknown, current_utf8, line, column});
        } catch (...) {
            this->exceptions.push_back(std::current_exception());
        }
    }
    // Add an EndOfFile token at the end
    this->tokens.push_back(Token{TokenType::EndOfFile, "\\0", this->current_line, this->current_column});
}

const vector<bao::Token>& 
bao::Lexer :: get_tokens() const {
    if (this->exceptions.empty()) {
        return this->tokens;
    }
    throw utils::ErrorList(this->exceptions);
}

// Get the current code point as a UTF-8 string
std::string 
bao::Lexer :: current_utf8() const {
    const UnicodeString unicode_str(this->it.current32());
    string utf8str;
    unicode_str.toUTF8String(utf8str);
    return utf8str;
}

// Get the current code point as a UChar32
UChar32 
bao::Lexer :: current_code_point() const {
    return this->it.current32();
}

// Get the next code point
void 
bao::Lexer :: next() {
    if (this->it.hasNext()) {
        this->code_point_index++;
        this->it.next32();
        this->current_column++;
    } else {
        // If no code point is available, throw an exception
        throw out_of_range("Lỗi nội bộ: Không còn mã điểm nào để đọc");
    }
}

// Peek at the next code point without advancing the iterator
std::string 
bao::Lexer :: peek() const {
    if (StringCharacterIterator copy(it); copy.hasNext()) {
        const UChar32 code_point = copy.next32();
        string utf8str;
        const UnicodeString unicode_str(code_point);
        unicode_str.toUTF8String(utf8str);
        return utf8str;
    }
    // If no code point is available, throw an exception
    throw out_of_range("Lỗi nội bộ: Không còn mã điểm nào để đọc");
}

// Skip whitespace characters and newlines
void 
bao::Lexer :: skip_whitespace() {
    while (this->it.hasNext() && (this->current_utf8() == " " || is_new_line(this->current_code_point()))) {
        if (is_new_line(this->current_code_point())) {
            this->tokens.push_back(bao::Token{bao::TokenType::Newline, "\\n", this->current_line, this->current_column});
            this->current_line++;
            this->current_column = 0;
        }
        try {
            this->next();
        } catch ([[maybe_unused]] out_of_range &e) {
            throw;
        }
    }
}

// Seek to a specific code point index
void 
bao::Lexer :: seek(
    const int code_point_index
) {
    it.setToStart(); // Reset iterator to the start
    int i;
    for (i = 0; i < code_point_index && it.hasNext(); ++i) {
        it.next32(); // Move to the next code point
    }
    if (i != code_point_index) {
        throw out_of_range("Lỗi nội bộ: Index mã điểm nằm ngoài phạm vi");
    }
}

// Handle identifiers
auto
bao::Lexer :: handle_identifier() -> bao::Token {
    try {
        const int line = this->current_line;
        const int column = this->current_column;
        string identifier;
        // Get first identifier
        while (this->it.hasNext() && (u_isalnum(this->current_code_point()) || this->current_utf8() == "_")) {
            identifier += this->current_utf8();
            this->next();
        }
        while (this->it.hasNext() && this->current_utf8() == " ") {
            this->next();
        }
        // Handles multi-word keywords
        if (u_isalpha(this->current_code_point())) {
            const int anchor = this->code_point_index; // In case this doesn't work out
            const int anchor_line = this->current_line;
            const int anchor_column = this->current_column;
            string second_identifier;
            while (this->it.hasNext() && (u_isalnum(this->current_code_point()) || this->current_utf8() == "_")) {
                second_identifier += this->current_utf8();
                this->next();
            }
            if (keywords.contains(format("{} {}", identifier, second_identifier))) {
                return Token{TokenType::Keyword, format("{} {}", identifier, second_identifier), line, column};
            }
            // Fall back
            this->code_point_index = anchor;
            this->current_line = anchor_line;
            this->current_column = anchor_column;
            this->seek(anchor);
        }

        // Special identifier
        if (identifier == "E") {
            return Token{TokenType::Operator, identifier, line, column};
        }

        // Handles single-word keyword
        if (keywords.contains(identifier)) {
            return Token{TokenType::Keyword, identifier, line, column};
        }

        // Else is just an identifier
        return Token{TokenType::Identifier, identifier, line, column};
    } catch ([[maybe_unused]] out_of_range &e) {
        throw;
    }
}

// Handle numbers
auto
bao::Lexer :: handle_number() -> bao::Token {
    try {
        const int line = this->current_line;
        const int column = this->current_column;
        string number;
        number += this->current_utf8();
        this->next();
        bool is_float = false;
        // Handles both integers and floats
        while (this->it.hasNext() && u_isdigit(this->current_code_point()) || (this->current_utf8() == "." && !is_float)) {
            // If encounters a dot, it becomes a float
            if (this->current_utf8() == ".") {
                is_float = true;
                number += this->current_utf8();
                this->next();
                continue;
            }
            // Otherwise, it's a digit
            number += this->current_utf8();
            this->next();
        }
        return Token{TokenType::Literal, number, line, column};
    } catch ([[maybe_unused]] out_of_range& e) {
        throw;
    }
}

auto 
bao::Lexer :: handle_symbols() -> bao::Token {
    try {
        const int line = this->current_line;
        const int column = this->current_column;
        // Check for double operators
        if (const string double_operator = this->current_utf8() + this->peek(); operators.contains(double_operator)) {
            this->next();
            this->next();
            return Token{TokenType::Operator, double_operator, line, column};
        }

        // Otherwise, it's a single operator
        const string op = this->current_utf8(); // Fixed cuz me is dumb
        this->next();
        return Token{TokenType::Operator, op, line, column};
    } catch ([[maybe_unused]] out_of_range &e) {
        throw;
    }
}

auto 
is_new_line(
    const UChar32 cp
) -> bool {
    return cp == 0x000A; // LF
           // cp == 0x000D || // CR
           // cp == 0x000C || // FF
           // cp == 0x0085 || // NEL
           // cp == 0x2028 || // Line Separator
           // cp == 0x2029;   // Paragraph Separator
}
