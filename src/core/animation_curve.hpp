#pragma once

#include "math.hpp"
#include "timer.hpp"
#include "types.hpp"

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
        bool Locked;
    };
    enum class Tangent
    {
        In,
        Out
    };
    enum class Preset
    {
        Zero,
        One,
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
    };

public:
    AnimationCurve();
    void ApplyPreset(Preset preset);
    void EnableLinearInterpolation(bool value);
    void AddKey(float time, float value);
    float Evaluate(float t) const;
    float Evaluate(const Timer &timer) const;
    const std::vector<Point> &Points() const { return points; }
    void RemoveKeyframe(int i);
    void SetPoint(int i, float t, float v);
    void SetOutTangent(int i, float t, float v);
    void SetInTangent(int i, float t, float v);
    void SetOutTangentValue(int i, float v);
    void SetInTangentValue(int i, float v);
    void ToggleTangentSplitJoin(int i, Tangent dominantTangnent = Tangent::Out);
    Vec2 Anchor(int i) const;
    Vec2 OutTangent(int i) const;
    Vec2 InTangent(int i) const;

    inline const Point &operator[](int i) const { return points[i]; }
    inline int PointCount() const { return points.size(); }
    inline bool HasOutTangent(int i) const { return i <= points.size() - 2; }
    inline bool HasInTangent(int i) const { return i >= 1; }
    inline float Time() const { return EndTime() - StartTime(); }
    inline float StartTime() const { return points.front().Time; }
    inline float EndTime() const { return points.back().Time; }
    inline int Segments() const { return points.size() - 1; }

private:
    float function(float t, float p0, float out, float in, float p1) const;
    float clampTangent(float tan) const;

private:
    std::vector<Point> points;
    bool linear;
};