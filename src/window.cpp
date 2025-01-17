#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "deps/imgui.hpp"
#include "io/binary.hpp"
#include "io/obj.hpp"
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
    auto msaaSamples = physicalDevice.GetMaxUsableSampleCount();

    device.RequireSwapchainExtension();
    device.EnableValidationLayers();
    if (!device.Create(physicalDevice))
        return false;

    if (!swapChain.Create(device, surface, physicalDevice))
        return false;

    imageViews.resize(swapChain.images.size());
    for (size_t i = 0; i < swapChain.images.size(); i++)
    {
        if (!imageViews[i].Create(device, swapChain.images[i], swapChain.imageFormat))
            return false;
    }

    color.Samples(msaaSamples);
    color.Usage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    if (!color.Create(device, swapChain.extent, swapChain.imageFormat))
        return false;
    if (!colorMemory.Allocate(physicalDevice, device, color.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;
    color.BindMemory(device, colorMemory, 0);
    if (!colorView.Create(device, color, swapChain.imageFormat))
        return false;

    depthImage.Samples(msaaSamples);
    depthImage.UsageDepthOnly();
    if (!depthImage.Create(device, swapChain.extent, physicalDevice.depthFormat))
        return false;
    if (!depthImageMemory.Allocate(physicalDevice, device, depthImage.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;
    depthImage.BindMemory(device, depthImageMemory, 0);
    depthImageView.AspectMaskDepth();
    if (!depthImageView.Create(device, depthImage, physicalDevice.depthFormat))
        return false;

    shaderModules.vert = gl::CreateShaderModule(device, io::BinaryFile::Load("dummy.vert.spv")->Bytes());
    shaderModules.frag = gl::CreateShaderModule(device, io::BinaryFile::Load("dummy.frag.spv")->Bytes());
    if (shaderModules.vert == VK_NULL_HANDLE || shaderModules.frag == VK_NULL_HANDLE)
    {
        fmtx::Error("Failed to create shader modules");
        return false;
    }
    renderPass.AddColorAttachment(swapChain.imageFormat);
    renderPass.SetDepthAttachment(physicalDevice.depthFormat);
    if (!renderPass.Create(device, shaderModules))
        return false;

    swapChainFramebuffers.resize(swapChain.images.size());
    for (auto i = 0; i < swapChain.images.size(); i++)
    {
        swapChainFramebuffers[i].ClearAttachments();
        swapChainFramebuffers[i].AddAttachment(colorView);
        swapChainFramebuffers[i].AddAttachment(depthImageView);
        swapChainFramebuffers[i].AddAttachment(imageViews[i]);
        if (!swapChainFramebuffers[i].Create(device, renderPass, swapChain.extent))
            return false;
    }

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
    graphicsPipeline.SetMultisample(msaaSamples);
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

    if (!commandPool.Create(device, physicalDevice.queueFamilyIndices.graphicsFamily.value()))
        return false;

    shortLivedCommandPool.TransientOnly();
    if (!shortLivedCommandPool.Create(device, physicalDevice.queueFamilyIndices.graphicsFamily.value()))
        return false;

    if (!commandBuffers.Allocate(device, commandPool, MAX_FRAMES_IN_FLIGHT))
        return false;

    if (!imageAvailableSemaphores.Create(device, MAX_FRAMES_IN_FLIGHT))
        return false;

    if (!renderFinishedSemaphores.Create(device, MAX_FRAMES_IN_FLIGHT))
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

    io::OBJ obj;
    if (!obj.Load("viking_room.obj"))
    {
        fmtx::Error("Failed to load obj");
        return false;
    }
    auto mesh = obj.GetMesh();
    vertices.clear();
    indices.clear();
    vertices.reserve(mesh.vertices.size());
    indices.reserve(mesh.indices.size());
    for (const auto &v : mesh.vertices)
        vertices.push_back({v.pos, v.color, v.texCoord});

    for (const auto &i : mesh.indices)
        indices.push_back(i);

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

    gl::CopyBuffer(device, shortLivedCommandPool.handle, device.graphicsQueue.handle, stagingBuffer, vertexBuffer, stagingBuffer.Size());

    stagingBuffer.Destroy(device);
    stagingBufferMemory.Free(device);

    indexStagingBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (!indexStagingBuffer.Create(device, sizeof(indices[0]) * indices.size()))
        return false;
    auto indexMemRequirements = indexStagingBuffer.MemoryRequirements(device);
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

    gl::CopyBuffer(device, shortLivedCommandPool.handle, device.graphicsQueue.handle, indexStagingBuffer, indexBuffer, indexStagingBuffer.Size());

    indexStagingBuffer.Destroy(device);
    indexStagingBufferMemory.Free(device);

    gl::Buffer imageStagingBuffer;
    gl::Memory imageStagingMemory;
    io::Image rawImage;
    if (!rawImage.Load("viking_room.png"))
    {
        fmtx::Error("Failed to load image");
        return false;
    }

    imageStagingBuffer.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if (!imageStagingBuffer.Create(device, rawImage.Size()))
        return false;

    if (!imageStagingMemory.Allocate(physicalDevice, device, imageStagingBuffer.MemoryRequirements(device), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        return false;

    imageStagingBuffer.BindMemory(device, imageStagingMemory, 0);
    imageStagingMemory.Map(device, 0, rawImage.Size());
    imageStagingMemory.CopyRaw(device, rawImage.GetPixelData(), rawImage.Size());
    imageStagingMemory.Unmap(device);

    texture.MipLevels(rawImage.RecommendedMipLevels());
    texture.Usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    if (!texture.Create(device, rawImage.Extent(), VK_FORMAT_R8G8B8A8_SRGB))
        return false;
    if (!textureMemory.Allocate(physicalDevice, device, texture.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        return false;

    texture.BindMemory(device, textureMemory, 0);
    texture.TransitionLayout(device, shortLivedCommandPool, device.graphicsQueue.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    texture.CopyFromBuffer(device, shortLivedCommandPool, device.graphicsQueue.handle, imageStagingBuffer, rawImage.Extent());
    // texture.TransitionLayout(device, shortLivedCommandPool, device.graphicsQueue.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    imageStagingBuffer.Destroy(device);
    imageStagingMemory.Free(device);

    texture.GenerateMipmaps(device, shortLivedCommandPool, device.graphicsQueue.handle, physicalDevice.TrySampledImageFilterLinear(VK_FORMAT_R8G8B8A8_SRGB));

    if (!textureView.Create(device, texture, VK_FORMAT_R8G8B8A8_SRGB))
        return false;

    textureSampler.MaxAnisotropy(physicalDevice);
    textureSampler.LinearFilter();
    textureSampler.LinearMipmap();
    textureSampler.MaxLod(rawImage.RecommendedMipLevels());
    if (!textureSampler.Create(device))
        return false;

    descriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
    descriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT);
    descriptorPool.MaxSets(MAX_FRAMES_IN_FLIGHT);

    if (!descriptorPool.Create(device))
        return false;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(2, graphicsPipeline.descriptorSetLayouts[0]);
    if (!descriptorPool.Allocate(device, descriptorSetLayouts, MAX_FRAMES_IN_FLIGHT))
        return false;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        descriptorPool.descriptorSets[i].WriteUniformBuffer(0, uniformBuffers[i], 0, sizeof(UniformBufferObject));
        descriptorPool.descriptorSets[i].WriteCombinedImageSampler(1, textureView, textureSampler);
        device.UpdateDescriptorSets(descriptorPool.descriptorSets[i].writes);
    }
    fmtx::Info("Descriptor sets updated");

    return true;
}

void Window::ShutdownGL()
{
    device.WaitIdle();

    textureSampler.Destroy(device);
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
    for (int i = 0; i < swapChainFramebuffers.size(); i++)
        swapChainFramebuffers[i].Destroy(device);
    graphicsPipeline.Destroy(device);
    graphicsPipeline.DestroyLayout(device);
    descriptorPool.Destroy(device);
    graphicsPipeline.DestroyDescriptorSetLayouts(device);
    renderPass.Destroy(device);
    gl::DestroyShaderModule(device, shaderModules.vert);
    gl::DestroyShaderModule(device, shaderModules.frag);
    depthImageView.Destroy(device);
    depthImage.Destroy(device);
    depthImageMemory.Free(device);
    colorView.Destroy(device);
    color.Destroy(device);
    colorMemory.Free(device);
    for (auto i = 0; i < imageViews.size(); i++)
        imageViews[i].Destroy(device);
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
    for (int i = 0; i < swapChainFramebuffers.size(); i++)
        swapChainFramebuffers[i].Destroy(device);

    colorView.Destroy(device);
    color.Destroy(device);
    colorMemory.Free(device);

    depthImageView.Destroy(device);
    depthImage.Destroy(device);
    depthImageMemory.Free(device);

    for (auto i = 0; i < imageViews.size(); i++)
        imageViews[i].Destroy(device);

    swapChainFramebuffers.clear();
    imageViews.clear();
    swapChain.Destroy(device);

    // recreate swap chain
    physicalDevice.QuerySwapChainSupport(surface);

    if (!swapChain.Create(device, surface, physicalDevice))
    {
        fmtx::Error("Failed to recreate swap chain");
    }

    // recreate image views
    imageViews.resize(swapChain.images.size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        if (!imageViews[i].Create(device, swapChain.images[i], swapChain.imageFormat))
            fmtx::Error("Failed to recreate image views");
    }

    // recreate color image
    if (!color.Create(device, swapChain.extent, swapChain.imageFormat))
        fmtx::Error("Failed to recreate color image");
    if (!colorMemory.Allocate(physicalDevice, device, color.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        fmtx::Error("Failed to allocate color image memory");
    color.BindMemory(device, colorMemory, 0);
    if (!colorView.Create(device, color, swapChain.imageFormat))
        fmtx::Error("Failed to recreate color image view");

    // recreate depth image
    depthImage.UsageDepthOnly();
    if (!depthImage.Create(device, swapChain.extent, physicalDevice.depthFormat))
        fmtx::Error("Failed to recreate depth image");
    if (!depthImageMemory.Allocate(physicalDevice, device, depthImage.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        fmtx::Error("Failed to allocate depth image memory");
    depthImage.BindMemory(device, depthImageMemory, 0);
    depthImageView.AspectMaskDepth();
    if (!depthImageView.Create(device, depthImage, physicalDevice.depthFormat))
        fmtx::Error("Failed to recreate depth image view");

    // recreate framebuffers
    swapChainFramebuffers.resize(swapChain.images.size());
    for (int i = 0; i < swapChainFramebuffers.size(); i++)
    {
        swapChainFramebuffers[i].ClearAttachments();
        swapChainFramebuffers[i].AddAttachment(colorView);
        swapChainFramebuffers[i].AddAttachment(depthImageView);
        swapChainFramebuffers[i].AddAttachment(imageViews[i]);
        if (!swapChainFramebuffers[i].Create(device, renderPass, swapChain.extent))
            fmtx::Error("Failed to recreate framebuffers");
    }
}

void Window::Swap()
{
    if (!active)
        return;

    inFlightFences.Wait(currentFrame, device);
    uint32_t imageIndex;
    VkResult nextResult = swapChain.AcquireNextImage(device, &imageIndex, imageAvailableSemaphores.handles[currentFrame]);
    if (nextResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        fmtx::Warn("Vulkan acquire next image returned out of date");
        RecreateSwapChain();
        return;
    }
    else if (nextResult != VK_SUCCESS && nextResult != VK_SUBOPTIMAL_KHR)
    {
        fmtx::Error("Failed to acquire swap chain image");
        Close();
        return;
    }
    inFlightFences.Reset(currentFrame, device);

    commandBuffers.Reset(currentFrame);
    if (commandBuffers.Begin(currentFrame) != VK_SUCCESS)
    {
        fmtx::Error("Failed to begin recording command buffer");
        active = false;
        return;
    }

    float time = SDL_GetTicks() / 1000.0f;
    auto model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto proj = glm::perspective(glm::radians(45.0f), swapChain.extent.width / (float)swapChain.extent.height, 0.1f, 10.0f);
    proj[1][1] *= -1;
    ubos[currentFrame].mvp = proj * view * model;

    uniformBuffersMemory[currentFrame].CopyRaw(device, &ubos[currentFrame], sizeof(ubos[currentFrame]));
    commandBuffers.ClearColor({0.0f, 0.0f, 0.0f, 1.0f});
    commandBuffers.ClearDepthStencil();
    commandBuffers.CmdBeginRenderPass(currentFrame, renderPass, swapChainFramebuffers[imageIndex], swapChain.extent);
    commandBuffers.CmdBindGraphicsPipeline(currentFrame, graphicsPipeline);
    commandBuffers.CmdViewport(currentFrame, {0, 0}, swapChain.extent);
    commandBuffers.CmdScissor(currentFrame, {0, 0}, swapChain.extent);
    commandBuffers.CmdBindDescriptorSet(currentFrame, graphicsPipeline, descriptorPool.descriptorSets[currentFrame].handle);
    commandBuffers.CmdBindVertexBuffer(currentFrame, vertexBuffer);
    commandBuffers.CmdBindIndexBuffer(currentFrame, indexBuffer);
    commandBuffers.CmdDrawIndexed(currentFrame, static_cast<uint32_t>(indices.size()));

    commandBuffers.CmdEndRenderPass(currentFrame);
    if (commandBuffers.End(currentFrame) != VK_SUCCESS)
    {
        fmtx::Error("Failed to record command buffer");
        Close();
        return;
    }
    // end

    device.graphicsQueue.Clear();
    device.graphicsQueue.AddWaitSemaphore(imageAvailableSemaphores.handles[currentFrame]);
    device.graphicsQueue.AddSignalSemaphore(renderFinishedSemaphores.handles[currentFrame]);
    device.graphicsQueue.AddWaitStage(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    if (device.graphicsQueue.Submit(commandBuffers.handles[currentFrame], inFlightFences.handles[currentFrame]) != VK_SUCCESS)
    {
        fmtx::Error("Failed to submit draw command buffer");
        Close();
        return;
    }

    device.presentQueue.Clear();
    device.presentQueue.AddWaitSemaphore(renderFinishedSemaphores.handles[currentFrame]);
    device.presentQueue.AddSwapChain(swapChain.handle);
    device.presentQueue.AddImageIndex(imageIndex);
    VkResult presentResult = device.presentQueue.Present();

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
        fmtx::Error("Failed to present swap chain image");
        Close();
        return;
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
    active = false;
}

void Window::Debug()
{
    // glEnable(GL_DEBUG_OUTPUT);
    // gl_printInfo();
    // gl_bindDebugCallback();
}
