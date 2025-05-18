cmake_minimum_required(VERSION 3.31)
project(baoc)
set(CMAKE_CXX_STANDARD 26)

# For people not using CLion
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# Find ICU (request common components)
find_package(ICU REQUIRED COMPONENTS i18n uc)

# Add source files
add_executable(${CMAKE_PROJECT_NAME}
        main.cpp
        src/test.cpp
        src/utils.cpp
        src/filereader/reader.cpp
        src/lexer/lexer.cpp
        src/parser/parser.cpp
        src/sema/analyzer.cpp
        src/lexer/maps.cpp
        src/mir/translator.cpp
)

# Get header files
target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE include
)

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE ICU::i18n ICU::uc
)