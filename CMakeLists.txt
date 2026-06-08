cmake_minimum_required(VERSION 3.29)
project(GraphicEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

# 1. Restore the compilation pipeline for external libraries
add_subdirectory(external/glfw)
add_subdirectory(external/glad)

# 2. Map the include directories
include_directories(
        external/glfw/include
        external/glad/include
        external/assimp/include
        include
        external/glm
)

# 3. Compile the executable including our new image implementation
add_executable(GraphicEngine
        src/main.cpp
        src/shader_utils.cpp
        src/mesh.cpp
        src/model.cpp
        src/stb_image.cpp
)

# 4. Link the compiled external targets and system libraries
target_link_libraries(GraphicEngine
        ${OPENGL_LIBRARIES}
        glfw
        glad
        assimp
        /usr/lib/x86_64-linux-gnu/libdraco.so.8
        ${CMAKE_DL_LIBS}
        m
)

# 5. Automatically transfer resources to the build directory
add_custom_command(TARGET GraphicEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:GraphicEngine>/res")