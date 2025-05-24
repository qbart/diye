#pragma once

#include "math.hpp"

class Timer
{
public:
    inline Timer(float alert = 1) : alert(alert),
                                    accumulatedTime(0),
                                    triggered(false)

    {
    }

    inline void Update(float dt)
    {
        triggered = false;
        accumulatedTime += dt;
        if (accumulatedTime >= alert)
        {
            accumulatedTime = 0;
            triggered = true;
        }
    }

    inline float Evaluate() const
    {
        return Mathf::Clamp01(accumulatedTime / alert);
    }

    inline float EvaluateReversed() const
    {
        return Mathf::Clamp01((1*alert - accumulatedTime) / alert);
    }

    inline void Reset()
    {
        accumulatedTime = 0;
        triggered = false;
    }

    inline bool IsTriggered() const
    {
        return triggered;
    }

private:
    float accumulatedTime = 0;
    float alert = 1;
    bool triggered = false;
};

inline Timer operator""_ms(unsigned long long ms)
{
    return Timer(ms / 1000.0f);
}
