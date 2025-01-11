#include "sampler.hpp"

namespace gl
{
    Sampler::Sampler() : createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1;
        createInfo.unnormalizedCoordinates = VK_FALSE;
        createInfo.compareEnable = VK_FALSE;
        createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.mipLodBias = 0.0f;
        createInfo.minLod = 0.0f;
        createInfo.maxLod = 0.0f;
    }

    bool Sampler::Create(const Device &device)
    {
        if (vkCreateSampler(device.handle, &createInfo, nullptr, &handle) == VK_SUCCESS)
            return true;

        fmtx::Error("failed to create sampler");
        return false;
    }

    void Sampler::Destroy(const Device &device)
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroySampler(device.handle, handle, nullptr);
    }

    void Sampler::MaxAnisotropy(const PhysicalDevice &physicalDevice)
    {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = physicalDevice.properties.limits.maxSamplerAnisotropy;
    }

    void Sampler::MinFilter(VkFilter filter)
    {
        createInfo.minFilter = filter;
    }

    void Sampler::MagFilter(VkFilter filter)
    {
        createInfo.magFilter = filter;
    }


    void Sampler::MipmapMode(VkSamplerMipmapMode mode)
    {
        createInfo.mipmapMode = mode;
    }

    void Sampler::MinLod(float lod)
    {
        createInfo.minLod = lod;
    }

    void Sampler::MaxLod(float lod)
    {
        createInfo.maxLod = lod;
    }

    void Sampler::LinearFilter()
    {
        MinFilter(VK_FILTER_LINEAR);
        MagFilter(VK_FILTER_LINEAR);
    }

    void Sampler::LinearMipmap()
    {
        MipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    }
}