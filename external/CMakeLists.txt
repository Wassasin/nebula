# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Local copy of GLFW")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Local copy of GLFW")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Local copy of GLFW")

add_subdirectory(glfw)

# GLM
add_subdirectory(glm)
