#pragma once

#include "deps/sdl.hpp"
#include "gl/vulkan.hpp"
#include "input.hpp"
#include "io/image.hpp"

class Window
{
public:
    const static int MAX_FRAMES_IN_FLIGHT = 2;

    using Ptr = std::shared_ptr<Window>;
    struct Vertex
    {
        Vec3 pos;
        Vec3 color;
        Vec2 uv;
    };
    struct UniformBufferObject
    {
        Mat4 mvp;
    };

    static Ptr New(int w, int h, const std::string &title);
    Window() {}
    ~Window();

    SDL_Window *Get() const { return wnd; }
    void Swap();
    void PollEvents();
    void Close();
    void Debug();
    void RecreateSwapChain();
    inline bool IsOpen() const { return isOpen; }
    inline const Dimension &Size() const { return size; }
    inline Input GetInput() { return Input(inputs); }
    inline Vec2 MousePosition() const { return mousePos; }
    inline Vec2 MouseRelativePosition() const { return mouseRelPos; }
    inline Vec2 MouseWheel() const { return mouseWheel; }
    inline bool MouseButtonDown(uint8 button) const { return HashMapHasKey(mouseInputs, button) && mouseInputs.at(button); }
    inline bool MouseWheelScrolled() const { return SDL_GetTicks() - lastTimeWheeled < 100; }
    inline bool WasResized() const { return resized; }

private:
    bool InitGL();
    void ShutdownGL();
    bool active = false;
    bool isOpen = false;
    Dimension size;
    SDL_Window *wnd = nullptr;
    SDL_Event event;
    HashMap<int32, bool> inputs;
    HashMap<uint8, bool> mouseInputs;
    Vec2 mousePos = Vec2(0, 0);
    Vec2 mouseRelPos = Vec2(0, 0);
    Vec2 mouseWheel = Vec2(0, 0);
    uint64 lastTimeWheeled = 0;
    bool resized = false;

    gl::Instance instance;
    gl::Device device;
    gl::Surface surface;
    gl::PhysicalDevice physicalDevice;
    gl::SwapChain swapChain;
    gl::RenderPass renderPass;
    std::vector<gl::ImageView> imageViews;
    gl::ShaderModules shaderModules;
    gl::Pipeline graphicsPipeline;
    std::vector<gl::Framebuffer> swapChainFramebuffers;
    gl::CommandPool commandPool;
    gl::CommandPool shortLivedCommandPool;
    gl::CommandBuffer commandBuffers;
    gl::Semaphore imageAvailableSemaphores;
    gl::Semaphore renderFinishedSemaphores;
    gl::Fence inFlightFences;
    uint32_t currentFrame = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    gl::Buffer vertexBuffer;
    gl::Memory vertexBufferMemory;
    gl::Buffer stagingBuffer;
    gl::Memory stagingBufferMemory;
    gl::Buffer indexBuffer;
    gl::Memory indexBufferMemory;
    gl::Buffer indexStagingBuffer;
    gl::Memory indexStagingBufferMemory;
    gl::Image texture;
    gl::Image color;
    gl::Memory textureMemory;
    gl::Memory colorMemory;
    gl::ImageView textureView;
    gl::ImageView colorView;
    gl::Image depthImage;
    gl::Memory depthImageMemory;
    gl::ImageView depthImageView;
    gl::Sampler textureSampler;
    std::vector<gl::Buffer> uniformBuffers;
    std::vector<gl::Memory> uniformBuffersMemory;
    std::vector<UniformBufferObject> ubos;
    gl::DescriptorPool descriptorPool;
};
