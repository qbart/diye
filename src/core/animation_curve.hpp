#pragma once

#include "types.hpp"
#include "math.hpp"

class AnimationCurve
{

public:
    AnimationCurve();
    void AddKey(float time, float value);
    float Evaluate(float t) const;
    const std::vector<Vec2> &Points() const { return points; }
    void SetKeyframe(int i, float t, float v);
    inline float Time() const
    {
        return points.back().x - points.front().x;
    }

private:
    float interpolate(int anchorA, int anchorB, float t) const;
    void sort();

    std::vector<Vec2> points;
};