#include "pipeline.hpp"

namespace gl
{
    Pipeline::Pipeline() : handle(VK_NULL_HANDLE),
                           layout(VK_NULL_HANDLE),
                           createInfo({}),
                           layoutCreateInfo({}),
                           dynamicStateCreateInfo({}),
                           viewportStateCreateInfo({}),
                           colorBlendStateCreateInfo({}),
                           multisampleStateCreateInfo({}),
                           rasterizationStateCreateInfo({}),
                           inputAssemblyStateCreateInfo({}),
                           vertexInputStateCreateInfo({}),
                           depthStencilStateCreateInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        createInfo.basePipelineIndex = -1;              // Optional
        createInfo.pDepthStencilState = nullptr;        // Optional
        createInfo.layout = VK_NULL_HANDLE;
        createInfo.subpass = 0;

        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 0;            // Optional
        layoutCreateInfo.pSetLayouts = nullptr;         // Optional
        layoutCreateInfo.pushConstantRangeCount = 0;    // Optional
        layoutCreateInfo.pPushConstantRanges = nullptr; // Optional

        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = 0;
        dynamicStateCreateInfo.pDynamicStates = nullptr;

        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 0;
        viewportStateCreateInfo.pViewports = nullptr;
        viewportStateCreateInfo.scissorCount = 0;
        viewportStateCreateInfo.pScissors = nullptr;

        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlendStateCreateInfo.attachmentCount = 0;
        colorBlendStateCreateInfo.pAttachments = nullptr;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional
    }

    bool Pipeline::Create(const gl::Device &device)
    {
        if (vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle) == VK_SUCCESS)
        {
            fmtx::Info("Created graphics pipeline");
            return true;
        }

        fmtx::Error("Failed to create graphics pipeline");
        return false;
    }

    void Pipeline::Destroy(const gl::Device &device)
    {
        vkDestroyPipeline(device.handle, handle, nullptr);
    }

    bool Pipeline::CreateLayout(const gl::Device &device)
    {
        if (vkCreatePipelineLayout(device.handle, &layoutCreateInfo, nullptr, &layout) == VK_SUCCESS)
        {
            createInfo.layout = layout;
            fmtx::Info("Created pipeline layout");
            return true;
        }

        fmtx::Error("Failed to create pipeline layout");
        return false;
    }

    void Pipeline::DestroyLayout(const gl::Device &device)
    {
        vkDestroyPipelineLayout(device.handle, layout, nullptr);
    }

    bool Pipeline::CreateDescriptorSetLayouts(const gl::Device &device)
    {
        descriptorSetLayouts.resize(descriptorSetLayoutCreateInfos.size());
        for (int i = 0; i < descriptorSetLayoutCreateInfos.size(); ++i)
        {
            if (vkCreateDescriptorSetLayout(device.handle, &descriptorSetLayoutCreateInfos[i], nullptr, &descriptorSetLayouts[i]) != VK_SUCCESS)
            {
                fmtx::Error("Failed to create descriptor set layout");
                return false;
            }
        }
        layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

        fmtx::Info("Created descriptor set layouts");

        return true;
    }

    void Pipeline::DestroyDescriptorSetLayouts(const gl::Device &device)
    {
        for (auto layout : descriptorSetLayouts)
            vkDestroyDescriptorSetLayout(device.handle, layout, nullptr);
    }

    void Pipeline::AddShaderStage(VkShaderStageFlagBits stage, VkShaderModule handle, const char *entrypoint)
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = stage;
        info.module = handle;
        info.pName = entrypoint;

        shaderStages.push_back(info);

        createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        createInfo.pStages = shaderStages.data();
    }

    void Pipeline::AddDynamicViewport(int numViewports)
    {
        for (int i = 0; i < numViewports; ++i)
        {
            dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);

            viewportStateCreateInfo.viewportCount += 1;
        }

        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        createInfo.pDynamicState = &dynamicStateCreateInfo;
        createInfo.pViewportState = &viewportStateCreateInfo;
    }

    void Pipeline::AddDynamicScissor(int numScissors)
    {
        for (int i = 0; i < numScissors; ++i)
        {
            dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

            viewportStateCreateInfo.scissorCount += 1;
        }

        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        createInfo.pDynamicState = &dynamicStateCreateInfo;
        createInfo.pViewportState = &viewportStateCreateInfo;
    }

    VkPipelineColorBlendAttachmentState &Pipeline::AddColorBlendAttachment()
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        colorBlendAttachments.push_back(colorBlendAttachment);
        colorBlendStateCreateInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlendStateCreateInfo.pAttachments = colorBlendAttachments.data();
        createInfo.pColorBlendState = &colorBlendStateCreateInfo;

        return colorBlendAttachments.back();
    }

    void Pipeline::SetMultisample(VkSampleCountFlagBits samples)
    {
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = samples;
        multisampleStateCreateInfo.minSampleShading = 1.0f;          // Optional
        multisampleStateCreateInfo.pSampleMask = nullptr;            // Optional
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;      // Optional

        createInfo.pMultisampleState = &multisampleStateCreateInfo;
    }

    void Pipeline::SetRasterization(VkFrontFace frontFace,
                                    VkCullModeFlags cullMode)
    {
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.lineWidth = 1.0f;
        rasterizationStateCreateInfo.cullMode = cullMode;
        rasterizationStateCreateInfo.frontFace = frontFace;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

        createInfo.pRasterizationState = &rasterizationStateCreateInfo;
    }

    void Pipeline::SetInputAssembly(VkPrimitiveTopology topology)
    {
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = topology;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        createInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    }

    void Pipeline::SetVertexInput()
    {
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; // Optional

        createInfo.pVertexInputState = &vertexInputStateCreateInfo;
    }

    void Pipeline::SetRenderPass(const gl::RenderPass &renderPass)
    {
        createInfo.renderPass = renderPass.handle;
    }

    void Pipeline::SetDepthStencil()
    {
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f; // Optional
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f; // Optional
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.front = {}; // Optional
        depthStencilStateCreateInfo.back = {};  // Optional
        createInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    }

    VkVertexInputBindingDescription &Pipeline::AddVertexInputBindingDescription(std::uint32_t binding, VkVertexInputRate inputRate)
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = 0;
        bindingDescription.inputRate = inputRate;

        vertexInputBindingDescriptions.push_back(bindingDescription);
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
        return vertexInputBindingDescriptions.back();
    }

    VkVertexInputAttributeDescription &Pipeline::AddVertexInputAttributeDescription(std::uint32_t binding, std::uint32_t location, VkFormat format, std::uint32_t offset)
    {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = binding;
        attributeDescription.location = location;
        attributeDescription.format = format;
        attributeDescription.offset = offset;

        vertexInputAttributeDescriptions.push_back(attributeDescription);
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
        return vertexInputAttributeDescriptions.back();
    }

    int Pipeline::AddDescriptorSetLayout()
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 0;
        layoutInfo.pBindings = nullptr;

        descriptorSetLayoutCreateInfos.push_back(layoutInfo);

        return descriptorSetLayoutCreateInfos.size() - 1;
    }

    VkDescriptorSetLayoutBinding &Pipeline::AddDescriptorSetLayoutBinding(int descriptorSetLayout, int binding, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = type;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;

        if (descriptorSetLayoutBindings.find(descriptorSetLayout) == descriptorSetLayoutBindings.end())
            descriptorSetLayoutBindings[descriptorSetLayout] = std::vector<VkDescriptorSetLayoutBinding>();
        descriptorSetLayoutBindings[descriptorSetLayout].push_back(layoutBinding);

        descriptorSetLayoutCreateInfos[descriptorSetLayout].bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings[descriptorSetLayout].size());
        descriptorSetLayoutCreateInfos[descriptorSetLayout].pBindings = descriptorSetLayoutBindings[descriptorSetLayout].data();

        return descriptorSetLayoutBindings[descriptorSetLayout].back();
    }
}
