cmake_minimum_required(VERSION 3.5)

project(console_bridge_vendor)

find_package(ament_cmake REQUIRED)

macro(build_console_bridge)
  set(extra_cmake_args)
  if(DEFINED CMAKE_BUILD_TYPE)
    list(APPEND extra_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
  endif()

  include(ExternalProject)
  # Get console_bridge 0.4.0 + 2 patches:
  # 4a52e5dfe6b8301bc18e472ad585ea216f3762f7: explicitly enable CMake policy 42 to prevent warning
  # ad25f7307da76be2857545e7e5c2a20727eee542: use new version of gtest to fix windows warnings
  ExternalProject_Add(console_bridge-ad25f73
    URL https://github.com/ros/console_bridge/archive/ad25f7307da76be2857545e7e5c2a20727eee542.tar.gz
    URL_MD5 5a7ab572593e2483ab1b08154c2af0b6
    TIMEOUT 600
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_install
      ${extra_cmake_args}
      -Wno-dev
  )

  # The external project will install to the build folder, but we'll install that on make install.
  install(
    DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_install/
    DESTINATION
      ${CMAKE_INSTALL_PREFIX}
  )
endmacro()

find_package(console_bridge QUIET)

if(NOT console_bridge_FOUND OR "${console_bridge_VERSION}" VERSION_LESS 0.4.0)
  build_console_bridge()
endif()

# this ensures that the package has an environment hook setting the PATH
ament_package()
