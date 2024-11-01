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

    for (int anchor = 0; anchor < Anchors() - 1; ++anchor)
    {
        int i = anchor * 3;
        if (time > points[i].x && time < points[i + 3].x)
        {
            // fmt::print("Adding key after anchor {}  [{}]\n", anchor, i);
            // we are between anchor and anchor+1,
            // and we need to skip the out tangent of i
            int gap = 1;

            auto inTangent = Vec2(time + 0.1f, value - 0.1f);
            auto anchorPoint = Vec2(time, value);
            auto outTangent = Vec2(time + 0.1f, value + 0.1f);
            auto smoothTangent = (inTangent + outTangent) * 0.5f;

            points.emplace(points.begin() + i + 1 + 1, smoothTangent);
            points.emplace(points.begin() + i + 2 + 1, anchorPoint);
            points.emplace(points.begin() + i + 3 + 1, smoothTangent);
            return;
        }
    }
}

void AnimationCurve::SetKeyframe(int anchor, float t, float v)
{
    points[anchor * 3] = Vec2(t, v);
}

void AnimationCurve::RemoveKeyframe(int anchor)
{
    // never delete first or last anchor
    if (anchor == 0 || anchor == Anchors() - 1)
        return;

    points.erase(
        points.begin() + anchor * 3 - 1,
        points.begin() + anchor * 3 + 2);
}

void AnimationCurve::SetPoint(int i, float t, float v)
{
    bool isAnchor = i % 3 == 0;
    if (isAnchor)
    {
        float sx = points[i].x;
        float sy = points[i].y;
        float lowerLimit = 0;
        float upperLimit = 1;
        if (i != 0)
            lowerLimit = points[i - 3].x;
        if (i != points.size() - 1)
            upperLimit = points[i + 3].x;

        float x = Mathf::Clamp(t, lowerLimit, upperLimit);
        float y = v;
        points[i] = Vec2(x, v);

        // move tangents
        Vec2 d = Vec2(x - sx, y - sy);
        if (i > 0)
        {
            points[i - 1] = points[i - 1] + d;
        }
        if (i < points.size() - 1)
        {
            points[i + 1] = points[i + 1] + d;
        }
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

float AnimationCurve::interpolate(int anchorA, int anchorB, float t) const
{
    assert(anchorA + 3 == anchorB);

    const auto &p0 = points[anchorA];
    const auto &outTangent0 = points[anchorA + 1] - p0;             // p0out - p0
    const auto &inTangent1 = points[anchorB - 1] - points[anchorB]; // p1in - p1
    const auto &p1 = points[anchorB];

    float x = function(t, p0.x, outTangent0.x, inTangent1.x, p1.x);
    float y = function(t, p0.y, outTangent0.y, inTangent1.y, p1.y);
    return y;
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