cmake_minimum_required(VERSION 3.10)
project(cpp_multithreading_lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(multithreaded_ds
    src/hazard_pointer.cpp
)

add_executable(test_hazard_pointer tests/test_hazard_pointer.cpp)
target_link_libraries(test_hazard_pointer multithreaded_ds pthread)

enable_testing()
add_test(NAME test_hazard_pointer COMMAND test_hazard_pointer) 