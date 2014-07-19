cmake_minimum_required(VERSION 2.8)

project(nebula)

# Compile external dependencies 
add_subdirectory(external)

include_directories("${PROJECT_BINARY_DIR}")

# CMAKE submodules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/external/wubwubcmake/modules")

# Add main project target
file(GLOB_RECURSE NEBULA_SOURCES src/*.cpp)
file(GLOB_RECURSE NEBULA_HEADERS src/*.hpp)

add_executable(nebula
	${NEBULA_SOURCES}
	${NEBULA_HEADERS}
)

# Default sane options
add_definitions("-Wall -Wextra -Weffc++ -std=c++0x -pedantic -g -O3")

# Dependencies
include_directories(SYSTEM external/glfw/include/GLFW/)
target_link_libraries(nebula ${GLFW_LIBRARIES} glfw)

include_directories(SYSTEM external/glm/)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(nebula ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
target_link_libraries(nebula ${GLUT_glut_LIBRARY})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(nebula ${GLEW_LIBRARY})

find_package(msgpack REQUIRED)
include_directories(${msgpack_INCLUDE_DIRS})
target_link_libraries(nebula ${msgpack_LIBRARIES})

find_package(Boost COMPONENTS system filesystem program_options iostreams REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
target_link_libraries(nebula ${Boost_LIBRARIES})

# Copy shaders to build dir
include(MacroAddCopyTarget)
add_copy_target(nebula-shaders "src/shaders" "shaders")
add_copy_target(nebula-textures "src/textures" "textures")
