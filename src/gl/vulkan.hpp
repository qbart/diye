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
#include "surface.hpp"
#include "physical_device.hpp"
#include "device.hpp"

namespace vulkan
{
    struct CreateSwapChainInfo
    {
        gl::Surface surface;
        gl::PhysicalDevice physicalDevice;
        gl::Device device;
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

        bool Create(const gl::Device &device);
        void Destroy(const gl::Device &device);
        bool CreateLayout(const gl::Device &device);
        void DestroyLayout(const gl::Device &device);

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

    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window);
    SwapChain CreateSwapChain(const CreateSwapChainInfo &info);
    void DestroySwapChain(const gl::Device &device, const SwapChain &swapChain);
    std::vector<VkImageView> CreateImageViews(const gl::Device &device, const SwapChain &swapChain);
    void DestroyImageViews(const gl::Device &device, const std::vector<VkImageView> &views);
    void DestroyFramebuffers(const gl::Device &device, const std::vector<VkFramebuffer> &framebuffers);
    ShaderModule CreateShaderModule(const gl::Device &device, const std::vector<char> &code);
    void DestroyShaderModule(const gl::Device &device, const ShaderModule &module);
    RenderPass CreateRenderPass(const gl::Device &device, const SwapChain &swapChain, const ShaderModules &modules);
    void DestroyRenderPass(const gl::Device &device, const RenderPass &renderPass);
};