#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
    struct ShaderModules
    {
        VkShaderModule vert;
        VkShaderModule frag;
    };

    VkShaderModule CreateShaderModule(const gl::Device &device, const std::vector<char> &code);
    void DestroyShaderModule(const gl::Device &device, const VkShaderModule &handle);
}