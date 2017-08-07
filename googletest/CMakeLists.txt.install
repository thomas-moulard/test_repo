project(gtest CXX C)
cmake_minimum_required(VERSION 2.6.2)

find_package(Threads)

include_directories(
  include
  .  # to find the source files included with "src/gtest*.cc"
)

add_library(gtest STATIC src/gtest-all.cc)
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})
if(NOT WIN32)
  set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
endif()

add_library(gtest_main STATIC src/gtest_main.cc)
