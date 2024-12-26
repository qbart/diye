#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <string>
#include <vector>
#include <optional>
#include "../deps/fmt.hpp"
#include "../deps/sdl.hpp"
#include "core.hpp"
#include "instance.hpp"

namespace vulkan
{
    struct Surface
    {
        VkSurfaceKHR handle;
        SDL_Window *window;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct PhysicalDevice
    {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        std::vector<VkExtensionProperties> extensions;
        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndices queueFamilyIndices;
        SwapChainSupportDetails swapChainSupport;

        bool IsDiscreteGPU() const;
        bool IsValid() const;
        bool IsExtensionSupported(const gl::CStrings &extensions) const;
        void QuerySwapChainSupport(const Surface &surface);
        void QueryQueueFamilies(const Surface &surface);
    };

    struct CreateDeviceInfo
    {
        PhysicalDevice physicalDevice;
        gl::CStrings validationLayers;
        gl::CStrings requiredExtensions;
    };

    struct Device
    {
        VkDevice handle;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
        VkResult WaitIdle() const;
    };

    struct CreateSwapChainInfo
    {
        Surface surface;
        PhysicalDevice physicalDevice;
        Device device;
        gl::CStrings validationLayers;
    };

    struct SwapChain
    {
        VkSwapchainKHR handle;
        std::vector<VkImage> images;
        VkFormat imageFormat;
        VkExtent2D extent;

        bool IsValid() const;
    };

    struct ShaderModule
    {
        VkShaderModule handle;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct ShaderModules
    {
        ShaderModule vert;
        ShaderModule frag;
    };

    struct ShaderStage
    {
        VkShaderModule module;
        VkShaderStageFlagBits stage;
    };

    struct RenderPass
    {
        VkRenderPass handle;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct Pipeline
    {
        VkPipeline handle;
        VkPipelineLayout layout;
        VkPipelineLayoutCreateInfo layoutCreateInfo;
        VkGraphicsPipelineCreateInfo createInfo;
        std::vector<VkDynamicState> dynamicStates;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;

        Pipeline();

        bool Create(const Device &device);
        void Destroy(const Device &device);
        bool CreateLayout(const Device &device);
        void DestroyLayout(const Device &device);

        void AddShaderStage(VkShaderStageFlagBits stage, const ShaderModule &module, const char *entrypoint = "main");
        void AddDynamicViewport(int numViewports = 1);
        void AddDynamicScissor(int numScissors = 1);
        VkPipelineColorBlendAttachmentState &AddColorBlendAttachment();
        void SetMultisample();
        void SetRasterization(VkFrontFace frontFace, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT);
        void SetInputAssembly(VkPrimitiveTopology topology);
        void SetVertexInput();
        void SetRenderPass(const RenderPass &renderPass);

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }

    };

    Surface CreateSurface(const gl::Instance &instance, SDL_Window *window);
    void DestroySurface(const gl::Instance &instance, const Surface &surface);
    std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device);
    std::vector<PhysicalDevice> GetPhysicalDevices(const gl::Instance &instance, const Surface &surface);
    PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices);
    Device CreateDevice(const CreateDeviceInfo &info);
    void DestroyDevice(const Device &device);
    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window);
    SwapChain CreateSwapChain(const CreateSwapChainInfo &info);
    void DestroySwapChain(const Device &device, const SwapChain &swapChain);
    std::vector<VkImageView> CreateImageViews(const Device &device, const SwapChain &swapChain);
    void DestroyImageViews(const Device &device, const std::vector<VkImageView> &views);
    void DestroyFramebuffers(const Device &device, const std::vector<VkFramebuffer> &framebuffers);
    ShaderModule CreateShaderModule(const Device &device, const std::vector<char> &code);
    void DestroyShaderModule(const Device &device, const ShaderModule &module);
    RenderPass CreateRenderPass(const Device &device, const SwapChain &swapChain, const ShaderModules &modules);
    void DestroyRenderPass(const Device &device, const RenderPass &renderPass);
};