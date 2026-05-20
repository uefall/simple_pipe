set(SIMPLE_PIPE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/result.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/enums.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/log/logger.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/context/frame_meta.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/spec/graph_spec.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/spec/runtime_config.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/operator.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/node_registry.hpp
    ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/source_operator.hpp
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
    log/logger.cpp
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

if(SIMPLE_PIPE_WITH_OPENCV)
  list(APPEND SIMPLE_PIPE_HEADERS
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/frame_buffer.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/media_sample.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/i_media_reader.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/opencv_image_reader.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/opencv_video_reader.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/media/frame_meta_builder.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/pull_source.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/image_source.hpp
      ${CMAKE_SOURCE_DIR}/include/simple_pipe/operators/generic/video_source.hpp
  )
  list(APPEND SIMPLE_PIPE_SOURCES
      media/opencv_image_reader.cpp
      media/opencv_video_reader.cpp
      media/frame_meta_builder.cpp
      operators/generic/pull_source.cpp
      operators/generic/image_source.cpp
      operators/generic/video_source.cpp
  )
endif()

add_library(simple_pipe STATIC ${SIMPLE_PIPE_SOURCES} ${SIMPLE_PIPE_HEADERS})
target_include_directories(simple_pipe
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(simple_pipe PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(simple_pipe PRIVATE -Wall -Wextra -Wpedantic)

if(SIMPLE_PIPE_WITH_OPENCV)
  target_link_libraries(simple_pipe PUBLIC ${OpenCV_LIBS})
  target_include_directories(simple_pipe PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_compile_definitions(simple_pipe PUBLIC SIMPLE_PIPE_WITH_OPENCV=1)
endif()

add_library(simple_pipe::simple_pipe ALIAS simple_pipe)
