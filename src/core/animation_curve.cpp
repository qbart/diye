#include "animation_curve.hpp"
#include <algorithm>

AnimationCurve::AnimationCurve()
{
    points = {
        Vec2(0, 0),       // anchor 1
        Vec2(0.9f, 0), // out tangent
        Vec2(1, -0.5f),      // in tangent
        Vec2(1, 1),       // anchor 2
    };
    sort();
}

void AnimationCurve::AddKey(float time, float value)
{
    // int insertAt = 0;
    // for (int i = 0; i < points.size(); i += 3)
    // {
    //     if (time < points[i].x)
    //         insertAt = i;
    // }

    // points.insert(points.begin() + insertAt - 1, Vec2(time, value + 0.5f));
    // points.insert(points.begin() + insertAt, Vec2(time, value));
    // points.insert(points.begin() + insertAt + 1, Vec2(time, value - 0.5f));
}

void AnimationCurve::SetKeyframe(int i, float t, float v)
{
    points[i] = Vec2(t, v);
    // sort();
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

    int lenAnchors = points.size() / 3;
    for (int i = 0; i < lenAnchors; ++i)
    {
        if (t <= points[i * 3 + 3].x)
        {
            float u = (t - points[i * 3].x) / (points[i * 3 + 3].x - points[i * 3].x);
            return interpolate(i * 3, i * 3 + 3, u);
        }
    }

    return 0.0f;
}

void AnimationCurve::sort()
{
    // Ensure points are sorted by time
    // std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe &a, const Keyframe &b)
    //           { return a.time < b.time; });
}
