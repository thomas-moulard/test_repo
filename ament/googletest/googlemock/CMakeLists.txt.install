project(gmock CXX C)
cmake_minimum_required(VERSION 2.6.2)

find_package(gtest_vendor REQUIRED)

find_package(Threads)

include_directories(
  include
  .  # to find the source files included with "src/gmock*.cc"
  ${gtest_vendor_BASE_DIR}/include
  ${gtest_vendor_BASE_DIR}
)

add_library(gmock STATIC
  ${gtest_vendor_BASE_DIR}/src/gtest-all.cc
  src/gmock-all.cc)
target_link_libraries(gmock ${CMAKE_THREAD_LIBS_INIT})
if(NOT WIN32)
  set_target_properties(gmock PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
endif()

add_library(gmock_main STATIC src/gmock_main.cc)
