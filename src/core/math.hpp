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
                glm::radians(angles.z)
            )
        );
    }
};