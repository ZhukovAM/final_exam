cmake_minimum_required(VERSION 2.6)
project(final)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lev")

set(SOURCE_FILES main.cpp)

FIND_PATH(LIBEV_INCLUDE_DIR ev.h HINTS /usr/include/libev )
FIND_LIBRARY(LIBEV_LIBRARIES NAMES ev)

IF(LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES)
    SET(LIBEV_FOUND TRUE)
ENDIF(LIBEV_INCLUDE_DIR AND LIBEV_LIBRARIES)

IF(LIBEV_FOUND)
    IF (NOT Libev_FIND_QUIETLY)
        MESSAGE(STATUS "Found libev includes:	${LIBEV_INCLUDE_DIR}/libev/ev.h")
        MESSAGE(STATUS "Found libev library: ${LIBEV_LIBRARIES}")
    ENDIF (NOT Libev_FIND_QUIETLY)
ELSE(LIBEV_FOUND)
    IF (Libev_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could NOT find libev development files")
    ENDIF (Libev_FIND_REQUIRED)
ENDIF(LIBEV_FOUND)


include_directories("${LIBEV_INCLUDE_DIR}")

link_directories("${LIBEV_LIBRARIES}")

add_executable(final ${SOURCE_FILES})

target_link_libraries(final ${LIBEV_LIBRARIES})

