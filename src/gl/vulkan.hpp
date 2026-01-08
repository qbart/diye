#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <string>
#include <vector>
#include <optional>
#include "../deps/fmt.hpp"
#include "../deps/sdl.hpp"
#include "core.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "physical_device.hpp"
#include "device.hpp"
#include "queue.hpp"
#include "swap_chain.hpp"
#include "shader_modules.hpp"
#include "render_pass.hpp"
#include "pipeline.hpp"
#include "buffer.hpp"
#include "memory.hpp"
#include "command_pool.hpp"
#include "command_buffer.hpp"
#include "semaphore.hpp"
#include "fence.hpp"
#include "image.hpp"
#include "image_view.hpp"
#include "framebuffer.hpp"
#include "sampler.hpp"
#include "descriptor_pool.hpp"

namespace vk
{
void InitFunctions(VkDevice device);
void ImageTransitionLayout(VkDevice device, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

inline PFN_vkCmdBeginRenderingKHR CmdBeginRenderingKHR = nullptr;
inline PFN_vkCmdEndRenderingKHR   CmdEndRenderingKHR   = nullptr;
inline PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT = nullptr;
inline PFN_vkCmdEndDebugUtilsLabelEXT   CmdEndDebugUtilsLabelEXT   = nullptr;
inline PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT = nullptr;
inline PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = nullptr;
}
