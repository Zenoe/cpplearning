cmake_minimum_required(VERSION 3.12)
project(hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For strict warnings: add_compile_options can be used globally
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)

add_executable(hello hello.cpp)
target_link_libraries(hello stdc++fs re2)
target_include_directories(hello PRIVATE /opt/cpp /opt/cpp/include /opt/cpp/common)

add_executable(resource resource_usage.cpp)
target_link_libraries(resource pthread)

add_executable(resource_ramdom resource_usage_random.cpp)
target_link_libraries(resource_ramdom pthread)
