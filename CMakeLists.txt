cmake_minimum_required(VERSION 3.16)
project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Applies to every target below - equivalent to -Wall -Wextra on every g++ call
add_compile_options(-Wall -Wextra)

# Optional: enable with  cmake -DENABLE_ASAN=ON ..
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -g)
    add_link_options(-fsanitize=address)
endif()

# The core order book logic, shared by both the demo and the tests.
# Built once here, then reused as a library - this is the CMake equivalent
# of listing orderbook.cpp on every g++ command by hand.
add_library(orderbook_core STATIC
    src/orderbook.cpp
)
target_include_directories(orderbook_core PUBLIC src)

# The interactive demo executable (what main.cpp builds into)
add_executable(orderbook_demo src/main.cpp)
target_link_libraries(orderbook_demo PRIVATE orderbook_core)

# The test executable
add_executable(test_stage2 tests/test_stage2.cpp)
target_link_libraries(test_stage2 PRIVATE orderbook_core)

# Lets you run tests via:  ctest   (after building)
enable_testing()
add_test(NAME stage2_correctness COMMAND test_stage2)