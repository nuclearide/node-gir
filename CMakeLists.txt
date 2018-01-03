cmake_minimum_required(VERSION 2.8)

project(girepository)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(NODE_MODULES "node_modules")

# src
file(GLOB_RECURSE SOURCE_FILES "src/*.cc" "src/*.h")

# cmake-js
include_directories(${CMAKE_JS_INC})
include_directories("src")


# using pkg-config for getting gobject introspection lib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GOBJECT_INTROSPECTION REQUIRED gobject-introspection-1.0)
include_directories(${GOBJECT_INTROSPECTION_INCLUDE_DIRS})
link_directories(${GOBJECT_INTROSPECTION_LIBRARY_DIRS})

# create taret for the shared library (bindings)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# link the bindings against our dependencies
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
target_link_libraries(${PROJECT_NAME} ${GOBJECT_INTROSPECTION_LIBRARIES})
