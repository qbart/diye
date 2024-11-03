#include "animation_curve.hpp"
#include <algorithm>
#include <fmt/core.h>

AnimationCurve::AnimationCurve()
{
    Point p0;
    p0.Time = 0;
    p0.Value = 0;
    p0.OutTangent = 1;
    p0.InTangent = 0;
    p0.Weight = 0;
    p0.Locked = true;

    Point p1;
    p1.Time = 1;
    p1.Value = 1;
    p1.OutTangent = 0;
    p1.InTangent = 1;
    p1.Weight = 0;
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
            p.OutTangent = 0.5f;
            p.InTangent = 0.5f;
            p.Weight = 0;
            p.Locked = false;

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
    // if (IsAnchor(i))
    // {
    //     float sx = points[i].P.x;
    //     float sy = points[i].P.y;
    //     float lowerLimit = 0;
    //     float upperLimit = 1;
    //     if (i != 0)
    //         lowerLimit = points[i - 3].P.x;
    //     if (i != points.size() - 1)
    //         upperLimit = points[i + 3].P.x;

    //     float x = Mathf::Clamp(t, lowerLimit, upperLimit);
    //     float y = v;
    //     points[i].P = Vec2(x, v);

    //     // move tangents
    //     Vec2 d = Vec2(x - sx, y - sy);
    //     if (i > 0)
    //     {
    //         points[i - 1].P = points[i - 1].P + d;
    //     }
    //     if (i < points.size() - 1)
    //     {
    //         points[i + 1].P = points[i + 1].P + d;
    //     }
    // }
    // else
    // {
    //     points[i].P = Vec2(t, v);
    //     if (points[i].Locked)
    //     {
    //         if (IsOutTangent(i) && i - 2 > 0)
    //         {
    //             points[i - 2].P = points[i].P;
    //         }
    //         if (IsInTangent(i) && i + 2 < points.size() - 1)
    //         {
    //             points[i + 2].P = points[i].P;
    //         }
    //     }
    // }
}

void AnimationCurve::ToggleTangentSplitJoin(int i)
{
    // assert(IsTangent(i));
    // bool wasLocked = points[i].Locked;
    // bool nowLocked = !wasLocked;
    // points[i].Locked = nowLocked;

    // if (IsOutTangent(i))
    // {
    //     points[i - 1].Locked = nowLocked;
    //     if (i - 2 > 0) // if there is a corresponding in-tangent
    //     {
    //         points[i - 2].Locked = nowLocked;
    //         if (nowLocked)
    //             points[i - 2].P = points[i].P;
    //     }
    // }
    // if (IsInTangent(i))
    // {
    //     points[i + 1].Locked = nowLocked;
    //     if (i + 2 < points.size() - 1) // if there is a corresponding out-tangent
    //     {
    //         points[i + 2].Locked = nowLocked;
    //         if (nowLocked)
    //             points[i + 2].P = points[i].P;
    //     }
    // }
}

Vec2 AnimationCurve::Anchor(int i) const
{
    return Vec2(points[i].Time, points[i].Value);
}

Vec2 AnimationCurve::OutTangent(int i) const
{
    auto p0 = Anchor(i);
    auto outTangentEnd = p0 + Vec2(1.0f, points[i].OutTangent) * 0.1f; // dx = 1, dy = OutTangent
    return outTangentEnd;
}

Vec2 AnimationCurve::InTangent(int i) const
{
    auto p0 = Anchor(i);
    auto inTangentEnd = p0 - Vec2(1.0f, points[i].InTangent) * 0.1f; // dx = 1, dy = InTangent
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

float AnimationCurve::interpolate(int a, int b, float t) const
{
    return function(t, points[a].Value, points[a].OutTangent, points[b].InTangent, points[b].Value);
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
            float u = (t - points[i].Time) / (points[i + 1].Time - points[i].Time);
            return interpolate(i, i + 1, u);
        }
    }

    return 0.0f;
}
