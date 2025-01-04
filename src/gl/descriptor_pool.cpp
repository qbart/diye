#include "descriptor_pool.hpp"

namespace gl
{
    DescriptorSet::DescriptorSet() : handle(VK_NULL_HANDLE)
    {
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
        descriptorWrite.pBufferInfo = &bufferInfos.back();
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

    DescriptorPool::DescriptorPool() : createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.poolSizeCount = 0;
        createInfo.pPoolSizes = nullptr;
        createInfo.maxSets = 0;
    }

    bool DescriptorPool::Create(const Device &device)
    {
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

}