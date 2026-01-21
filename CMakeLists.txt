cmake_minimum_required(VERSION 3.20)
project(typeinfo_reproducer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# Option to enable/disable the strcmp workaround
option(USE_STRCMP_FIX "Use strcmp workaround for typeinfo comparison" OFF)

if(USE_STRCMP_FIX)
  add_compile_definitions(USE_STRCMP_FIX)
  message(STATUS "Building WITH strcmp workaround")
else()
  message(STATUS "Building WITHOUT strcmp workaround (will demonstrate bug)")
endif()

# Library 1: Store library with default visibility
# Mimics libparthenon.dylib
add_library(store SHARED libstore.cpp)
target_compile_options(store PRIVATE -fvisibility=default)
target_include_directories(store PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Library 2: Fetch library with HIDDEN visibility
# Mimics Python extension (pyKamayan.so) built with nanobind
add_library(fetch SHARED libfetch.cpp)
target_compile_options(fetch PRIVATE -fvisibility=hidden)
target_include_directories(fetch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Main test executable
add_executable(test_typeinfo main.cpp)
target_link_libraries(test_typeinfo PRIVATE store fetch)
target_include_directories(test_typeinfo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add a test target that runs both versions
enable_testing()

# Test without fix (should fail on macOS, possibly Linux)
add_test(NAME test_without_fix
         COMMAND test_typeinfo
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Build a second version with the fix
add_library(store_fixed SHARED libstore.cpp)
target_compile_options(store_fixed PRIVATE -fvisibility=default -DUSE_STRCMP_FIX)
target_include_directories(store_fixed PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(fetch_fixed SHARED libfetch.cpp)
target_compile_options(fetch_fixed PRIVATE -fvisibility=hidden -DUSE_STRCMP_FIX)
target_include_directories(fetch_fixed PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_typeinfo_fixed main.cpp)
target_compile_definitions(test_typeinfo_fixed PRIVATE USE_STRCMP_FIX)
target_link_libraries(test_typeinfo_fixed PRIVATE store_fixed fetch_fixed)
target_include_directories(test_typeinfo_fixed PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Test with fix (should pass)
add_test(NAME test_with_fix
         COMMAND test_typeinfo_fixed
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Expect the first test to fail
set_tests_properties(test_without_fix PROPERTIES WILL_FAIL TRUE)

message(STATUS "")
message(STATUS "===================================")
message(STATUS "TypeInfo Reproducer Configuration:")
message(STATUS "===================================")
message(STATUS "Build 'test_typeinfo' to see the bug")
message(STATUS "Build 'test_typeinfo_fixed' to see the workaround")
message(STATUS "Run 'ctest -V' to run both tests")
message(STATUS "===================================")
