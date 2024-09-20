# Copyright (C) 2015-2024 Cemalettin Dervis

cmake_minimum_required(VERSION 3.15)

project(linq
  DESCRIPTION  "A header-only LINQ library for C++"
  HOMEPAGE_URL "https://github.com/cemderv/linq"
  LANGUAGES    CXX
  VERSION      0.7.0
)

option(LINQ_BUILD_TESTS "Build linq unit tests" OFF)

add_library(linq INTERFACE)

target_sources(linq
  INTERFACE
  FILE_SET
    HEADERS
      BASE_DIRS include/
      FILES include/linq.hpp
)

target_sources(linq
  PRIVATE
    include/linq.hpp
)

target_compile_features(linq INTERFACE cxx_std_11)

if (LINQ_BUILD_TESTS)
  add_subdirectory(tests)
endif ()

