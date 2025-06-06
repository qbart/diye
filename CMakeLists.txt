cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

project(diye)

add_executable(${PROJECT_NAME}
    src/deps/sdl.cpp
    src/deps/fmt.cpp
    src/gl/instance.cpp
    src/gl/surface.cpp
    src/gl/physical_device.cpp
    src/gl/device.cpp
    src/gl/queue.cpp
    src/gl/swap_chain.cpp
    src/gl/shader_modules.cpp
    src/gl/render_pass.cpp
    src/gl/pipeline.cpp
    src/gl/buffer.cpp
    src/gl/memory.cpp
    src/gl/command_pool.cpp
    src/gl/command_buffer.cpp
    src/gl/semaphore.cpp
    src/gl/fence.cpp
    src/gl/image.cpp
    src/gl/image_view.cpp
    src/gl/framebuffer.cpp
    src/gl/sampler.cpp
    src/gl/descriptor_pool.cpp
    src/gl/app.cpp
    src/gl/debug_renderer.cpp
    src/core/transform.cpp
    src/core/math.cpp
    src/core/camera.cpp
    src/core/animation_curve.cpp
    src/geometry/half_edge.cpp
    src/geometry/half_edge_mesh.cpp
    src/io/binary.cpp
    src/io/image.cpp
    src/io/obj.cpp

    src/experiments/experiment.cpp

    src/ui/ui.cpp

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
find_package(Vulkan REQUIRED)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMDD_NO_SIMD=1")

    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(CARBON Carbon REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${CARBON_LIBRARY}
        fmt::fmt
        GLEW::GLEW
        glm::glm
        imgui::imgui
        imguizmo::imguizmo
        SDL2::SDL2
        SDL2::SDL2main
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        Vulkan::Vulkan
    )
elseif (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMDD_NO_SIMD=1")

    target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        GLEW::GLEW
        glm::glm
        imgui::imgui
        imguizmo::imguizmo
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_image
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
