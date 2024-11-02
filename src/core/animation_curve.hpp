#pragma once

#include "types.hpp"
#include "math.hpp"

class AnimationCurve
{
public:
    struct Point
    {
        Vec2 P;
        float Weight;
        bool Locked;
    };

public:
    AnimationCurve();
    void AddKey(float time, float value);
    float Evaluate(float t) const;
    Vec2 EvaluateVec2(float t) const;
    const std::vector<Point> &Points() const { return points; }
    void SetKeyframe(int anchor, float t, float v);
    void RemoveKeyframe(int anchor);
    void SetPoint(int i, float t, float v);
    void ToggleTangentSplitJoin(int i);
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
    inline int Anchors() const { return points.size() / 3 + 1; }
    inline int Segments() const { return points.size() / 3; }
    inline bool IsTangent(int i) const { return i % 3 != 0; }
    inline bool IsAnchor(int i) const { return i % 3 == 0; }
    inline bool IsOutTangent(int i) const { return i % 3 == 1; }
    inline bool IsInTangent(int i) const { return i % 3 == 2; }
    inline Vec2 WeightedTangent(int i) const
    {
        return points[i].P * points[i].Weight;
    }

private:
    float function(float t, float p0, float out, float in, float p1) const;
    Vec2 function(float t, const Vec2 &p0, const Vec2 &out, const Vec2 &in, const Vec2 &p1) const;
    float interpolate(int anchorA, int anchorB, float t) const;
    Vec2 interpolateVec2(int anchorA, int anchorB, float t) const;

private:
    // Cubic Hermite spline
    //
    // PO IPO IPO IPO IP  layout
    // 0   1   2   3   4  anchors
    // 01 234 567 890 12  points
    // 01 201 201 201 12  modulo
    std::vector<Point> points;
};