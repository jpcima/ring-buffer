cmake_minimum_required(VERSION 3.5)

project(ring_buffer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD "17")

find_package(Threads REQUIRED)

add_library(ring_buffer STATIC "sources/ring_buffer.cc")
target_include_directories(ring_buffer PUBLIC "include")
target_link_libraries(ring_buffer PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_executable(ring_buffer_test EXCLUDE_FROM_ALL "tests/main.cc")
target_link_libraries(ring_buffer_test PRIVATE ring_buffer)
