#pragma once

#include "core.hpp"
#include "device.hpp"
#include "render_pass.hpp"
#include <unordered_map>

namespace gl
{
    class Pipeline
    {
    public:
        VkPipeline handle;
        VkPipelineLayout layout;
        VkPipelineLayoutCreateInfo layoutCreateInfo;
        VkGraphicsPipelineCreateInfo createInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
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
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfos;
        std::unordered_map<int, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;
        std::string label;

        Pipeline();

        bool Create(const gl::Device &device);
        void Destroy(const gl::Device &device);
        void Label(const std::string& str);
        bool CreateLayout(const gl::Device &device);
        void DestroyLayout(const gl::Device &device);
        bool CreateDescriptorSetLayouts(const gl::Device &device);
        void DestroyDescriptorSetLayouts(const gl::Device &device);

        void AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule handle, const char *entrypoint = "main");
        void AddDynamicViewport(int numViewports = 1);
        void AddDynamicScissor(int numScissors = 1);
        VkPipelineColorBlendAttachmentState &AddColorBlendAttachment();
        void SetMultisample(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
        void SetRasterization(VkFrontFace frontFace, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT);
        void SetInputAssembly(VkPrimitiveTopology topology);
        void SetVertexInput();
        void SetRenderPass(const RenderPass &renderPass);
        void SetDepthStencil();
        VkVertexInputBindingDescription &AddVertexInputBindingDescription(std::uint32_t binding, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);
        VkVertexInputAttributeDescription &AddVertexInputAttributeDescription(std::uint32_t binding, std::uint32_t location, VkFormat format = VK_FORMAT_R32G32B32_SFLOAT, std::uint32_t offset = 0);
        int AddDescriptorSetLayout();
        VkDescriptorSetLayoutBinding &AddDescriptorSetLayoutBinding(int descriptorSetLayout, int binding, VkDescriptorType type, VkShaderStageFlags stageFlags);
    };
}
