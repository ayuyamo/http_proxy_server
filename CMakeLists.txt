cmake_minimum_required(VERSION 3.13)  # CMake version check
project(http_proxy_proj)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard

# Add main.cpp file of the project root directory as a source file
set(SOURCE_FILES simple_proxy_server.cpp)

find_package(CURL REQUIRED)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(simple_proxy_server ${SOURCE_FILES})

target_link_libraries(simple_proxy_server CURL::libcurl)