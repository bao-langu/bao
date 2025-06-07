#include <bao/tests/test.h>
#include <fstream>
#include <iostream>
#include <cstdint>

typedef uint8_t BYTE;

int bytecode_test() {
    std::ofstream file("test/bytecode_test.goi", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    BYTE data[] = { 0xBA, 0x0B, 0xA0, 0xBA };
    
    BYTE version[4];
    for (int i = 0; i < 4; i++) {
        version[i] = (BAO_VERSION >> (8 * 3 - i * 8)) & 0xFF;
    }

    for (int i = 0; i < 4; i++) {
        std::cout << "0x" << std::hex << (int)version[i] << " ";
    }
    std::cout << '\n';

    file.write(reinterpret_cast<const char*>(data), sizeof(data));
    file.write(reinterpret_cast<const char*>(version), sizeof(version));
    file.close();
    return 0;
}