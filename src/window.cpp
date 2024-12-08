#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "deps/imgui.hpp"
#include "io/binary.hpp"

Window::Ptr Window::New(int w, int h, const std::string &title)
{
    if (sdl::Init() != 0)
    {
        fmtx::Error("Failed to init SDL");
        return nullptr;
    }

    auto wnd = sdl::CreateWindow(title, w, h);
    if (wnd == nullptr)
    {
        fmtx::Error(sdl::GetError());
        return nullptr;
    }
    vulkan::CreateInstanceInfo info;
    info.title = title;
    info.extensions = sdl::GetVulkanExtensions(wnd, true);
    info.validationLayers = vulkan::CStrings({"VK_LAYER_KHRONOS_validation"});
    auto supportedExt = vulkan::GetSupportedInstanceExtensions();

    for (const auto &ext : supportedExt)
        fmtx::Info(fmt::format("Supported extension: {}", ext.extensionName));
    for (const auto &ext : info.extensions)
        fmtx::Info(fmt::format("Requested extension: {}", ext));

    auto instance = vulkan::CreateInstance(info, true);
    if (!instance.IsValid())
    {
        fmtx::Error("Failed to create Vulkan instance");
        return nullptr;
    }
    auto surface = vulkan::CreateSurface(instance, wnd);

    auto devices = vulkan::GetPhysicalDevices(instance, surface);
    auto physicalDevice = vulkan::SelectBestPhysicalDevice(devices);
    if (!physicalDevice.IsValid())
    {
        fmtx::Error("Failed to select physical device");
        return nullptr;
    }
    for (const auto &ext : physicalDevice.extensions)
        fmtx::Info(fmt::format("Supported device extension: {}", ext.extensionName));
    fmtx::Info(fmt::format("Selected device: {}", physicalDevice.properties.deviceName));
    fmtx::Info(fmt::format("Selected device API version: {}", physicalDevice.properties.apiVersion));
    fmtx::Info(fmt::format("Selected device driver version: {}", physicalDevice.properties.driverVersion));

    vulkan::CreateDeviceInfo deviceInfo;
    deviceInfo.physicalDevice = physicalDevice;
    deviceInfo.validationLayers = info.validationLayers;
    deviceInfo.requiredExtensions = vulkan::CStrings({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    auto device = vulkan::CreateDevice(deviceInfo);
    if (!device.IsValid())
    {
        fmtx::Error("Failed to create device");
        return nullptr;
    }
    vulkan::CreateSwapChainInfo swapChainInfo;
    swapChainInfo.surface = surface;
    swapChainInfo.physicalDevice = physicalDevice;
    swapChainInfo.device = device;
    auto swapChain = vulkan::CreateSwapChain(swapChainInfo);
    if (!swapChain.IsValid())
    {
        fmtx::Error("Failed to create swap chain");
        return nullptr;
    }
    auto imageViews = vulkan::CreateImageViews(device, swapChain);
    if (imageViews.empty())
    {
        fmtx::Error("Failed to create image views");
        return nullptr;
    }
    vulkan::ShaderModules shaderModules;
    shaderModules.vert = vulkan::CreateShaderModule(device, BinaryFile::Load("dummy.vert.spv")->Bytes());
    shaderModules.frag = vulkan::CreateShaderModule(device, BinaryFile::Load("dummy.frag.spv")->Bytes());
    if (!shaderModules.vert.IsValid() || !shaderModules.frag.IsValid())
    {
        fmtx::Error("Failed to create shader modules");
        return nullptr;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = shaderModules.vert.handle;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = shaderModules.frag.handle;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain.extent.width;
    viewport.height = (float)swapChain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.extent;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    auto renderPass = vulkan::CreateRenderPass(device, swapChain, shaderModules);
    if (!renderPass.IsValid())
    {
        fmtx::Error("Failed to create render pass");
        return nullptr;
    }

    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device.handle, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass.handle;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    std::vector<VkFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.resize(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        VkImageView attachments[] = {imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.handle;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChain.extent.width;
        framebufferInfo.height = swapChain.extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.handle, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    VkCommandPool commandPool;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(device.handle, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(2);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device.handle, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    imageAvailableSemaphores.resize(2);
    renderFinishedSemaphores.resize(2);
    inFlightFences.resize(2);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 2; i++)
    {
        if (vkCreateSemaphore(device.handle, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
        if (vkCreateSemaphore(device.handle, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
        if (vkCreateFence(device.handle, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fences!");
        }
    }

    auto ptr = std::make_shared<Window>();
    ptr->imageAvailableSemaphores = imageAvailableSemaphores;
    ptr->renderFinishedSemaphores = renderFinishedSemaphores;
    ptr->inFlightFences = inFlightFences;
    ptr->commandBuffers = commandBuffers;
    ptr->commandPool = commandPool;
    ptr->swapChainFramebuffers = swapChainFramebuffers;
    ptr->graphicsPipeline = graphicsPipeline;
    ptr->pipelineLayout = pipelineLayout;
    ptr->wnd = wnd;
    ptr->instance = instance;
    ptr->device = device;
    ptr->surface = surface;
    ptr->physicalDevice = physicalDevice;
    ptr->swapChain = swapChain;
    ptr->imageViews = imageViews;
    ptr->shaderModules = shaderModules;
    ptr->renderPass = renderPass;
    ptr->size.w = w;
    ptr->size.h = h;
    ptr->isOpen = true;

    return ptr;
}

Window::~Window()
{
    vkDeviceWaitIdle(device.handle);
    for (size_t i = 0; i < 2; i++)
    {
        vkDestroySemaphore(device.handle, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device.handle, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device.handle, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device.handle, commandPool, nullptr);

    for (auto framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device.handle, framebuffer, nullptr);
    vkDestroyPipeline(device.handle, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device.handle, pipelineLayout, nullptr);
    vulkan::DestroyRenderPass(device, renderPass);
    vulkan::DestroyShaderModule(device, shaderModules.vert);
    vulkan::DestroyShaderModule(device, shaderModules.frag);
    vulkan::DestroyImageViews(device, imageViews);
    vulkan::DestroySwapChain(device, swapChain);
    vulkan::DestroyDevice(device);
    vulkan::DestroySurface(instance, surface);
    vulkan::DestroyInstance(instance);
    sdl::DestroyWindow(wnd);
    sdl::Quit();
}

void Window::Swap()
{
    vkWaitForFences(device.handle, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device.handle, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device.handle, swapChain.handle, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(commandBuffers[currentFrame],  0);
    // record command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.handle;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.extent.width);
    viewport.height = static_cast<float>(swapChain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
    // end

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.handle};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(device.presentQueue, &presentInfo);
    currentFrame = (currentFrame + 1) % 2;
}

void Window::PollEvents()
{
    resized = false;
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            isOpen = false;
            break;

        case SDL_KEYDOWN:
            inputs[event.key.keysym.sym] = true;
            break;

        case SDL_KEYUP:
            inputs[event.key.keysym.sym] = false;
            break;

        case SDL_MOUSEMOTION:
            mousePos.x = event.motion.x;
            mousePos.y = event.motion.y;
            mouseRelPos.x = event.motion.xrel;
            mouseRelPos.y = event.motion.yrel;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseInputs[event.button.button] = true;
            break;

        case SDL_MOUSEBUTTONUP:
            mouseInputs[event.button.button] = false;
            break;

        case SDL_MOUSEWHEEL:
            mouseWheel.x = event.wheel.preciseX;
            mouseWheel.y = event.wheel.preciseY;
            lastTimeWheeled = SDL_GetTicks();
            break;

        case SDL_WINDOWEVENT_RESIZED:
            size.w = event.window.data1;
            size.h = event.window.data2;
            resized = true;
            break;
        }

        // ImGui_ImplSDL2_ProcessEvent(&event);
    }
}

void Window::Close()
{
    isOpen = false;
}

void Window::Debug()
{
    // glEnable(GL_DEBUG_OUTPUT);
    // gl_printInfo();
    // gl_bindDebugCallback();
}
