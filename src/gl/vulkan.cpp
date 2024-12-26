#include "vulkan.hpp"
#include <map>
#include <set>
#include <algorithm>

namespace vulkan
{

    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            fmtx::Debug(fmt::format("Using fixed Extent from surface capabilities {}x{}", capabilities.currentExtent.width, capabilities.currentExtent.height));
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = sdl::GetVulkanFramebufferSize(window);
            fmtx::Debug(fmt::format("Framebuffer size: {}x{}", actualExtent.width, actualExtent.height));
            fmtx::Debug(fmt::format("Min image extent: {}x{}", capabilities.minImageExtent.width, capabilities.minImageExtent.height));
            fmtx::Debug(fmt::format("Max image extent: {}x{}", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height));

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChain CreateSwapChain(const CreateSwapChainInfo &info)
    {
        gl::SwapChainSupportDetails swapChainSupport = info.physicalDevice.swapChainSupport;
        // at this point we know that this format is available
        VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        // guaranteed to be available
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D extent = SelectSwapExtent(swapChainSupport.capabilities, info.surface.window);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = info.surface.handle;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT
        createInfo.pNext = nullptr;

        uint32_t queueFamilyIndices[] = {
            info.physicalDevice.queueFamilyIndices.graphicsFamily.value(),
            info.physicalDevice.queueFamilyIndices.presentFamily.value()};

        if (queueFamilyIndices[0] != queueFamilyIndices[1])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        SwapChain swapChain;
        if (vkCreateSwapchainKHR(info.device.handle, &createInfo, nullptr, &swapChain.handle) != VK_SUCCESS)
        {
            swapChain.handle = VK_NULL_HANDLE;
            return swapChain;
        }
        swapChain.imageFormat = surfaceFormat.format;
        swapChain.extent = extent;

        vkGetSwapchainImagesKHR(info.device.handle, swapChain.handle, &imageCount, nullptr);
        swapChain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(info.device.handle, swapChain.handle, &imageCount, swapChain.images.data());

        return swapChain;
    }

    void DestroySwapChain(const gl::Device &device, const SwapChain &swapChain)
    {
        if (swapChain.handle != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device.handle, swapChain.handle, nullptr);
    }

    std::vector<VkImageView> CreateImageViews(const gl::Device &device, const SwapChain &swapChain)
    {
        std::vector<VkImageView> views;
        std::vector<bool> valid;
        views.resize(swapChain.images.size());
        valid.resize(swapChain.images.size());
        bool allValid = true;

        for (size_t i = 0; i < swapChain.images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChain.images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChain.imageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            valid[i] = vkCreateImageView(device.handle, &createInfo, nullptr, &views[i]) == VK_SUCCESS;
            if (!valid[i])
                allValid = false;
        }

        if (!allValid)
        {
            for (size_t i = 0; i < views.size(); i++)
            {
                if (valid[i])
                    vkDestroyImageView(device.handle, views[i], nullptr);
            }
            views.clear();
        }

        return views;
    }

    void DestroyImageViews(const gl::Device &device, const std::vector<VkImageView> &views)
    {
        for (auto imageView : views)
        {
            vkDestroyImageView(device.handle, imageView, nullptr);
        }
    }

    void DestroyFramebuffers(const gl::Device &device, const std::vector<VkFramebuffer> &framebuffers)
    {
        for (auto fb : framebuffers)
        {
            vkDestroyFramebuffer(device.handle, fb, nullptr);
        }
    }

    ShaderModule CreateShaderModule(const gl::Device &device, const std::vector<char> &code)
    {
        ShaderModule mod;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            mod.handle = VK_NULL_HANDLE;
        else
            mod.handle = shaderModule;

        return mod;
    }

    void DestroyShaderModule(const gl::Device &device, const ShaderModule &module)
    {
        if (module.handle != VK_NULL_HANDLE)
            vkDestroyShaderModule(device.handle, module.handle, nullptr);
    }

    RenderPass CreateRenderPass(const gl::Device &device, const SwapChain &swapChain, const ShaderModules &modules)
    {
        RenderPass renderPass;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain.imageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &renderPass.handle) != VK_SUCCESS)
        {
            renderPass.handle = VK_NULL_HANDLE;
        }

        return renderPass;
    }

    void DestroyRenderPass(const gl::Device &device, const RenderPass &renderPass)
    {
        if (renderPass.handle != VK_NULL_HANDLE)
            vkDestroyRenderPass(device.handle, renderPass.handle, nullptr);
    }

    bool vulkan::SwapChain::IsValid() const
    {
        return handle != VK_NULL_HANDLE;
    }


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
                           vertexInputStateCreateInfo({})
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
        return vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle) == VK_SUCCESS;
    }

    void Pipeline::Destroy(const gl::Device &device)
    {
        vkDestroyPipeline(device.handle, handle, nullptr);
    }

    bool Pipeline::CreateLayout(const gl::Device &device)
    {
        bool ok = vkCreatePipelineLayout(device.handle, &layoutCreateInfo, nullptr, &layout) == VK_SUCCESS;
        if (ok)
            createInfo.layout = layout;

        return ok;
    }

    void Pipeline::DestroyLayout(const gl::Device &device)
    {
        vkDestroyPipelineLayout(device.handle, layout, nullptr);
    }

    void Pipeline::AddShaderStage(VkShaderStageFlagBits stage, const ShaderModule &module, const char *entrypoint)
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = stage;
        info.module = module.handle;
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

    void Pipeline::SetMultisample()
    {
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

    void Pipeline::SetRenderPass(const RenderPass &renderPass)
    {
        createInfo.renderPass = renderPass.handle;
    }
};