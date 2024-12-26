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

    auto ptr = std::make_shared<Window>();
    ptr->wnd = wnd;
    ptr->size.w = w;
    ptr->size.h = h;
    ptr->isOpen = true;
    bool ok = ptr->InitGL();
    if (!ok)
    {
        fmtx::Error("Failed to init GL");
        ptr->ShutdownGL();
        return nullptr;
    }

    return ptr;
}

Window::~Window()
{
    ShutdownGL();
    sdl::DestroyWindow(wnd);
    sdl::Quit();
}

bool Window::InitGL()
{
    instance.SetExtensions(sdl::GetVulkanExtensions(wnd, true));
    instance.SetValidationLayers();
    if (!instance.Create())
    {
        fmtx::Error("Failed to create Vulkan instance");
        return false;
    }
    surface = vulkan::CreateSurface(instance, wnd);

    auto devices = vulkan::GetPhysicalDevices(instance, surface);
    physicalDevice = vulkan::SelectBestPhysicalDevice(devices);
    if (!physicalDevice.IsValid())
    {
        fmtx::Error("Failed to select physical device");
        return false;
    }
    // for (const auto &ext : physicalDevice.extensions)
    //     fmtx::Debug(fmt::format("Supported device extension: {}", ext.extensionName));
    fmtx::Info(fmt::format("Selected device: {}", physicalDevice.properties.deviceName));
    fmtx::Info(fmt::format("Selected device API version: {}", physicalDevice.properties.apiVersion));
    fmtx::Info(fmt::format("Selected device driver version: {}", physicalDevice.properties.driverVersion));

    vulkan::CreateDeviceInfo deviceInfo;
    deviceInfo.physicalDevice = physicalDevice;
    deviceInfo.validationLayers = gl::CStrings({"VK_LAYER_KHRONOS_validation"});
    deviceInfo.requiredExtensions = gl::CStrings({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    device = vulkan::CreateDevice(deviceInfo);
    if (!device.IsValid())
    {
        fmtx::Error("Failed to create device");
        return false;
    }
    vulkan::CreateSwapChainInfo swapChainInfo;
    swapChainInfo.surface = surface;
    swapChainInfo.physicalDevice = physicalDevice;
    swapChainInfo.device = device;
    swapChain = vulkan::CreateSwapChain(swapChainInfo);
    if (!swapChain.IsValid())
    {
        fmtx::Error("Failed to create swap chain");
        return false;
    }
    imageViews = vulkan::CreateImageViews(device, swapChain);
    if (imageViews.empty())
    {
        fmtx::Error("Failed to create image views");
        return false;
    }
    shaderModules.vert = vulkan::CreateShaderModule(device, BinaryFile::Load("dummy.vert.spv")->Bytes());
    shaderModules.frag = vulkan::CreateShaderModule(device, BinaryFile::Load("dummy.frag.spv")->Bytes());
    if (!shaderModules.vert.IsValid() || !shaderModules.frag.IsValid())
    {
        fmtx::Error("Failed to create shader modules");
        return false;
    }

    renderPass = vulkan::CreateRenderPass(device, swapChain, shaderModules);
    if (!renderPass.IsValid())
    {
        fmtx::Error("Failed to create render pass");
        return false;
    }

    graphicsPipeline.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, shaderModules.vert);
    graphicsPipeline.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules.frag);
    graphicsPipeline.AddDynamicViewport();
    graphicsPipeline.AddDynamicScissor();
    graphicsPipeline.AddColorBlendAttachment();
    graphicsPipeline.SetMultisample();
    graphicsPipeline.SetRasterization(VK_FRONT_FACE_CLOCKWISE);
    graphicsPipeline.SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    graphicsPipeline.SetVertexInput();
    graphicsPipeline.SetRenderPass(renderPass);

    if (!graphicsPipeline.CreateLayout(device))
    {
        fmtx::Error("failed to create pipeline layout!");
        return false;
    }

    if (!graphicsPipeline.Create(device))
    {
        fmtx::Error("failed to create graphics pipeline!");
        return false;
    }

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
            fmtx::Error("failed to create framebuffer!");
            return false;
        }
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = physicalDevice.queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(device.handle, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        fmtx::Error("failed to create command pool!");
        return false;
    }

    commandBuffers.resize(2);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device.handle, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        fmtx::Error("failed to allocate command buffers!");
        return false;
    }

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
            fmtx::Error("failed to create semaphores!");
            return false;
        }
        if (vkCreateSemaphore(device.handle, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            fmtx::Error("failed to create semaphores!");
            return false;
        }
        if (vkCreateFence(device.handle, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            fmtx::Error("failed to create fences!");
            return false;
        }
    }

    return true;
}

void Window::ShutdownGL()
{
    device.WaitIdle();
    for (size_t i = 0; i < 2; i++)
    {
        vkDestroySemaphore(device.handle, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device.handle, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device.handle, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device.handle, commandPool, nullptr);

    vulkan::DestroyFramebuffers(device, swapChainFramebuffers);
    graphicsPipeline.Destroy(device);
    graphicsPipeline.DestroyLayout(device);
    vulkan::DestroyRenderPass(device, renderPass);
    vulkan::DestroyShaderModule(device, shaderModules.vert);
    vulkan::DestroyShaderModule(device, shaderModules.frag);
    vulkan::DestroyImageViews(device, imageViews);
    vulkan::DestroySwapChain(device, swapChain);
    vulkan::DestroyDevice(device);
    vulkan::DestroySurface(instance, surface);
    instance.Destroy();
}

void Window::RecreateSwapChain()
{
    // wait
    device.WaitIdle();

    // clean swap chain
    vulkan::DestroyFramebuffers(device, swapChainFramebuffers);
    vulkan::DestroyImageViews(device, imageViews);
    vulkan::DestroySwapChain(device, swapChain);

    // recreate swap chain
    physicalDevice.QuerySwapChainSupport(surface);

    vulkan::CreateSwapChainInfo swapChainInfo;
    swapChainInfo.surface = surface;
    swapChainInfo.physicalDevice = physicalDevice;
    swapChainInfo.device = device;
    swapChain = vulkan::CreateSwapChain(swapChainInfo);

    if (!swapChain.IsValid())
    {
        fmtx::Error("Failed to recreate swap chain");
    }

    // recreate image views
    imageViews = vulkan::CreateImageViews(device, swapChain);
    if (imageViews.empty())
    {
        fmtx::Error("Failed to create image views");
    }
    // recreate framebuffers
    swapChainFramebuffers.clear();
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
            fmtx::Error("Failed to create framebuffers");
        }
    }
}

void Window::Swap()
{
    if (!active)
        return;

    vkWaitForFences(device.handle, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    VkResult nextResult = vkAcquireNextImageKHR(device.handle, swapChain.handle, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (nextResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        fmtx::Warn("Vulkan acquire next image returned out of date");
        RecreateSwapChain();
        return;
    }
    else if (nextResult != VK_SUCCESS && nextResult != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(device.handle, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
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
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.handle);

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

    VkResult presentResult = vkQueuePresentKHR(device.presentQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || resized)
    {
        if (!resized)
            fmtx::Error(fmt::format("Vulkan queue present returned out of date"));

        resized = false;
        RecreateSwapChain();
    }
    else if (presentResult == VK_SUBOPTIMAL_KHR)
    {
        fmtx::Warn(fmt::format("Vulkan queue present returned suboptimal"));
        RecreateSwapChain();
    }
    else if (presentResult != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

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

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                fmtx::Debug("[sdl] Window resized");
                size.w = event.window.data1;
                size.h = event.window.data2;
                resized = true;
            }
            if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
            {
                fmtx::Debug("[sdl] Window focus gained");
                active = true;
            }
            else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                fmtx::Debug("[sdl] Window focus lost");
                active = false;
            }
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
