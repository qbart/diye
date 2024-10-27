#pragma once

#include "core/all.hpp"
#include "deps/sdl.hpp"

class Image
{
public:
    Image();
    ~Image();

    bool Load(const std::string &filename);

    inline unsigned char *GetPixelData() const
    {
        return data;
    }

public:
    int32 Width, Height, Channels;

private:
    unsigned char *data = nullptr;
    SDL_Surface *surface = nullptr; 
};