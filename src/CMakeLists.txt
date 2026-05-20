set(SIMPLE_PIPE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/result.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/enums.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/context/frame_meta.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/spec/graph_spec.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/spec/runtime_config.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/operator.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/node_registry.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/app_source.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/app_sink.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/passthrough.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/resize.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/mock_detector.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/mock_motion.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/mock_tracker.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/mock_safety_rules.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/builder/pipeline_builder.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/pipeline/pipeline_base.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/pipeline/simple_pipeline.hpp
)

set(SIMPLE_PIPE_SOURCES
    context/frame_meta.cpp
    spec/graph_spec.cpp
    spec/runtime_config.cpp
    operators/operator.cpp
    operators/node_registry.cpp
    operators/generic/app_source.cpp
    operators/generic/app_sink.cpp
    operators/generic/passthrough.cpp
    operators/generic/resize.cpp
    operators/generic/mock_detector.cpp
    operators/generic/mock_motion.cpp
    operators/generic/mock_tracker.cpp
    operators/generic/mock_safety_rules.cpp
    builder/pipeline_builder.cpp
    pipeline/pipeline_base.cpp
    pipeline/simple_pipeline.cpp
)

add_library(simple_pipe STATIC ${SIMPLE_PIPE_SOURCES} ${SIMPLE_PIPE_HEADERS})
target_include_directories(simple_pipe
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(simple_pipe PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(simple_pipe PRIVATE -Wall -Wextra -Wpedantic)

add_library(simple_pipe::simple_pipe ALIAS simple_pipe)
