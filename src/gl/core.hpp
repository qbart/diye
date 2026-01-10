#pragma once

#include "../deps/fmt.hpp"
#include "../deps/sdl.hpp"
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

namespace gl
{
using CStrings = std::vector<const char *>;
using Strings  = std::vector<std::string>;
} // namespace gl