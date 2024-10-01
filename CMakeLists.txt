# Copyright (C) 2015-2024 Cemalettin Dervis

cmake_minimum_required(VERSION 3.15)

project(linq
  DESCRIPTION "A header-only LINQ library for C++"
  HOMEPAGE_URL "https://github.com/cemderv/linq"
  LANGUAGES CXX
  VERSION 0.8.0
)

# Options
option(LINQ_BUILD_TESTS "Build linq unit tests" OFF)
option(LINQ_ENABLE_CPPCHECK "Enable additional checks using cppcheck?" OFF)
option(LINQ_ENABLE_HARDENING "Enable C++ compiler hardening flags?" OFF)
option(LINQ_ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer?" OFF)

include(cmake/CompilerSetup.cmake)

add_library(linq INTERFACE)
target_include_directories(linq INTERFACE include)
target_compile_features(linq INTERFACE cxx_std_17)

if (LINQ_BUILD_TESTS)
  add_subdirectory(tests)
endif ()

