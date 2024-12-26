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
#include "swap_chain.hpp"
#include "shader_modules.hpp"
#include "render_pass.hpp"

namespace vulkan
{
    struct ShaderStage
    {
        VkShaderModule module;
        VkShaderStageFlagBits stage;
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

        void AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule handle, const char *entrypoint = "main");
        void AddDynamicViewport(int numViewports = 1);
        void AddDynamicScissor(int numScissors = 1);
        VkPipelineColorBlendAttachmentState &AddColorBlendAttachment();
        void SetMultisample();
        void SetRasterization(VkFrontFace frontFace, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT);
        void SetInputAssembly(VkPrimitiveTopology topology);
        void SetVertexInput();
        void SetRenderPass(const gl::RenderPass &renderPass);

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }

    };

    std::vector<VkImageView> CreateImageViews(const gl::Device &device, VkFormat format, const std::vector<VkImage> &images);
    void DestroyImageViews(const gl::Device &device, const std::vector<VkImageView> &views);
    void DestroyFramebuffers(const gl::Device &device, const std::vector<VkFramebuffer> &framebuffers);
};