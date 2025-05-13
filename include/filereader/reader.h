//
// Created by doqin on 13/05/2025.
//

#ifndef READER_H
#define READER_H
#include <string>

using std::string;

namespace bao {
    class Reader {
        string path;
    public:
        /**
         *
         * @param path The source file path
         */
        explicit Reader(string path);

        // [[nodiscard]] makes you have to use the return value
        /**
         *
         * @return The content of the file
         */
        [[nodiscard]] string read() const;

    private:
        /**
         *
         * @param content The string content that needs to be normalized
         * @return The normalized string
         */
        [[nodiscard]] static string normalize(const string& content) ;
    };
}
#endif //READER_H
