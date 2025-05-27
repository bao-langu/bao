cmake_minimum_required(VERSION 3.28)
project(baoc)
set(CMAKE_CXX_STANDARD 23)

# For people not using CLion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
        src/codegen/generator.cpp
)

# ICU stuff
# Find ICU (request common components)
find_package(ICU REQUIRED COMPONENTS i18n uc)

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE ICU::i18n ICU::uc
)

# LLVM's dependency
find_package(zstd CONFIG REQUIRED)
if(NOT TARGET zstd::libzstd_shared AND TARGET zstd::libzstd)
    add_library(zstd::libzstd_shared ALIAS zstd::libzstd)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE zstd::libzstd)

if(WIN32)
    set(LLVM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/llvm/lib/cmake/llvm)
endif()

# Get LLVM
find_package(LLVM REQUIRED CONFIG)
# Check if LLVM was found
if(NOT LLVM_FOUND)
    message(FATAL_ERROR "LLVM not found!")
endif()
message(STATUS "Found LLVM ${LLVM_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_CMAKE_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dirs: ${LLVM_LIBRARY_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})
target_include_directories(baoc PRIVATE ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Map stuff
llvm_map_components_to_libraries(LLVM_LIBS
    core
    support
    irreader 
    asmparser
    analysis
    bitwriter
    codegen
    executionengine
    mc
    target
    transformutils
    native
)
# Link the damn library
target_link_libraries(baoc PRIVATE ${LLVM_LIBS})

# Get header files
target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE include
)