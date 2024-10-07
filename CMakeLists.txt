cmake_minimum_required(VERSION 3.29)

project(squash)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/gl.cpp
    src/input.cpp
    src/ui.cpp
    src/window.cpp
    )

set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    find_library(CARBON Carbon REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
        ${CARBON_LIBRARY}
        fmt::fmt
        GLEW::GLEW
        glfw
        glm::glm
    )
else()
    # For non-macOS systems, you can specify general linking here
    target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        GLEW::GLEW
        glfw
        glm::glm
        imgui::imgui
    )
endif()