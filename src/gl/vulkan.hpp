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
inline constexpr float ColorGreen[4]  = {0.45f, 0.80f, 0.35f, 1.0f};
inline constexpr float ColorBlue[4]   = {0.35f, 0.55f, 0.90f, 1.0f};
inline constexpr float ColorRed[4]    = {0.90f, 0.30f, 0.30f, 1.0f};
inline constexpr float ColorYellow[4] = {0.90f, 0.85f, 0.25f, 1.0f};
inline constexpr float ColorPurple[4] = {0.70f, 0.45f, 0.85f, 1.0f};
inline constexpr float ColorGray[4]   = {0.60f, 0.60f, 0.60f, 1.0f};

void InitFunctions(VkDevice device);
void ImageTransitionLayout(VkDevice device, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
void SetObjectName(VkDevice device, uint64_t handle, VkObjectType objectType, const std::string& label);

inline PFN_vkCmdBeginRenderingKHR CmdBeginRenderingKHR = nullptr;
inline PFN_vkCmdEndRenderingKHR   CmdEndRenderingKHR   = nullptr;
inline PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT = nullptr;
inline PFN_vkCmdEndDebugUtilsLabelEXT   CmdEndDebugUtilsLabelEXT   = nullptr;
inline PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT = nullptr;
inline PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = nullptr;
}
