#include "animation_curve.hpp"
#include <algorithm>
#include <fmt/core.h>

AnimationCurve::AnimationCurve()
{
    points = {
        Vec2(0, 0),     // anchor 1
        Vec2(0.9f, 0),  // out tangent
        Vec2(1, -0.5f), // in tangent
        Vec2(1, 1),     // anchor 2
    };
}

void AnimationCurve::AddKey(float time, float value)
{
    if (time < 0)
        time = 0;
    if (time > 1)
        time = 1;

    for (int anchor = 0; anchor < Segments(); ++anchor)
    {
        int i = anchor * 3;
        if (time > points[i].x && time < points[i + 3].x)
        {
            // we are between anchor i and i+1,
            // and we need to skip the out tangent of i
            points.insert(points.begin() + 1 + 1, Vec2(time, value - 0.1f));
            points.insert(points.begin() + 2 + 1, Vec2(time, value));
            points.insert(points.begin() + 3 + 1, Vec2(time, value + 0.1f));
            return;
        }
    }
}

void AnimationCurve::SetKeyframe(int anchor, float t, float v)
{
    points[anchor * 3] = Vec2(t, v);
}

void AnimationCurve::SetPoint(int i, float t, float v)
{
    bool isAnchor = i % 3 == 0;
    if (isAnchor)
    {
        points[i] = Vec2(t, v);
    }
    else
    {
        points[i] = Vec2(t, v);
    }
}

const Vec2 &AnimationCurve::operator[](int anchor) const
{
    return points[anchor * 3];
}

float AnimationCurve::interpolate(int anchorA, int anchorB, float t) const
{
    assert(anchorA + 3 == anchorB);

    const auto &p0 = points[anchorA];
    const auto &outTangent0 = points[anchorA + 1] - p0;             // p0out - p0
    const auto &inTangent1 = points[anchorB - 1] - points[anchorB]; // p1in - p1
    const auto &p1 = points[anchorB];

    float t2 = t * t;
    float t3 = t2 * t;
    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h01 = -2.0f * t3 + 3.0f * t2;
    float h11 = t3 - t2;

    Vec2 result = p0 * h00 + outTangent0 * h10 + p1 * h01 + inTangent1 * h11;
    return result.y;
}

float AnimationCurve::Evaluate(float t) const
{
    if (t <= points.front().x)
        return points.front().y;
    if (t >= points.back().x)
        return points.back().y;

    int segments = points.size() / 3;
    for (int anchor = 0; anchor < segments; ++anchor)
    {
        int i = anchor * 3;
        if (t <= points[i + 3].x)
        {
            float u = (t - points[i].x) / (points[i + 3].x - points[i].x);
            return interpolate(i, i + 3, u);
        }
    }

    return 0.0f;
}