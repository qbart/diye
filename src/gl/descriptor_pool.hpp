#pragma once

#include "device.hpp"
#include "buffer.hpp"
#include "image_view.hpp"
#include "sampler.hpp"

namespace gl
{
    class DescriptorSet
    {
    public:
        VkDescriptorSet handle;
        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorImageInfo> imageInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        DescriptorSet();

        void WriteUniformBuffer(uint32_t binding, const Buffer &buffer, VkDeviceSize offset, VkDeviceSize range);
        void WriteCombinedImageSampler(uint32_t binding, const ImageView &imageView, const Sampler &sampler);
        void WriteImage(uint32_t binding, const ImageView &imageView);
        void WriteSampler(uint32_t binding, const Sampler &sampler);
    };

    class DescriptorPool
    {
    public:
        VkDescriptorPool handle;
        VkDescriptorPoolCreateInfo createInfo;
        std::vector<VkDescriptorPoolSize> descriptorPoolSize;
        std::vector<DescriptorSet> descriptorSets;

        DescriptorPool();
        bool Create(const Device &device);
        void Destroy(const Device &device);
        void AddPoolSize(VkDescriptorType type, uint32_t count);
        void MaxSets(uint32_t count);
        bool Allocate(const Device &device, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts, uint32_t count);
        void UpdateDescriptorSet(const Device &device, uint32_t set);
    };
}
