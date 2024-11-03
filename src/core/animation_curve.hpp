#pragma once

#include "types.hpp"
#include "math.hpp"

class AnimationCurve
{
public:
    static float TangentLimit;

    struct Point
    {
        float Time;
        float Value;
        float OutTangent;
        float InTangent;
        float Weight;
        bool Locked;
    };

public:
    AnimationCurve();
    void AddKey(float time, float value);
    float Evaluate(float t) const;
    const std::vector<Point> &Points() const { return points; }
    void RemoveKeyframe(int i);
    void SetPoint(int i, float t, float v);
    void SetOutTangent(int i, float t, float v);
    void SetInTangent(int i, float t, float v);
    void ToggleTangentSplitJoin(int i);
    Vec2 Anchor(int i) const;
    Vec2 OutTangent(int i) const;
    Vec2 InTangent(int i) const;

    inline const Point &operator[](int i) const { return points[i]; }
    inline int PointCount() const { return points.size(); }
    inline bool HasOutTangent(int i) const { return i <= points.size() - 2; }
    inline bool HasInTangent(int i) const { return i >= 1; }
    inline float Time() const
    {
        return EndTime() - StartTime();
    }
    inline float StartTime() const
    {
        return points.front().Time;
    }
    inline float EndTime() const
    {
        return points.back().Time;
    }
    inline int Segments() const { return points.size() - 1; }

private:
    float function(float t, float p0, float out, float in, float p1) const;
    float clampTangent(float tan) const;

private:
    std::vector<Point> points;
};