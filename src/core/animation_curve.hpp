#pragma once

#include "types.hpp"
#include "math.hpp"

class AnimationCurve
{
public:
    struct Point
    {
        Vec2 P;
        bool Locked;
    };

public:
    AnimationCurve();
    void AddKey(float time, float value);
    float Evaluate(float t) const;
    const std::vector<Point> &Points() const { return points; }
    void SetKeyframe(int anchor, float t, float v);
    void RemoveKeyframe(int anchor);
    void SetPoint(int i, float t, float v);
    inline float Time() const
    {
        return points.back().P.x - points.front().P.x;
    }
    inline float StartTime() const
    {
        return points.front().P.x;
    }
    inline float EndTime() const
    {
        return points.back().P.x;
    }
    const Point &operator[](int anchor) const;
    int Anchors() const { return points.size() / 3 + 1; }
    inline int Segments() const { return points.size() / 3; }

private:
    float function(float t, float p0, float out, float in, float p1) const;
    float interpolate(int anchorA, int anchorB, float t) const;

private:
    // Cubic Hermite spline
    // 
    // PO IPO IPO IPO IP  layout
    // 0   1   2   3   4  anchors
    // 01 234 567 890 12  points
    std::vector<Point> points;
};