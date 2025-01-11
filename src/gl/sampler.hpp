#pragma once

#include "core.hpp"
#include "device.hpp"
#include "physical_device.hpp"

namespace gl
{
    class Sampler
    {
    public:
        VkSampler handle;
        VkSamplerCreateInfo createInfo;

        Sampler();
        bool Create(const Device &device);
        void Destroy(const Device &device);
        void MaxAnisotropy(const PhysicalDevice &physicalDevice);
        void MinFilter(VkFilter filter);
        void MagFilter(VkFilter filter);
        void MipmapMode(VkSamplerMipmapMode mode);
        void MinLod(float lod);
        void MaxLod(float lod);
        void LinearFilter();
        void LinearMipmap();
    };
}