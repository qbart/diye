cmake_minimum_required(VERSION 3.29)

project(diye)

add_executable(${PROJECT_NAME}
    src/core/transform.cpp
    src/core/camera.cpp
    src/core/animation_curve.cpp
    src/geometry/half_edge.cpp
    src/geometry/half_edge_mesh.cpp

    src/experiments/experiment.cpp

    src/debug_draw_renderer.cpp
    src/gl.cpp
    src/image.cpp
    src/input.cpp
    src/ui.cpp
    src/window.cpp

    src/main.cpp
    )

set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

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
        glm::glm
        imgui::imgui
        imguizmo::imguizmo
        SDL2::SDL2
        SDL2::SDL2main
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    )
else()
    # For non-macOS systems, you can specify general linking here
    target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        GLEW::GLEW
        glm::glm
        imgui::imgui
        imguizmo::imguizmo
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_image::SDL2_image
    )
endif()