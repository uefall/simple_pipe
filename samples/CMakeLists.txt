add_executable(run_demo_cpp run_demo.cpp)
target_link_libraries(run_demo_cpp PRIVATE simple_pipe)
target_compile_definitions(run_demo_cpp PRIVATE SIMPLE_PIPE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_executable(run_five_node_100_cpp run_five_node_100.cpp)
target_link_libraries(run_five_node_100_cpp PRIVATE simple_pipe)
target_compile_definitions(run_five_node_100_cpp PRIVATE SIMPLE_PIPE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
