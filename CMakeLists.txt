cmake_minimum_required(VERSION 3.5)

# Used with documentation
set(MAIN_PROJECT_NAME "Kabanero")
set(MAIN_PROJECT_VERSION "0.1.0")

set(EXECUTABLE_NAME "kabanero")
project("${EXECUTABLE_NAME}" C CXX)

set(IMGUI_SFML_BUILD_EXAMPLES 0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE SOURCE_FILES      src/*.cpp include/*.hpp)
# file(GLOB_RECURSE SOURCE_TEST_FILES test/*.cpp)
file(GLOB_RECURSE RESOURCE_FILES resources/*)

include_directories(/usr/bin/include)
include_directories(include)
include_directories(lib/glm)
include_directories(lib/box2d/Box2D)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# add_subdirectory(test)

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "Build Box2D examples")
add_subdirectory("lib/box2d/Box2D")

# Documentation
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API
documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
  message("Doxygen found.")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()

# Add executable
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# Resources
set(RES_SOURCE "${CMAKE_SOURCE_DIR}/resources")
set(RES_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")

add_custom_command(
 TARGET ${PROJECT_NAME} POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E create_symlink ${RES_SOURCE} ${RES_DEST}
 DEPENDS ${RES_DEST}
 COMMENT "symbolic link resources folder from ${RES_SOURCE} => ${RES_DEST}"
)

# SFML
find_package(SFML REQUIRED system window graphics network audio)
if (SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR} ${IMGUI_SFML_INCLUDE_DIRS})
  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${IMGUI_SFML_DEPENDENCIES})
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
if (OPENGL_FOUND)
  target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
  # or: target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_gl_LIBRARY})
  target_link_libraries(${EXECUTABLE_NAME} m)  # if you use maths.h
endif()

# include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/Box2D/Box2D )
target_link_libraries(${EXECUTABLE_NAME} Box2D)
