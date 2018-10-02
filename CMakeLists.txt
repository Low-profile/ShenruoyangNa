cmake_minimum_required(VERSION 2.8.8)
project(IntervalBitwise)

find_package(FLEX)
message(STATUS "Found FLEX ${FLEX_VERSION}")

set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

FLEX_TARGET(lexer scanner.l lexer.cpp)
add_executable(parser parser.cpp lexer.cpp ast.cpp stmt.cpp expr.cpp)


find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(parser ${llvm_libs})
