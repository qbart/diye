#pragma once

#include "core/all.hpp"
#include <unordered_map>
#include "deps/sdl.hpp"

class Input
{
public:
    Input(const HashMap<int32, bool> &keys) : inputs(keys) {};
    bool KeyJustReleased(int key);
    bool KeyDown(int key);

private:
    HashMap<int32, bool> wasDown;
    const HashMap<int32, bool> &inputs;
};