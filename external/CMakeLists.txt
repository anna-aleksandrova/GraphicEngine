
add_subdirectory(glfw)

add_subdirectory(glad)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)