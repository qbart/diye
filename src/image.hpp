#pragma once

#include "glm.hpp"
#include <vector>
#include "sdl.hpp"

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