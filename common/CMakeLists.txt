# common/CMakeLists.txt
add_library(common STATIC gutils.cpp zfs.cpp)

# if it's not a library
# add_executable(common glob_utils)

# without this line, g_utils.h could not be found in sub project xx.cpp
target_include_directories(common
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common
    PUBLIC stdc++fs    # For older compilers/C++17 (not needed for C++20)
)
