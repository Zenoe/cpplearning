cmake_minimum_required(VERSION 3.12)
# declare a cmake project,  sets the variable PROJECT_NAME
project(MyCppRootProject)
# Set global options for all subprojects (optional)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
#
# Add subdirectories containing their own CMakeLists.txt
add_subdirectory(common)
add_subdirectory(fd)
