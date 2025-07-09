cmake_minimum_required(VERSION 3.12)
project(cppfd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# for all targets, not modern, discouraged
# include_directories(/opt/cpp/include)

# For strict warnings: add_compile_options can be used globally
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)

# 1. fd2.cpp -> cpp_fd2 (needs re2 and stdc++fs)
add_executable(cpp_fd2 src/fd2.cpp)
target_link_libraries(cpp_fd2 stdc++fs re2)

# 2. fd.cpp -> cpp_fd (needs stdc++fs only)
add_executable(cpp_fd src/fd.cpp)
target_link_libraries(cpp_fd stdc++fs)

# 3. finder.cpp -> finder (no extra libs)
add_executable(finder src/finder.cpp)
# add include path per target
target_include_directories(finder PRIVATE /opt/cpp/include)
# If you want strict warnings for this only:
# target_compile_options(finder PRIVATE -Wall -Wextra -Wpedantic -Wshadow)


add_executable(cpp_fd3 src/fd3.cpp)
# target_include_directories(cpp_fd3 PRIVATE /opt/cpp/include)
# target_link_libraries(cpp_fd3 PRIVATE common re2 pthread )

target_include_directories(cpp_fd3 PRIVATE /opt/cpp/common)

add_executable(cpp_fd4 src/fd4.cpp)
target_link_libraries(cpp_fd4 re2 pthread)
