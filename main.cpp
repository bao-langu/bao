#include <stdexcept>
#include <bao/tests/test.h>
#include <bao/common/utils.h>

// --- Main program ---
int main(const int argc, char *argv[]) {

    if (argc < 2) {
        throw std::invalid_argument("Cú pháp: baoc [--test] [--huong-dan]");
    }
    if (bao::utils::arg_contains(argc, argv, "--huong-dan")) {
        bao::utils::print_usage();
    }
    if (bao::utils::arg_contains(argc, argv, "--test")) {
        if (test(argc, argv) != 0) {
            throw std::runtime_error("test failed");
        }
    }
    return 0;
}