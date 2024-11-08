#include "input.hpp"

bool Input::KeyJustReleased(int key)
{
    if (KeyDown(key))
        wasDown[key] = true;
    else if (wasDown.find(key) != wasDown.end() && wasDown[key])
    {
        wasDown[key] = false;
        return true;
    }

    return false;
}

bool Input::KeyDown(int key)
{
    const auto val = inputs.find(key);
    if (val != inputs.end())
    {
        return val->second;
    }
    return false;
}
