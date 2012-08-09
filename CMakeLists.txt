cmake_minimum_required(VERSION 2.8)
project(RINGBUF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
add_library(ringbuf ringbuf.c)

find_package(PkgConfig)
pkg_check_modules(GLIB glib-2.0)

add_executable(test-ringbuf EXCLUDE_FROM_ALL test_ringbuf.c)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(test-ringbuf ringbuf ${GLIB_LIBRARIES})
add_custom_target(test test-ringbuf DEPENDS test-ringbuf)
