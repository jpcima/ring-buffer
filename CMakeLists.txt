cmake_minimum_required(VERSION 3.6)

project(ring_buffer LANGUAGES CXX)

find_package(Threads REQUIRED)

if("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(ring_buffer_CXX_STANDARD "17")
elseif("cxx_std_14" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(ring_buffer_CXX_STANDARD "14")
else()
  message(FATAL_ERROR "at least C++14 support is required")
endif()
set(CMAKE_CXX_STANDARD "${ring_buffer_CXX_STANDARD}")

add_library(ring_buffer STATIC "sources/ring_buffer.cc")
target_include_directories(ring_buffer PUBLIC "include")
target_link_libraries(ring_buffer PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if(ring_buffer_CXX_STANDARD LESS "17")
  target_compile_definitions(ring_buffer PUBLIC "RING_BUFFER_NO_STD_SHARED_MUTEX=1")
  find_package(Boost COMPONENTS thread system REQUIRED)
  target_include_directories(ring_buffer PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(ring_buffer PUBLIC "${Boost_THREAD_LIBRARY}" "${Boost_SYSTEM_LIBRARY}")
endif()

add_executable(ring_buffer_test EXCLUDE_FROM_ALL "tests/main.cc")
target_link_libraries(ring_buffer_test PRIVATE ring_buffer)
