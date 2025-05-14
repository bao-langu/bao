//
// Created by doqin on 13/05/2025.
//
#include <bao/filereader/reader.h>
#include <utility>
#include <fstream>
#include <filesystem>
#include <format>
#include <sstream>
#include <unicode/utypes.h>
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>

namespace fs = std::filesystem;
using std::stringstream;
using std::fstream;
using std::istreambuf_iterator;
using std::runtime_error;
using std::invalid_argument;
using std::format;
using icu::Normalizer2;
using icu::UnicodeString;
using std::out_of_range;

bao::Reader::Reader(string path) {
    this->path = std::move(path);
}

string bao::Reader::read() const {
    // --- Get the full src path ---
    fs::path curr_dir = fs::current_path(); // Work directory
    fs::path src_path = path; // Src path input
    fs::path full_src_path; // Final path

    // In case src path is already absolute
    if (src_path.is_absolute()) {
        full_src_path = src_path;
    } else {
        full_src_path = curr_dir / src_path;
    }

    // Make sure it's valid
    if (!fs::exists(full_src_path)) {
        throw invalid_argument(
            format("Lỗi nội bộ: tệp không tồn tại: {}", full_src_path.string()));
    }

    // Content from file
    stringstream buffer;
    fstream file(full_src_path);
    if (!file) {
        throw runtime_error(
            format("Lỗi nội bộ: gặp sự cố đọc tệp: {}", full_src_path.string()));
    }

    // Gets the raw string
    string raw((istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    string contents = normalize(raw);

    file.close(); // Good practice
    return contents;
}

string bao::Reader::get_line(int target_line) const {
    // --- Get the full src path ---
    fs::path curr_dir = fs::current_path(); // Work directory
    fs::path src_path = path; // Src path input
    fs::path full_src_path; // Final path

    // In case src path is already absolute
    if (src_path.is_absolute()) {
        full_src_path = src_path;
    } else {
        full_src_path = curr_dir / src_path;
    }

    // Make sure it's valid
    if (!fs::exists(full_src_path)) {
        throw invalid_argument(
            format("Lỗi: tệp không tồn tại: {}", full_src_path.string()));
    }

    int current_line = 1;
    string line;
    fstream file(full_src_path);
    if (!file) {
        throw runtime_error(
            format("Lỗi nội bộ: gặp sự cố đọc tệp: {}", full_src_path.string()));
    }

    while (getline(file, line)) {
        if (current_line == target_line) {
            return normalize(line);
        }
        current_line++;
    }

    throw out_of_range("Lỗi: dòng nằm ngoài số dòng của tệp");
}

string bao::Reader::normalize(const string& content) {
    // Error code object
    UErrorCode errorCode = U_ZERO_ERROR;

    // Get a pointer to the Normalizer2 instance for NFC normalization
    const Normalizer2* normalizer = Normalizer2::getNFCInstance(errorCode);
    if (U_FAILURE(errorCode)) {
        throw runtime_error(
            format("Lỗi nội bộ: Gặp sự cố kiếm đối tượng Normalizer2: {}",
                u_errorName(errorCode)));
    }

    const UnicodeString source = UnicodeString::fromUTF8(content);
    const UnicodeString normalized = normalizer->normalize(source, errorCode);
    if (U_FAILURE(errorCode)) {
        throw runtime_error(
        format("Lỗi nội bộ: Gặp sự cố đồng bộ hóa chuỗi: {}",
            u_errorName(errorCode)));
    }

    string normalizedUTF8;
    normalized.toUTF8String(normalizedUTF8);

    return normalizedUTF8;
}
