include(GoogleTest)

add_executable(simple_pipe_tests
    test_helper.hpp
    test_builder.cpp
    test_pipeline.cpp
    test_runtime_configure.cpp
    test_graphs.cpp
    test_image_source.cpp
)

target_link_libraries(simple_pipe_tests
    PRIVATE
        simple_pipe
        GTest::gtest_main
)

if(TARGET simple_pipe AND SIMPLE_PIPE_WITH_OPENCV)
  target_compile_definitions(simple_pipe_tests PRIVATE SIMPLE_PIPE_WITH_OPENCV=1)
endif()

target_compile_definitions(simple_pipe_tests
    PRIVATE
        SIMPLE_PIPE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

gtest_discover_tests(simple_pipe_tests)
