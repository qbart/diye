#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "deps/imgui.hpp"
#include "io/binary.hpp"
#include "deps/fmt.hpp"
#include <array>

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
    instance.EnableValidationLayers();
    if (!instance.Create())
        return false;

    if (!surface.Create(instance, wnd))
        return false;

    auto devices = gl::GetPhysicalDevices(instance, surface);
    physicalDevice = gl::SelectBestPhysicalDevice(devices);
    if (!physicalDevice.IsValid())
    {
        fmtx::Error("Failed to select physical device");
        return false;
    }

    device.RequireSwapchainExtension();
    device.EnableValidationLayers();
    if (!device.Create(physicalDevice))
        return false;

    if (!swapChain.Create(device, surface, physicalDevice))
        return false;

    imageViews = device.CreateImageViews(swapChain.imageFormat, swapChain.images);
    if (imageViews.empty())
    {
        fmtx::Error("Failed to create image views");
        return false;
    }
    depthImage.Usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (!depthImage.Create(device, swapChain.extent, physicalDevice.depthFormat))
        return false;
    if (!depthImageMemory.Allocate(physicalDevice, device, depthImage.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;
    depthImage.BindMemory(device, depthImageMemory, 0);
    depthImageView.AspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
    if (!depthImageView.Create(device, depthImage, physicalDevice.depthFormat))
        return false;

    shaderModules.vert = gl::CreateShaderModule(device, BinaryFile::Load("dummy.vert.spv")->Bytes());
    shaderModules.frag = gl::CreateShaderModule(device, BinaryFile::Load("dummy.frag.spv")->Bytes());
    if (shaderModules.vert == VK_NULL_HANDLE || shaderModules.frag == VK_NULL_HANDLE)
    {
        fmtx::Error("Failed to create shader modules");
        return false;
    }
    renderPass.AddColorAttachment(swapChain.imageFormat);
    renderPass.SetDepthAttachment(physicalDevice.depthFormat);
    if (!renderPass.Create(device, shaderModules))
        return false;

    VkDeviceSize uboBufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    ubos.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers[i].Usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        if (!uniformBuffers[i].Create(device, uboBufferSize))
            return false;
        VkMemoryRequirements uboMemRequirements = uniformBuffers[i].MemoryRequirements(device);
        if (!uniformBuffersMemory[i].Allocate(physicalDevice, device, uboMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            return false;
        uniformBuffers[i].BindMemory(device, uniformBuffersMemory[i], 0);
        uniformBuffersMemory[i].Map(device, 0, uboBufferSize);
    }

    graphicsPipeline.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, shaderModules.vert);
    graphicsPipeline.AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules.frag);
    graphicsPipeline.AddDynamicViewport();
    graphicsPipeline.AddDynamicScissor();
    graphicsPipeline.AddColorBlendAttachment();
    graphicsPipeline.SetDepthStencil();
    graphicsPipeline.SetMultisample();
    graphicsPipeline.SetRasterization(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    graphicsPipeline.SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    graphicsPipeline.SetVertexInput();
    graphicsPipeline.SetRenderPass(renderPass);
    graphicsPipeline.AddVertexInputBindingDescription(0).stride = sizeof(Vertex);
    graphicsPipeline.AddVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
    graphicsPipeline.AddVertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
    graphicsPipeline.AddVertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv));
    int setLayout = graphicsPipeline.AddDescriptorSetLayout();
    graphicsPipeline.AddDescriptorSetLayoutBinding(setLayout, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    graphicsPipeline.AddDescriptorSetLayoutBinding(setLayout, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

    if (!graphicsPipeline.CreateDescriptorSetLayouts(device))
        return false;

    if (!graphicsPipeline.CreateLayout(device))
        return false;

    if (!graphicsPipeline.Create(device))
        return false;

    swapChainFramebuffers = device.CreateFramebuffers(renderPass, imageViews, swapChain.extent);
    if (swapChainFramebuffers.empty())
    {
        fmtx::Error("failed to create framebuffers");
        return false;
    }

    if (!commandPool.Create(device, physicalDevice.queueFamilyIndices.graphicsFamily.value()))
        return false;

    shortLivedCommandPool.TransientOnly();
    if (!shortLivedCommandPool.Create(device, physicalDevice.queueFamilyIndices.graphicsFamily.value()))
        return false;

    if (!commandBuffers.Allocate(device, commandPool, 2))
        return false;

    if (!imageAvailableSemaphores.Create(device, 2))
        return false;

    if (!renderFinishedSemaphores.Create(device, 2))
        return false;

    if (!inFlightFences.Create(device, 2))
        return false;

    vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};
    indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4};

    stagingBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (!stagingBuffer.Create(device, sizeof(vertices[0]) * vertices.size()))
        return false;
    VkMemoryRequirements memRequirements = stagingBuffer.MemoryRequirements(device);
    if (!stagingBufferMemory.Allocate(physicalDevice, device, memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        return false;

    stagingBuffer.BindMemory(device, stagingBufferMemory, 0);
    stagingBufferMemory.Map(device, 0, stagingBuffer.Size());
    stagingBufferMemory.CopyRaw(device, vertices.data(), stagingBuffer.Size());
    stagingBufferMemory.Unmap(device);

    vertexBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!vertexBuffer.Create(device, stagingBuffer.Size()))
        return false;
    if (!vertexBufferMemory.Allocate(physicalDevice, device, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;
    vertexBuffer.BindMemory(device, vertexBufferMemory, 0);

    gl::CopyBuffer(device, shortLivedCommandPool.handle, device.graphicsQueue, stagingBuffer, vertexBuffer, stagingBuffer.Size());

    stagingBuffer.Destroy(device);
    stagingBufferMemory.Free(device);

    indexStagingBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (!indexStagingBuffer.Create(device, sizeof(indices[0]) * indices.size()))
        return false;
    VkMemoryRequirements indexMemRequirements = indexStagingBuffer.MemoryRequirements(device);
    if (!indexStagingBufferMemory.Allocate(physicalDevice, device, indexMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        return false;

    indexStagingBuffer.BindMemory(device, indexStagingBufferMemory, 0);
    indexStagingBufferMemory.Map(device, 0, indexStagingBuffer.Size());
    indexStagingBufferMemory.CopyRaw(device, indices.data(), indexStagingBuffer.Size());
    indexStagingBufferMemory.Unmap(device);

    indexBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (!indexBuffer.Create(device, indexStagingBuffer.Size()))
        return false;
    if (!indexBufferMemory.Allocate(physicalDevice, device, indexMemRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;

    indexBuffer.BindMemory(device, indexBufferMemory, 0);

    gl::CopyBuffer(device, shortLivedCommandPool.handle, device.graphicsQueue, indexStagingBuffer, indexBuffer, indexStagingBuffer.Size());

    indexStagingBuffer.Destroy(device);
    indexStagingBufferMemory.Free(device);

    gl::Buffer imageStagingBuffer;
    gl::Memory imageStagingMemory;
    Image rawImage;
    if (!rawImage.Load("texture.png"))
    {
        fmtx::Error("Failed to load image");
        return false;
    }
    imageStagingBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (!imageStagingBuffer.Create(device, rawImage.Size()))
        return false;

    auto imageMemRequirements = imageStagingBuffer.MemoryRequirements(device);
    if (!imageStagingMemory.Allocate(physicalDevice, device, imageMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        return false;

    imageStagingBuffer.BindMemory(device, imageStagingMemory, 0);
    imageStagingMemory.Map(device, 0, rawImage.Size());
    imageStagingMemory.CopyRaw(device, rawImage.GetPixelData(), rawImage.Size());
    imageStagingMemory.Unmap(device);

    texture.Usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    if (!texture.Create(device, rawImage.Extent(), VK_FORMAT_R8G8B8A8_SRGB))
        return false;

    if (!textureMemory.Allocate(physicalDevice, device, imageMemRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;

    texture.BindMemory(device, textureMemory, 0);
    texture.TransitionLayout(device, shortLivedCommandPool, device.graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    texture.CopyFromBuffer(device, shortLivedCommandPool, device.graphicsQueue, imageStagingBuffer, rawImage.Extent());
    texture.TransitionLayout(device, shortLivedCommandPool, device.graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    imageStagingBuffer.Destroy(device);
    imageStagingMemory.Free(device);

    if (!textureView.Create(device, texture, VK_FORMAT_R8G8B8A8_SRGB))
        return false;

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = physicalDevice.properties.limits.maxSamplerAnisotropy;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    if (vkCreateSampler(device.handle, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        fmtx::Error("failed to create texture sampler");
        return false;
    }

    descriptorPoolSize.push_back({});
    descriptorPoolSize.push_back({});
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize.data();
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device.handle, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        fmtx::Error("failed to create descriptor pool");
        return false;
    }
    fmtx::Info("Descriptor pool created");

    std::vector<VkDescriptorSetLayout> descriptorLayouts(2, graphicsPipeline.descriptorSetLayouts[0]);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocInfo.pSetLayouts = descriptorLayouts.data();

    descriptorSets.resize(2);
    if (vkAllocateDescriptorSets(device.handle, &descriptorSetAllocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        fmtx::Error("failed to allocate descriptor sets");
        return false;
    }
    fmtx::Info("Descriptor sets allocated");

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].handle;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureView.handle;
        imageInfo.sampler = textureSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrite{};
        descriptorWrite.reserve(2);
        descriptorWrite.push_back({});
        descriptorWrite.push_back({});

        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = descriptorSets[i];
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferInfo;
        descriptorWrite[0].pImageInfo = nullptr;
        descriptorWrite[0].pTexelBufferView = nullptr;

        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = descriptorSets[i];
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pBufferInfo = nullptr;
        descriptorWrite[1].pImageInfo = &imageInfo;
        descriptorWrite[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device.handle, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
    }
    fmtx::Info("Descriptor sets updated");

    return true;
}

void Window::ShutdownGL()
{
    device.WaitIdle();

    vkDestroySampler(device.handle, textureSampler, nullptr);
    textureView.Destroy(device);
    texture.Destroy(device);
    textureMemory.Free(device);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers[i].Destroy(device);
        uniformBuffersMemory[i].Free(device);
    }
    indexBuffer.Destroy(device);
    indexBufferMemory.Free(device);
    vertexBuffer.Destroy(device);
    vertexBufferMemory.Free(device);
    imageAvailableSemaphores.Destroy(device);
    renderFinishedSemaphores.Destroy(device);
    inFlightFences.Destroy(device);
    shortLivedCommandPool.Destroy(device);
    commandPool.Destroy(device);
    device.DestroyFramebuffers(swapChainFramebuffers);
    graphicsPipeline.Destroy(device);
    graphicsPipeline.DestroyLayout(device);
    vkDestroyDescriptorPool(device.handle, descriptorPool, nullptr);
    graphicsPipeline.DestroyDescriptorSetLayouts(device);
    renderPass.Destroy(device);
    gl::DestroyShaderModule(device, shaderModules.vert);
    gl::DestroyShaderModule(device, shaderModules.frag);
    depthImageView.Destroy(device);
    depthImage.Destroy(device);
    depthImageMemory.Free(device);
    device.DestroyImageViews(imageViews);
    swapChain.Destroy(device);
    device.Destroy();
    surface.Destroy(instance);
    instance.Destroy();
}

void Window::RecreateSwapChain()
{
    // wait
    device.WaitIdle();

    // clean swap chain
    device.DestroyFramebuffers(swapChainFramebuffers);
    device.DestroyImageViews(imageViews);
    swapChain.Destroy(device);

    // recreate swap chain
    physicalDevice.QuerySwapChainSupport(surface);

    if (!swapChain.Create(device, surface, physicalDevice))
    {
        fmtx::Error("Failed to recreate swap chain");
    }

    // recreate image views
    imageViews = device.CreateImageViews(swapChain.imageFormat, swapChain.images);
    if (imageViews.empty())
    {
        fmtx::Error("Failed to create image views");
    }
    // recreate framebuffers
    swapChainFramebuffers = device.CreateFramebuffers(renderPass, imageViews, swapChain.extent);
    if (swapChainFramebuffers.empty())
    {
        fmtx::Error("Failed to create framebuffers");
    }
}

void Window::Swap()
{
    if (!active)
        return;

    inFlightFences.Wait(currentFrame, device);
    uint32_t imageIndex;
    VkResult nextResult = vkAcquireNextImageKHR(device.handle, swapChain.handle, UINT64_MAX, imageAvailableSemaphores.handles[currentFrame], VK_NULL_HANDLE, &imageIndex);
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
    inFlightFences.Reset(currentFrame, device);

    commandBuffers.Reset(currentFrame);
    if (commandBuffers.Begin(currentFrame) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.handle;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.extent;

    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers.handles[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers.handles[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.handle);

    commandBuffers.CmdViewport(currentFrame, {0, 0}, swapChain.extent);
    commandBuffers.CmdScissor(currentFrame, {0, 0}, swapChain.extent);

    VkBuffer vertexBuffers[] = {vertexBuffer.handle};
    VkDeviceSize offsets[] = {0};

    float time = SDL_GetTicks() / 1000.0f;
    auto model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto proj = glm::perspective(glm::radians(45.0f), swapChain.extent.width / (float)swapChain.extent.height, 0.1f, 10.0f);
    proj[1][1] *= -1;
    ubos[currentFrame].mvp = proj * view * model;

    uniformBuffersMemory[currentFrame].CopyRaw(device, &ubos[currentFrame], sizeof(ubos[currentFrame]));
    VkDescriptorSet bindDescriptorSets[] = {descriptorSets[currentFrame]};
    vkCmdBindDescriptorSets(commandBuffers.handles[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.layout, 0, 1, bindDescriptorSets, 0, nullptr);

    vkCmdBindVertexBuffers(commandBuffers.handles[currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffers.handles[currentFrame], indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffers.handles[currentFrame], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffers.handles[currentFrame]);
    if (commandBuffers.End(currentFrame) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
    // end

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores.handles[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers.handles[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores.handles[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFences.handles[currentFrame]) != VK_SUCCESS)
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

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
