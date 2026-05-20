cmake_minimum_required(VERSION 3.16)
project(simple_pipe VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SIMPLE_PIPE_BUILD_TESTS "Build unit tests" ON)
option(SIMPLE_PIPE_BUILD_SAMPLES "Build sample executables" ON)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)

if(SIMPLE_PIPE_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(nlohmann_json)
if(SIMPLE_PIPE_BUILD_TESTS)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
endif()

add_subdirectory(src)

if(SIMPLE_PIPE_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(SIMPLE_PIPE_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
