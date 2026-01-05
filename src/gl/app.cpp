#include "app.hpp"

#include "../io/binary.hpp"
#include "../io/obj.hpp"

namespace gl
{
    App::App() : wnd(nullptr),
                 needRecreateSwapChain(false),
                 imageIndex(0),
                 currentFrame(0),
                 maxFramesInFlight(2),
                 withUI(false)
    {
    }

    App::~App()
    {
    }

    void App::MaxFramesInFlight(int maxFrames)
    {
        maxFramesInFlight = maxFrames;
    }

    bool App::Init(SDL_Window *wnd)
    {
        this->wnd = wnd;
        bool ok = InitGL();
        if (!ok)
            ShutdownGL();
        return ok;
    }

    void App::Shutdown()
    {
        fmtx::Info("Shutting down Vulkan");
        ShutdownGL();
    }

    bool App::BeginFrame()
    {
        inFlightFences.Wait(currentFrame, device);
        VkResult nextResult = swapChain.AcquireNextImage(device, &imageIndex, imageAvailableSemaphores.handles[currentFrame]);
        if (nextResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            fmtx::Warn("Vulkan acquire next image returned out of date");
            RecreateSwapChain();
            return true;
        }
        else if (nextResult != VK_SUCCESS && nextResult != VK_SUBOPTIMAL_KHR)
        {
            fmtx::Error("Failed to acquire swap chain image");
            return false;
        }
        inFlightFences.Reset(currentFrame, device);

        return true;
    }

    bool App::EndFrame()
    {
        device.graphicsQueue.Clear();
        device.graphicsQueue.AddWaitSemaphore(imageAvailableSemaphores.handles[currentFrame]);
        device.graphicsQueue.AddSignalSemaphore(renderFinishedSemaphores.handles[currentFrame]);
        device.graphicsQueue.AddWaitStage(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        if (device.graphicsQueue.Submit(commandBuffers.handles[currentFrame], inFlightFences.handles[currentFrame]) != VK_SUCCESS)
        {
            fmtx::Error("Failed to submit draw command buffer");
            return false;
        }

        device.presentQueue.Clear();
        device.presentQueue.AddWaitSemaphore(renderFinishedSemaphores.handles[currentFrame]);
        device.presentQueue.AddSwapChain(swapChain.handle);
        device.presentQueue.AddImageIndex(imageIndex);
        VkResult presentResult = device.presentQueue.Present();

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || needRecreateSwapChain)
        {
            if (!needRecreateSwapChain)
                fmtx::Error(fmt::format("Vulkan queue present returned out of date"));

            needRecreateSwapChain = false;
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
            return false;
        }

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
        return true;
    }

    bool App::Render(Mat4 mvp)
    {
        commandBuffers.Reset(currentFrame);
        if (commandBuffers.Begin(currentFrame) != VK_SUCCESS)
        {
            fmtx::Error("Failed to begin recording command buffer");
            return false;
        }
        float time = SDL_GetTicks() / 1000.0f;
        ubos[currentFrame].mvp = mvp;

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
            return false;
        }

        return true;
    }

    bool App::InitGL()
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

        imageViews.resize(swapChain.images.size());
        for (size_t i = 0; i < swapChain.images.size(); i++)
        {
            if (!imageViews[i].Create(device, swapChain.images[i], swapChain.imageFormat))
                return false;
        }

        depthImage.UsageDepthOnly();
        if (!depthImage.Create(device, swapChain.extent, physicalDevice.depthFormat))
            return false;
        if (!depthImageMemory.Allocate(physicalDevice, device, depthImage.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            return false;
        depthImage.BindMemory(device, depthImageMemory, 0);
        depthImageView.AspectMaskDepth();
        if (!depthImageView.Create(device, depthImage, physicalDevice.depthFormat))
            return false;

        auto shaderVert = io::BinaryFile::Load("dummy.vert.spv");
        auto shaderFrag = io::BinaryFile::Load("dummy.frag.spv");
        if (shaderVert->IsEmpty() || shaderFrag->IsEmpty())
        {
            fmtx::Error("Failed to load shader files");
            return false;
        }
        shaderModules.vert = gl::CreateShaderModule(device, shaderVert->Bytes());
        shaderModules.frag = gl::CreateShaderModule(device, shaderFrag->Bytes());
        if (shaderModules.vert == VK_NULL_HANDLE || shaderModules.frag == VK_NULL_HANDLE)
        {
            fmtx::Error("Failed to create shader modules");
            return false;
        }
        renderPass.AddColorAttachment(swapChain.imageFormat).finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        renderPass.SetDepthAttachment(physicalDevice.depthFormat);
        if (!renderPass.Create(device, shaderModules))
            return false;

        swapChainFramebuffers.resize(swapChain.images.size());
        for (auto i = 0; i < swapChain.images.size(); i++)
        {
            swapChainFramebuffers[i].ClearAttachments();
            swapChainFramebuffers[i].AddAttachment(imageViews[i]);
            swapChainFramebuffers[i].AddAttachment(depthImageView);
            if (!swapChainFramebuffers[i].Create(device, renderPass, swapChain.extent))
                return false;
        }

        VkDeviceSize uboBufferSize = sizeof(UniformBufferObject);
        uniformBuffers.resize(maxFramesInFlight);
        uniformBuffersMemory.resize(maxFramesInFlight);
        ubos.resize(maxFramesInFlight);
        for (auto i = 0; i < maxFramesInFlight; i++)
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
        graphicsPipeline.AddDescriptorSetLayoutBinding(setLayout, 1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
        graphicsPipeline.AddDescriptorSetLayoutBinding(setLayout, 2, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

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

        if (!commandBuffers.Allocate(device, commandPool, maxFramesInFlight))
            return false;

        if (!imageAvailableSemaphores.Create(device, maxFramesInFlight))
            return false;

        if (!renderFinishedSemaphores.Create(device, maxFramesInFlight))
            return false;

        if (!inFlightFences.Create(device, maxFramesInFlight))
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

        int withUITextures = 1;
        if (withUI)
        {
            descriptorPool.createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            withUITextures = 2;
        }
        descriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxFramesInFlight);
        descriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxFramesInFlight * withUITextures);
        descriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, maxFramesInFlight);
        descriptorPool.MaxSets(maxFramesInFlight * withUITextures);

        if (!descriptorPool.Create(device))
            return false;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(maxFramesInFlight, graphicsPipeline.descriptorSetLayouts[0]);
        if (!descriptorPool.Allocate(device, descriptorSetLayouts, maxFramesInFlight))
            return false;

        for (int i = 0; i < maxFramesInFlight; i++)
        {
            descriptorPool.descriptorSets[i].WriteUniformBuffer(0, uniformBuffers[i], 0, sizeof(UniformBufferObject));
            descriptorPool.descriptorSets[i].WriteImage(1, textureView);
            descriptorPool.descriptorSets[i].WriteSampler(2, textureSampler);
            descriptorPool.UpdateDescriptorSet(device, i);
            // device.UpdateDescriptorSets(descriptorPool.descriptorSets[i].writes);
        }
        fmtx::Info("Descriptor sets updated");

        return true;
    }

    bool App::RecreateSwapChain()
    {
        bool result = true;
        // wait
        device.WaitIdle();

        // clean swap chain
        for (int i = 0; i < swapChainFramebuffers.size(); i++)
            swapChainFramebuffers[i].Destroy(device);

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
            result = false;
        }

        // recreate image views
        imageViews.resize(swapChain.images.size());
        for (size_t i = 0; i < imageViews.size(); i++)
        {
            if (!imageViews[i].Create(device, swapChain.images[i], swapChain.imageFormat))
            {
                fmtx::Error("Failed to recreate image views");
                result = false;
            }
        }

        // recreate depth image
        depthImage.UsageDepthOnly();
        if (!depthImage.Create(device, swapChain.extent, physicalDevice.depthFormat))
        {
            fmtx::Error("Failed to recreate depth image");
            result = false;
        }
        if (!depthImageMemory.Allocate(physicalDevice, device, depthImage.MemoryRequirements(device), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            fmtx::Error("Failed to allocate depth image memory");
            result = false;
        }
        depthImage.BindMemory(device, depthImageMemory, 0);
        depthImageView.AspectMaskDepth();
        if (!depthImageView.Create(device, depthImage, physicalDevice.depthFormat))
        {
            fmtx::Error("Failed to recreate depth image view");
            result = false;
        }

        // recreate framebuffers
        swapChainFramebuffers.resize(swapChain.images.size());
        for (int i = 0; i < swapChainFramebuffers.size(); i++)
        {
            swapChainFramebuffers[i].ClearAttachments();
            swapChainFramebuffers[i].AddAttachment(imageViews[i]);
            swapChainFramebuffers[i].AddAttachment(depthImageView);
            if (!swapChainFramebuffers[i].Create(device, renderPass, swapChain.extent))
            {
                fmtx::Error("Failed to recreate framebuffers");
                result = false;
            }
        }

        return result;
    }

    void App::ShutdownGL()
    {
        device.WaitIdle();

        textureSampler.Destroy(device);
        textureView.Destroy(device);
        texture.Destroy(device);
        textureMemory.Free(device);
        for (size_t i = 0; i < maxFramesInFlight; i++)
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
        for (auto i = 0; i < imageViews.size(); i++)
            imageViews[i].Destroy(device);
        swapChain.Destroy(device);
        device.Destroy();
        surface.Destroy(instance);
        instance.Destroy();
    }
}
