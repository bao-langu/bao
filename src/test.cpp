//
// Created by doqin on 13/05/2025.
//
#include <bao/test.h>

// --- Included libraries ---
#include <iostream>
#include <string>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/utypes.h>
#include <unicode/schriter.h>
#include <bao/filereader/reader.h>
#include <bao/utils.h>
#include <bao/lexer/lexer.h>
#include <bao/parser/parser.h>
#include <bao/sema/analyzer.h>

// --- Using types ---
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::u8string;
using std::exception;
using icu::Normalizer2;
using icu::UnicodeString;
using icu::StringCharacterIterator;

// --- Test functions ---
void semanticsTest();
void parserTest();
void lexerTest();
void readerTest();
int icuTest();

// Main test function
int test(int argc, char* argv[]) {
    semanticsTest();
    return 0;
}

void semanticsTest() {
    try {
        const bao::Reader reader("test.bao");
        const string source = reader.read();
        cout << "Nội dung tệp nguồn:" << endl;
        cout << source << endl;
        cout << "Đang phân loại ký hiệu..." << endl;
        bao::Lexer lexer(source);
        lexer.tokenize();
        vector<bao::Token> tokens = lexer.get_tokens();
        for (const auto& token : tokens) {
            bao::utils::print_token(token);
        }
        cout << "Đang phân tích cú pháp..." << endl;
        bao::Parser parser("test.bao", ".", tokens);
        bao::ast::Program program = std::move(parser.parse_program());
        cout << "Phân tích cú pháp thành công!" << endl;
        bao::utils::print_program(program);
        cout << "Đang phân tích ngữ nghĩa..." << endl;
        bao::Analyzer analyzer(std::move(program));
        program = std::move(analyzer.analyze_program());
        cout << "Phân tích ngữ nghĩa thành công!" << endl;
        bao::utils::print_program(program);
    } catch (const exception& e) {
        cerr << "Gặp sự cố trong quá trình kiểm tra ngữ nghĩa:\n" << endl << e.what() << endl;
    }
}

// Test the parser
void parserTest() {
    try {
        const bao::Reader reader("test.bao");
        const string source = reader.read();
        cout << "Nội dung tệp nguồn:" << endl;
        cout << source << endl;
        cout << "Đang phân loại ký hiệu..." << endl;
        bao::Lexer lexer(source);
        lexer.tokenize();
        vector<bao::Token> tokens = lexer.get_tokens();
        for (const auto& token : tokens) {
            bao::utils::print_token(token);
        }
        cout << "Đang phân tích cú pháp..." << endl;
        bao::Parser parser("test.bao", ".", tokens);
        const bao::ast::Program& program = parser.parse_program();
        cout << "Phân tích cú pháp thành công!" << endl;
        bao::utils::print_program(program);
    } catch (const exception& e) {
        cerr << "Gặp sự cố trong quá trình phân tích cú pháp:\n" << endl << e.what() << endl;
    }
}

// Test the lexer
void lexerTest() {
    try {
        const bao::Reader reader("test.bao");
        const string source = reader.read();
        cout << "Nội dung tệp nguồn:" << endl;
        cout << source << endl;
        cout << "Đang phân tích cú pháp..." << endl;
        bao::Lexer lexer(source);
        lexer.tokenize();
        for (const auto tokens = lexer.get_tokens(); const auto& token : tokens) {
            bao::utils::print_token(token);
        }
    } catch (exception& e) {
        cerr << "Gặp sự cố trong quá trình phân tích cú pháp:" << endl << e.what() << endl;
    }
}

// Test the file reader
void readerTest() {
    const bao::Reader reader("test.bao");
    try {
        const string content = reader.read();
        cout << "Full content:" << endl;
        cout << content << endl;
        cout << "Per character:" << endl;
        // Convert to Unicode UTF-32
        const UnicodeString ustr = UnicodeString::fromUTF8(content);
        StringCharacterIterator it(ustr);
        for (it.setToStart(); it.hasNext(); it.next32()) {
            const auto c = UnicodeString(it.current32());
            string utf8str;
            c.toUTF8String(utf8str);
            cout << utf8str << " ";
        }
        cout << endl;
    } catch (exception& e) {
        cerr << "Gặp sự cố đọc tệp nguồn:" << endl << e.what() << endl;
    }
}

int icuTest() {
    // Error code object
    UErrorCode errorCode = U_ZERO_ERROR;

    // Get a pointer to a Normalizer2 instance for NFC normalization.
    const Normalizer2* normalizer = Normalizer2::getNFCInstance(errorCode);
    if (U_FAILURE(errorCode)) {
        cerr << "Error obtaining Normalizer2 instance: " << u_errorName(errorCode) << endl;
        return 1;
    }

    // Example input: it could be any UTF-8 encoded string
    const u8string inputUTF8 = u8"a\u030A"; // 'a' followed by a combining ring above
    const UnicodeString source = UnicodeString::fromUTF8(inputUTF8);

    const UnicodeString normalized = normalizer->normalize(source, errorCode);
    if (U_FAILURE(errorCode)) {
        cerr << "Normalization failed: " << u_errorName(errorCode) << endl;
        return 1;
    }

    // Convert the normalized UnicodeString back to UTF-8.
    string normalizedUTF8;
    normalized.toUTF8String(normalizedUTF8);

    cout << "Normalized string: " << normalizedUTF8 << endl;
    return 0;
}