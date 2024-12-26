#include "shader_modules.hpp"

VkShaderModule gl::CreateShaderModule(const gl::Device &device, const std::vector<char> &code)
{
    VkShaderModule handle;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    if (vkCreateShaderModule(device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    else
        return handle;
}

void gl::DestroyShaderModule(const gl::Device &device, const VkShaderModule &handle)
{
    if (handle != VK_NULL_HANDLE)
        vkDestroyShaderModule(device.handle, handle, nullptr);
}
