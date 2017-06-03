cmake_minimum_required(VERSION 2.8.3)

project(poco_vendor)

set(PACKAGE_VERSION "1.0.0")

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(Poco COMPONENTS Foundation)

if(NOT Poco_FOUND)
  # If Poco was not found, download and build from source
  set(extra_cmake_args)
  if(DEFINED CMAKE_BUILD_TYPE)
    list(APPEND extra_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
  endif()
  if(WIN32)
    list(APPEND extra_cmake_args "-DCMAKE_CXX_FLAGS=/wd4244 /wd4530 /wd4577")
  else()
    list(APPEND extra_cmake_args "-DCMAKE_C_FLAGS=-Wno-shift-negative-value")
    list(APPEND extra_cmake_args "-DCMAKE_CXX_FLAGS=-std=c++14")
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT ANDROID)
      list(APPEND extra_cmake_args "-DPOCO_UNBUNDLED:BOOL=ON")
    endif()
  endif()
  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    list(APPEND extra_cmake_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    if(ANDROID)
      if(DEFINED ANDROID_ABI)
        list(APPEND extra_cmake_args "-DANDROID_ABI=${ANDROID_ABI}")
      endif()
      if(DEFINED ANDROID_CPP_FEATURES)
        list(APPEND extra_cmake_args "-DANDROID_CPP_FEATURES=${ANDROID_CPP_FEATURES}")
      endif()
      if(DEFINED ANDROID_FUNCTION_LEVEL_LINKING)
        list(APPEND extra_cmake_args "-DANDROID_FUNCTION_LEVEL_LINKING=${ANDROID_FUNCTION_LEVEL_LINKING}")
      endif()
      if(DEFINED ANDROID_NATIVE_API_LEVEL)
        list(APPEND extra_cmake_args "-DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}")
      endif()
      if(DEFINED ANDROID_NDK)
        list(APPEND extra_cmake_args "-DANDROID_NDK=${ANDROID_NDK}")
      endif()
      if(DEFINED ANDROID_STL)
        list(APPEND extra_cmake_args "-DANDROID_STL=${ANDROID_STL}")
      endif()
      if(DEFINED ANDROID_TOOLCHAIN_NAME)
        list(APPEND extra_cmake_args "-DANDROID_TOOLCHAIN_NAME=${ANDROID_TOOLCHAIN_NAME}")
      endif()
    endif()
  endif()
  include(ExternalProject)
  ExternalProject_Add(poco-1.7.7
    URL https://github.com/pocoproject/poco/archive/poco-1.7.7-release.tar.gz
    URL_MD5 247b97b545715dc38c8619e412fbcd96
    TIMEOUT 600
    CMAKE_ARGS
      -DENABLE_CPPUNIT:BOOL=OFF
      -DENABLE_CRYPTO:BOOL=OFF
      -DENABLE_DATA:BOOL=OFF
      -DENABLE_JSON:BOOL=OFF
      -DENABLE_MONGODB:BOOL=OFF
      -DENABLE_NET:BOOL=OFF
      -DENABLE_NETSSL:BOOL=OFF
      -DENABLE_PAGECOMPILER_FILE2PAGE:BOOL=OFF
      -DENABLE_PAGECOMPILER:BOOL=OFF
      -DENABLE_REDIS:BOOL=OFF
      -DENABLE_UTIL:BOOL=OFF
      -DENABLE_XML:BOOL=OFF
      -DENABLE_ZIP:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/poco_install
      ${extra_cmake_args}
      -Wno-dev
  )

  # The external project will install to the build folder, but we'll install that on make install.
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/poco_install/ DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

configure_file(poco_vendorConfig.cmake.in "${PROJECT_BINARY_DIR}/poco_vendorConfig.cmake" @ONLY)
configure_file(poco_vendorConfig-version.cmake.in "${PROJECT_BINARY_DIR}/poco_vendorConfig-version.cmake" @ONLY)

install(DIRECTORY cmake DESTINATION share/${PROJECT_NAME})

install(FILES
  "${PROJECT_BINARY_DIR}/poco_vendorConfig.cmake"
  "${PROJECT_BINARY_DIR}/poco_vendorConfig-version.cmake"
  DESTINATION share/${PROJECT_NAME}/cmake)
