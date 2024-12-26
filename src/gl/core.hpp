#pragma once

#include <vector>
#include <string>
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include "../deps/fmt.hpp"
#include "../deps/sdl.hpp"

namespace gl
{
    using CStrings = std::vector<const char *>;
    using Strings = std::vector<std::string>;
}