#pragma once

#include "deps/sdl.hpp"
#include "gl/vulkan.hpp"
#include "input.hpp"

class Window
{
public:
    using Ptr = std::shared_ptr<Window>;
    struct Vertex
    {
        Vec2 pos;
        Vec3 color;
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
    std::vector<VkImageView> imageViews;
    gl::ShaderModules shaderModules;
    gl::Pipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandPool shortLivedCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
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
    std::vector<gl::Buffer> uniformBuffers;
    std::vector<gl::Memory> uniformBuffersMemory;
    std::vector<UniformBufferObject> ubos;
    VkDescriptorPoolSize descriptorPoolSize;
    VkDescriptorPool descriptorPool;
};