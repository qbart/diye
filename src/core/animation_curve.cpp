#include "animation_curve.hpp"
#include <algorithm>
#include <fmt/core.h>

float AnimationCurve::TangentLimit = 25;

AnimationCurve::AnimationCurve()
{
    Point p0;
    p0.Time = 0;
    p0.Value = 0;
    p0.OutTangent = 0;
    p0.InTangent = 0;
    p0.Locked = true;

    Point p1;
    p1.Time = 1;
    p1.Value = 1;
    p1.OutTangent = 0;
    p1.InTangent = 0;
    p1.Locked = true;

    points = {p0, p1};
}

// NOTE: add better continuity without distorting the curve,
// it does work ok, but it could be better
void AnimationCurve::AddKey(float time, float value)
{
    if (time < 0)
        time = 0;
    if (time > 1)
        time = 1;

    for (int i = 0; i < points.size() - 1; ++i)
    {
        if (time > points[i].Time && time < points[i + 1].Time)
        {
            Point p;
            p.Time = time;
            p.Value = value;
            p.OutTangent = 0.f;
            p.InTangent = 0.f;
            p.Locked = true;

            points.emplace(points.begin() + i + 1, p);
            return;
        }
    }
}

void AnimationCurve::RemoveKeyframe(int i)
{
    // never delete first or last anchor
    if (i == 0 || i == points.size() - 1)
        return;

    points.erase(points.begin() + i);
}

void AnimationCurve::SetPoint(int i, float t, float v)
{
    float lowerLimit = 0;
    float upperLimit = 1;
    float gapLimit = 0.001;
    if (i == 0 || i == points.size() - 1)
        gapLimit = 0;

    if (i != 0)
        lowerLimit = points[i - 1].Time;
    if (i != points.size() - 1)
        upperLimit = points[i + 1].Time;

    float x = Mathf::Clamp(t, lowerLimit + gapLimit, upperLimit - gapLimit);
    float y = v;
    points[i].Time = x;
    points[i].Value = y;
}

void AnimationCurve::SetOutTangent(int i, float t, float v)
{
    if (i >= points.size() - 1)
        return;

    float dt = t - points[i].Time;
    float df = v - points[i].Value;

    if (dt == 0)
        return;

    points[i].OutTangent = clampTangent(df / dt);
    if (points[i].Locked)
        points[i].InTangent = points[i].OutTangent;
    // fmt::println("OutTangent: {}", points[i].OutTangent);
}

void AnimationCurve::SetInTangent(int i, float t, float v)
{
    if (i <= 0)
        return;

    float dt = t - points[i].Time;
    float df = v - points[i].Value;

    if (dt == 0)
        return;

    points[i].InTangent = clampTangent(df / dt);
    if (points[i].Locked)
        points[i].OutTangent = points[i].InTangent;
    // fmt::println("InTangent: ({} , {}) -> {}", t, v, points[i].InTangent);
}

void AnimationCurve::ToggleTangentSplitJoin(int i, Tangent dominantTangnent)
{
    if (i == 0 || i == points.size() - 1)
        return;

    points[i].Locked = !points[i].Locked;
    if (dominantTangnent == Tangent::Out)
        points[i].InTangent = points[i].OutTangent;
    else
        points[i].OutTangent = points[i].InTangent;
}

Vec2 AnimationCurve::Anchor(int i) const
{
    return Vec2(points[i].Time, points[i].Value);
}

Vec2 AnimationCurve::OutTangent(int i) const
{
    auto tangent = Mathf::NormalizedVec2(1.0f, points[i].OutTangent);
    auto outTangentEnd = Anchor(i) + tangent * 0.12f;
    return outTangentEnd;
}

Vec2 AnimationCurve::InTangent(int i) const
{
    auto tangent = Mathf::NormalizedVec2(1.0f, points[i].InTangent);
    auto inTangentEnd = Anchor(i) - tangent * 0.12f;
    return inTangentEnd;
}

float AnimationCurve::function(float t, float p0, float out, float in, float p1) const
{
    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h01 = -2.0f * t3 + 3.0f * t2;
    float h11 = t3 - t2;

    return p0 * h00 + out * h10 + p1 * h01 + in * h11;
}

float AnimationCurve::clampTangent(float tan) const
{
    float linearThreshold = TangentLimit;
    return Mathf::Clamp(tan, -TangentLimit - linearThreshold, TangentLimit + linearThreshold);
    // return tan;
}

float AnimationCurve::Evaluate(float t) const
{
    if (t <= StartTime())
        return points.front().Value;
    if (t >= EndTime())
        return points.back().Value;

    int segments = Segments();
    for (int i = 0; i < segments; ++i)
    {
        if (t <= points[i + 1].Time)
        {
            float tMax = points[i + 1].Time - points[i].Time;
            float tCurrent = (t - points[i].Time);
            float u = tCurrent / tMax;

            // clamp to avoid extreme values, this assumes
            // that allowed tangent values are between (-TangetLimit, TangentLimit),
            // anything outside will cause to switch to fixed value
            if (points[i].OutTangent >= TangentLimit)
                return points[i + 1].Value;
            if (points[i].OutTangent <= -TangentLimit)
                return points[i].Value;
            if (points[i + 1].InTangent >= TangentLimit)
                return points[i + 1].Value;
            if (points[i + 1].InTangent <= -TangentLimit)
                return points[i].Value;

            return function(u,
                            points[i].Value,
                            points[i].OutTangent * tMax,
                            points[i + 1].InTangent * tMax,
                            points[i + 1].Value);
        }
    }

    return 0.0f;
}
