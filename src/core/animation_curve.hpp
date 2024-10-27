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
    void SetKeyframe(int anchor, float t, float v);
    void SetPoint(int i, float t, float v);
    inline float Time() const
    {
        return points.back().x - points.front().x;
    }
    const Vec2 &operator[](int anchor) const;
    int Anchors() const { return points.size() / 3 + 1; }
    inline int Segments() const { return points.size() / 3; }

private:
    float interpolate(int anchorA, int anchorB, float t) const;

private:
    std::vector<Vec2> points;
};