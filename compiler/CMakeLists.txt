cmake_minimum_required(VERSION 3.10)
project(Compiler C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 and SDL2_image
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_image REQUIRED)
if(SDL2_IMAGE_FOUND)
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
endif()

# Enable testing
enable_testing()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files for the library
set(COMPILER_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/ir.cpp
    src/scematic.cpp
    src/graphics.cpp
)

# Create a library from the compiler sources
add_library(compiler_lib ${COMPILER_SOURCES})
target_include_directories(compiler_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(compiler_lib ${SDL2_LIBRARIES} SDL2_image)

# Main executable
add_executable(compiler src/main.cpp)
target_link_libraries(compiler compiler_lib ${SDL2_LIBRARIES} SDL2_image)

# Lexer tests
add_executable(lexer_test test/lexer_test.cpp)
target_link_libraries(lexer_test compiler_lib)
add_test(NAME LexerTest COMMAND lexer_test)

# Parser tests
add_executable(parser_test test/parser_test.cpp)
target_link_libraries(parser_test compiler_lib)
add_test(NAME ParserTest COMMAND parser_test)

# IR tests
add_executable(ir_test test/ir_test.cpp)
target_link_libraries(ir_test compiler_lib)
add_test(NAME IRTest COMMAND ir_test)

# Semantic analysis tests
add_executable(scematic_test test/scematic_test.cpp)
target_link_libraries(scematic_test compiler_lib)
add_test(NAME ScematicTest COMMAND scematic_test)

# Print build information
message(STATUS "Compiler project configured successfully")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Tests enabled with ctest")
