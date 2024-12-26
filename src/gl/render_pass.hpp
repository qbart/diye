#pragma once

#include "core.hpp"
#include "device.hpp"
#include "shader_modules.hpp"
#include "swap_chain.hpp"

namespace gl
{
    class RenderPass
    {
    public:
        VkRenderPass handle;

        bool Create(const gl::Device &device, const gl::SwapChain &swapChain, const ShaderModules &modules);
        void Destroy(const gl::Device &device);
    };
}