#include "animation_curve.hpp"
#include <algorithm>
#include <fmt/core.h>

AnimationCurve::AnimationCurve()
{
    points = {
        {Vec2(0, 0), true},       // anchor 1
        {Vec2(0.2f, 0.2f), true}, // out tangent
        {Vec2(1.2f, 1.2f), true}, // in tangent
        {Vec2(1, 1), true},       // anchor 2
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
        if (time > points[i].P.x && time < points[i + 3].P.x)
        {
            // fmt::print("Adding key after anchor {}  [{}]\n", anchor, i);
            // we are between anchor and anchor+1,
            // and we need to skip the out tangent of i
            int gap = 1;

            auto inTangent = Point{Vec2(time - 0.1f, value + 0.1f), true};
            auto anchorPoint = Point{Vec2(time, value), true};
            auto outTangent = Point{Vec2(time + 0.1f, value + 0.1f), true};
            auto smoothTangent = Point{(inTangent.P + outTangent.P) * 0.5f, true};

            points.emplace(points.begin() + i + 1 + 1, smoothTangent);
            points.emplace(points.begin() + i + 2 + 1, anchorPoint);
            points.emplace(points.begin() + i + 3 + 1, smoothTangent);
            return;
        }
    }
}

void AnimationCurve::SetKeyframe(int anchor, float t, float v)
{
    points[anchor * 3].P = Vec2(t, v);
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
    if (IsAnchor(i))
    {
        float sx = points[i].P.x;
        float sy = points[i].P.y;
        float lowerLimit = 0;
        float upperLimit = 1;
        if (i != 0)
            lowerLimit = points[i - 3].P.x;
        if (i != points.size() - 1)
            upperLimit = points[i + 3].P.x;

        float x = Mathf::Clamp(t, lowerLimit, upperLimit);
        float y = v;
        points[i].P = Vec2(x, v);

        // move tangents
        Vec2 d = Vec2(x - sx, y - sy);
        if (i > 0)
        {
            points[i - 1].P = points[i - 1].P + d;
        }
        if (i < points.size() - 1)
        {
            points[i + 1].P = points[i + 1].P + d;
        }
    }
    else
    {
        points[i].P = Vec2(t, v);
        if (points[i].Locked)
        {
            if (IsOutTangent(i) && i - 2 > 0)
            {
                points[i - 2].P = points[i].P;
            }
            if (IsInTangent(i) && i + 2 < points.size() - 1)
            {
                points[i + 2].P = points[i].P;
            }
        }
    }
}

void AnimationCurve::ToggleTangentSplitJoin(int i)
{
    assert(IsTangent(i));
    bool wasLocked = points[i].Locked;
    bool nowLocked = !wasLocked;
    points[i].Locked = nowLocked;

    if (IsOutTangent(i))
    {
        points[i - 1].Locked = nowLocked;
        if (i - 2 > 0) // if there is a corresponding in-tangent
        {
            points[i - 2].Locked = nowLocked;
            if (nowLocked)
                points[i - 2].P = points[i].P;
        }
    }
    if (IsInTangent(i))
    {
        points[i + 1].Locked = nowLocked;
        if (i + 2 < points.size() - 1) // if there is a corresponding out-tangent
        {
            points[i + 2].Locked = nowLocked;
            if (nowLocked)
                points[i + 2].P = points[i].P;
        }
    }
}

const AnimationCurve::Point &AnimationCurve::operator[](int anchor) const
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
    const auto &outTangent0 = points[anchorA + 1].P - p0.P;             // p0out - p0
    const auto &inTangent1 = points[anchorB - 1].P - points[anchorB].P; // p1in - p1
    const auto &p1 = points[anchorB];

    // float x = function(t, p0.P.x, outTangent0.x, inTangent1.x, p1.P.x);
    float y = function(t, p0.P.y, outTangent0.y, inTangent1.y, p1.P.y);
    return y;
}

float AnimationCurve::Evaluate(float t) const
{
    if (t <= points.front().P.x)
        return points.front().P.y;
    if (t >= points.back().P.x)
        return points.back().P.y;

    int segments = points.size() / 3;
    for (int anchor = 0; anchor < segments; ++anchor)
    {
        int i = anchor * 3;
        if (t <= points[i + 3].P.x)
        {
            float u = (t - points[i].P.x) / (points[i + 3].P.x - points[i].P.x);
            return interpolate(i, i + 3, u);
        }
    }

    return 0.0f;
}