#pragma once

#include "core.hpp"
#include "device.hpp"
#include "render_pass.hpp"

namespace gl
{
    class Pipeline
    {
    public:
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
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

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
        void SetRenderPass(const RenderPass &renderPass);
        VkVertexInputBindingDescription &AddVertexInputBindingDescription(std::uint32_t binding, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);
        VkVertexInputAttributeDescription &AddVertexInputAttributeDescription(std::uint32_t binding, std::uint32_t location, VkFormat format = VK_FORMAT_R32G32B32_SFLOAT, std::uint32_t offset = 0);
    };
}