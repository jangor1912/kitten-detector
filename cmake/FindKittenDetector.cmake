# Select install dir using provided version.
SET(KITTEN_DETECTOR_INSTALL_DIR /usr/local)

# List all libraries in deepstream.
SET(KITTEN_DETECTOR_LIB kitten-detector)

# Find all libraries in list.
find_library(${KITTEN_DETECTOR_LIB}_PATH NAMES ${KITTEN_DETECTOR_LIB} PATHS ${KITTEN_DETECTOR_INSTALL_DIR}/lib)
if(${KITTEN_DETECTOR_LIB}_PATH)
    message(INFO " Found kitten-detector library: ${${KITTEN_DETECTOR_LIB}_PATH}")
    set(KITTEN_DETECTOR_LIB_PATH ${${KITTEN_DETECTOR_LIB}_PATH})
else()
    message(FATAL ERROR " Unable to find lib: ${KITTEN_DETECTOR_LIB}")
    set(KITTEN_DETECTOR_LIB_PATH FALSE)
endif()

# Find include directories.
find_path(KITTEN_DETECTOR_INCLUDE_DIR
        NAMES
        pipeline.h
        HINTS
        ${KITTEN_DETECTOR_INSTALL_DIR}/include
        )

# Check libraries and includes.
if (KITTEN_DETECTOR_LIB AND KITTEN_DETECTOR_INCLUDE_DIR)
    set(KITTEN_DETECTOR_FOUND TRUE)
    message(INFO " KITTEN_DETECTOR_LIB_PATH = ${KITTEN_DETECTOR_LIB_PATH}")
    message(INFO " KITTEN_DETECTOR_INCLUDE_DIR = ${KITTEN_DETECTOR_INCLUDE_DIR}")
else()
    message(FATAL ERROR " Unable to find kitten-detector library")
endif()

