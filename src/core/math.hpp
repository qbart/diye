#pragma once

#include "types.hpp"

class Mathf
{
public:
    Mat4 static Fov(float degAngle, int w, int h, float zNear = 0.001f, float zFar = 1000.0f)
    {
        return glm::perspectiveFov(glm::radians(degAngle), (float)w, (float)h, zNear, zFar);
    }

    Mat4 static Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        return glm::ortho(left, right, bottom, top, zNear, zFar);
    }

    Vec3 static Cross(const Vec3 &a, const Vec3 &b)
    {
        return glm::cross(a, b);
    }

    float static Distance(const Vec2 &a, const Vec2 &b)
    {
        return glm::distance(a, b);
    }

    Vec2 static NormalizedVec2(float x, float y)
    {
        return glm::normalize(Vec2(x, y));
    }

    Vec3 static NormalizedVec3(float x, float y, float z)
    {
        return glm::normalize(Vec3(x, y, z));
    }

    Vec2 static Normalize(const Vec2 &v)
    {
        return glm::normalize(v);
    }

    Vec3 static Normalize(const Vec3 &v)
    {
        return glm::normalize(v);
    }

    Vec3 static Lerp(const Vec3 &a, const Vec3 &b, float t)
    {
        return glm::mix(a, b, t);
    }

    float static Lerp(float a, float b, float t)
    {
        return glm::mix(a, b, t);
    }

    float static Clamp(float val, float min, float max)
    {
        return glm::clamp(val, min, max);
    }

    float static Clamp01(float val)
    {
        return glm::clamp(val, 0.0f, 1.0f);
    }

    float static CloseTo(float a, float b, float epsilon = 0.0001f)
    {
        return glm::epsilonEqual(a, b, epsilon);
    }

    float static Abs(float val)
    {
        return glm::abs(val);
    }

    Quat static QuatAngles(const Vec3 &angles)
    {
        return glm::quat(
            glm::vec3(
                glm::radians(angles.x),
                glm::radians(angles.y),
                glm::radians(angles.z)));
    }

    Quat static Rotate(const Quat &q, float deg, const Vec3 &axes = UP)
    {
        return glm::rotate(q, glm::radians(deg), axes);
    }
};