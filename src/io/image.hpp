#pragma once

#include "../core/all.hpp"
#include "../deps/sdl.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

namespace io
{
    class Image
    {
    public:
        Image();
        ~Image();

        bool Load(const std::string &filename);
        void Unload();

        inline unsigned char *GetPixelData() const
        {
            return data;
        }

        VkDeviceSize Size() const
        {
            return Width * Height * Channels;
        }

        VkExtent2D Extent() const
        {
            return {
                static_cast<uint32_t>(Width),
                static_cast<uint32_t>(Height)};
        }

    public:
        int32 Width, Height, Channels;

    private:
        unsigned char *data = nullptr;
        SDL_Surface *surface = nullptr;
    };
}