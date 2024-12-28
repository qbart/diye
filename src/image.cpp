#include "image.hpp"
#include <fmt/core.h>

Image::Image() : Width(0), Height(0), Channels(0), data(nullptr), surface(nullptr)
{
}

Image::~Image()
{
    Unload();
}

bool Image::Load(const std::string &filename)
{
    surface = IMG_Load(filename.c_str());
    if (surface == nullptr)
    {
        fmt::print("Failed to load image {}\n", filename);
        return false;
    }
    Width = surface->w;
    Height = surface->h;
    Channels = surface->format->BytesPerPixel;

    data = static_cast<uint8 *>(surface->pixels);

    return true;
}

void Image::Unload()
{
    if (data != nullptr)
    {
        SDL_FreeSurface(surface);
        surface = nullptr;
        data = nullptr;
    }
}