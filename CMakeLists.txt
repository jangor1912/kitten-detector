# Followed instructions from https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
# Stolen cmake directory from https://gitlab.com/cclaunch/deepstream_app/-/tree/master/

cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(kitten-detector VERSION 1.0.1 DESCRIPTION "Library to detect kitties on videos")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_LIBDIR})

find_package(GStreamer REQUIRED)
find_package(DeepstreamAppCommon REQUIRED)
find_package(NVDS REQUIRED)
find_package(CUDA REQUIRED)
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(JSON-GLIB REQUIRED json-glib-1.0)

include_directories(
        ${GSTREAMER_INCLUDE_DIRS}
        ${GSTREAMER_VIDEO_INCLUDE_DIRS}
        ${GSTREAMER_RTSPSERVER_INCLUDE_DIRS}
        ${GSTREAMER_BASE_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
        ${JSON-GLIB_INCLUDE_DIRS}
        ${DEEPSTREAM_APP_COMMON_INCLUDE_DIRS}
        ${NVDS_INCLUDE_DIRS}
        ${CUDA_INCLUDE_DIRS}
        ${X11_INCLUDE_DIR}
)

add_library(kitten-detector SHARED
        src/pipeline.c src/pipeline.h
        src/sources/sources.c src/sources/sources.h
        src/sinks/sinks.c src/sinks/sinks.h
        src/handlers/handlers.c src/handlers/handlers.h
        src/utils/utils.c src/utils/utils.h
        src/inference/inference.c src/inference/inference.h
        src/muxer/muxer.c src/muxer/muxer.h
        src/sources/decoders.c src/sources/decoders.h
        src/structures/structures.h src/structures/structures.c
        src/probes/probes.c src/probes/probes.h
        src/metadata/metadata.c src/metadata/metadata.h src/probes/timestamps.c src/probes/timestamps.h src/recorder/recorder.c src/recorder/recorder.h)
add_executable(kitten-detector-cli cli/main.c)
target_link_libraries(kitten-detector-cli kitten-detector)
target_include_directories(kitten-detector-cli PRIVATE src)

set_target_properties(kitten-detector PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(kitten-detector PROPERTIES SOVERSION 1)
set_target_properties(kitten-detector PROPERTIES PUBLIC_HEADER src/pipeline.h)

target_include_directories(kitten-detector PRIVATE src)

include(GNUInstallDirs)

target_link_libraries(kitten-detector
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_VIDEO_LIBRARIES}
        ${GSTREAMER_RTSPSERVER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${JSON-GLIB_LIBRARIES}
        ${NVDS_LIBRARIES}
        ${CUDA_LIBRARIES}
        ${CMAKE_DL_LIBS}
        ${X11_LIBRARIES}
        )

install(TARGETS kitten-detector
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
