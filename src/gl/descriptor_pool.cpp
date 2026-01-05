#include "descriptor_pool.hpp"
#include "../deps/fmt.hpp"

namespace gl
{
    DescriptorSet::DescriptorSet() : handle(VK_NULL_HANDLE)
    {
        imageInfos.reserve(10);
        bufferInfos.reserve(10);
        writes.reserve(10);
    }

    void DescriptorSet::WriteUniformBuffer(uint32_t binding, const Buffer &buffer, VkDeviceSize offset, VkDeviceSize range)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer.handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        bufferInfos.emplace_back(bufferInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = handle;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfos.back();  // pointers needs to updated later as the vector may cause reallocation
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        writes.emplace_back(descriptorWrite);
    }

    void DescriptorSet::WriteCombinedImageSampler(uint32_t binding, const ImageView &imageView, const Sampler &sampler)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView.handle;
        imageInfo.sampler = sampler.handle;
        imageInfos.emplace_back(imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = handle;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfos.back();
        descriptorWrite.pTexelBufferView = nullptr;

        writes.emplace_back(descriptorWrite);
    }

    void DescriptorSet::WriteImage(uint32_t binding, const ImageView &imageView)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = VK_NULL_HANDLE;
        imageInfo.imageView = imageView.handle;
        imageInfos.emplace_back(imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = handle;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfos.back(); // pointers needs to updated later as the vectore may cause reallocation
        descriptorWrite.pTexelBufferView = nullptr;

        writes.emplace_back(descriptorWrite);
    }

    void DescriptorSet::WriteSampler(uint32_t binding, const Sampler &sampler)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sampler = sampler.handle;
        imageInfo.imageView = VK_NULL_HANDLE;
        imageInfos.emplace_back(imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = handle;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfos.back();
        descriptorWrite.pTexelBufferView = nullptr;

        writes.emplace_back(descriptorWrite);
    }

    DescriptorPool::DescriptorPool() : createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.poolSizeCount = 0;
        createInfo.pPoolSizes = nullptr;
        createInfo.maxSets = 0;
    }

    bool DescriptorPool::Create(const Device &device)
    {
        createInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
        createInfo.pPoolSizes = descriptorPoolSize.data();

        if (vkCreateDescriptorPool(device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("failed to create descriptor pool");
            return false;
        }
        fmtx::Info("Descriptor pool created");
        return true;
    }

    void DescriptorPool::Destroy(const Device &device)
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(device.handle, handle, nullptr);
    }

    void DescriptorPool::AddPoolSize(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = count;
        descriptorPoolSize.push_back(poolSize);

        createInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
        createInfo.pPoolSizes = descriptorPoolSize.data();
    }

    void DescriptorPool::MaxSets(uint32_t count)
    {
        createInfo.maxSets = count;
    }

    bool DescriptorPool::Allocate(const Device &device, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts, uint32_t count)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = handle;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<VkDescriptorSet> allocDescriptorSets;
        allocDescriptorSets.resize(count);

        if (vkAllocateDescriptorSets(device.handle, &allocInfo, allocDescriptorSets.data()) == VK_SUCCESS)
        {
            for (int i = 0; i < count; i++)
            {
                DescriptorSet descriptorSet;
                descriptorSet.handle = allocDescriptorSets[i];
                descriptorSets.emplace_back(descriptorSet);
            }
            fmtx::Info("Descriptor sets allocated");
            return true;
        }

        fmtx::Error("failed to allocate descriptor sets");
        return false;
    }

    void DescriptorPool::UpdateDescriptorSet(const Device &device, uint32_t set)
    {
        auto& ds = descriptorSets[set];

        uint32_t imageIndex  = 0;
        uint32_t bufferIndex = 0;

        // fix all the pointers
        for (auto& write : ds.writes)
        {
            if (write.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                write.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                write.pImageInfo = &ds.imageInfos[imageIndex++];
                write.pBufferInfo = nullptr;
            }
            else if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                write.pBufferInfo = &ds.bufferInfos[bufferIndex++];
                write.pImageInfo = nullptr;
            }
            else
            {
                fmtx::Warn(fmt::format("Unhandled descriptorType: {}", uint32(write.descriptorType)));
            }
        }

        vkUpdateDescriptorSets(device.handle,
                               static_cast<uint32_t>(ds.writes.size()),
                               ds.writes.data(),
                               0,
                               nullptr);
    }

}
