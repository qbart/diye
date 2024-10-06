cmake_minimum_required(VERSION 3.29)

project(squash-story-cpp)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

add_executable(squash
    src/main.cpp
    src/input.cpp
    src/gl.cpp
    src/window.cpp
    )

set_target_properties(squash PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )

target_link_libraries(squash PRIVATE
    fmt::fmt
    GLEW::GLEW
    glfw
    glm::glm
    )