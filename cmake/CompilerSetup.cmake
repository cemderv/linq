
#
# Enable AddressSanitizer
#
if (LINQ_ENABLE_ADDRESS_SANITIZER)
  message(STATUS "Enabling AddressSanitizer globally")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()


#
# Enables common warnings, treats warnings as errors, ...
#
function(enable_sane_compiler_flags targetName)
  message(STATUS "Enabling sane compiler flags for ${targetName}")

  if (MSVC)
    target_compile_options(${targetName} PRIVATE
      /W4     # Warning level 4
      /WX     # Treat warnings as errors
      /wd4702 # Unreachable code
    )
  else ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(IS_GCC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(IS_CLANG TRUE)
    endif ()

    if (IS_GCC OR IS_CLANG)
      target_compile_options(${targetName} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -pedantic-errors
        -Werror
        -Wimplicit-fallthrough
        -Wformat
        -Werror=format-security
      )
    endif ()
  endif ()
endfunction()


#
# Enables C++ hardening flags for a target.
#
function(enable_hardening_compiler_flags targetName)
  message(STATUS "Enabling hardening flags for ${targetName}")

  if (MSVC)
    target_compile_definitions(${targetName} PRIVATE
      -D_ITERATOR_DEBUG_LEVEL=2 # Enable checked iterators in the STL
    )
  else ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(IS_GCC TRUE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(IS_CLANG TRUE)
    endif ()

    if (IS_GCC OR IS_CLANG)
      # AddressSanitizer misbehaves when FORTIFY_SOURCE is enabled.
      # Only fortify source when we're not going to use AddressSanitizer anyway.
      if (NOT LINQ_ENABLE_ADDRESS_SANITIZER)
        target_compile_definitions(${targetName} PRIVATE
          -D_FORTIFY_SOURCE=3
          -D_GLIBCXX_ASSERTIONS
        )
      endif ()

      target_compile_options(${targetName} PRIVATE
        -fno-delete-null-pointer-checks
        -fno-strict-overflow
        -fno-strict-aliasing
      )
    endif ()
  endif ()
endfunction()


#
# Enables cppcheck checks for a target.
#
function(enable_cppcheck targetName)
  message(STATUS "Enabling cppcheck checks for ${targetName}")
  find_program(CPPCHECK_EXE NAMES "cppcheck")

  if (CPPCHECK_EXE)
    set(CPPCHECK_CMD ${CPPCHECK_EXE})
    list(APPEND CPPCHECK_CMD
      "--enable=warning"
      "--inconclusive"
      "--force"
      "--inline-suppr"
    )
    set_target_properties(${targetName} PROPERTIES CXX_CPPCHECK "${CPPCHECK_CMD}")
  else ()
    message(WARNING "cppcheck was not found; if you want cppcheck checks, please install it first")
  endif ()
endfunction()


#
# Enables all configured flags and checks for a target.
#
function(setup_compiler_for targetName)
  enable_sane_compiler_flags(${targetName})

  if (LINQ_ENABLE_HARDENING)
    enable_hardening_compiler_flags(${targetName})
  endif ()

  if (LINQ_ENABLE_CPPCHECK)
    enable_cppcheck(${targetName})
  endif ()
endfunction()
