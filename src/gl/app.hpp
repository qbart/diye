#pragma once

#include "../deps/sdl.hpp"
#include "vulkan.hpp"
#include "../io/image.hpp"

namespace gl
{
    class App
    {
    public:
        enum class State {
            Ok,
            Continue,
            Error,
        };

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
        App();
        ~App();

        void MaxFramesInFlight(int maxFrames);
        int MaxFramesInFlight() const { return maxFramesInFlight; }
        bool Init(SDL_Window *wnd);
        void Shutdown();
        bool Render(Mat4 mvp);
        State BeginFrame();
        std::uint32_t Frame() const { return currentFrame; }
        State EndFrame();
        void RequestRecreateSwapChain(bool recreate) { needRecreateSwapChain = recreate; }
        uint32_t ImageIndex() const { return imageIndex; }
        void WithUI(bool withUI) { this->withUI = withUI; }

    private:
        bool InitGL();
        bool RecreateSwapChain();
        void ShutdownGL();

    private:
        SDL_Window *wnd;
        bool needRecreateSwapChain;
        int maxFramesInFlight;

        std::uint32_t imageIndex;
        std::uint32_t currentFrame;
        bool withUI;

    public:
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
        gl::Memory textureMemory;
        gl::ImageView textureView;
        gl::Image depthImage;
        gl::Memory depthImageMemory;
        gl::ImageView depthImageView;
        gl::Sampler textureSampler;
        std::vector<gl::Buffer> uniformBuffers;
        std::vector<gl::Memory> uniformBuffersMemory;
        std::vector<UniformBufferObject> ubos;
        gl::DescriptorPool descriptorPool;
    };
}
