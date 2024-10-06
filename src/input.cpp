#include "input.hpp"

void Input::KeyCounter::Reset()
{
    value = 0;
}

void Input::KeyCounter::Inc()
{
    ++value;
    if (value > 2)
        value = 2;
}

bool Input::KeyCounter::One()
{
    return value == 1;
}

bool Input::KeyReleasedOnce(int key)
{
    if (KeyPress(key))
        keyReleaseCount[key].Reset();

    if (KeyRelease(key))
    {
        keyReleaseCount[key].Inc();
        return keyReleaseCount[key].One();
    }

    return false;
}