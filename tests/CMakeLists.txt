enable_testing()

if (NOT TARGET snitch)
  include(FetchContent)

  FetchContent_Declare(
    snitch
    GIT_REPOSITORY https://github.com/snitch-org/snitch.git
    GIT_TAG v1.2.5
    GIT_SHALLOW
  )

  FetchContent_MakeAvailable(snitch)
endif ()

# Unit tests
add_executable(linq_tests)

target_sources(linq_tests PRIVATE tests.cpp)

target_compile_features(linq_tests PRIVATE cxx_std_20)

target_link_libraries(linq_tests
  PRIVATE
  snitch
  linq
)

# Testbed application
add_executable(linq_testbed)

target_sources(linq_testbed PRIVATE testbed.cpp)

target_link_libraries(linq_testbed
  PRIVATE
  linq
)

target_compile_features(linq_testbed PRIVATE cxx_std_17)

setup_compiler_for(linq_tests)
setup_compiler_for(linq_testbed)