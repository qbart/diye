#include "math.hpp"

namespace Mathf
{
    Mat4 Fov(float degAngle, int w, int h, float zNear, float zFar)
    {
        return glm::perspectiveFov(glm::radians(degAngle), (float)w, (float)h, zNear, zFar);
    }

    Mat4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        return glm::ortho(left, right, bottom, top, zNear, zFar);
    }

    Vec3 Cross(const Vec3 &a, const Vec3 &b)
    {
        return glm::cross(a, b);
    }

    float Dot(const Vec3 &a, const Vec3 &b)
    {
        return glm::dot(a, b);
    }

    float Distance(const Vec3 &a, const Vec3 &b)
    {
        return glm::distance(a, b);
    }

    float Distance(const Vec2 &a, const Vec2 &b)
    {
        return glm::distance(a, b);
    }

    Vec2 NormalizedVec2(float x, float y)
    {
        return glm::normalize(Vec2(x, y));
    }

    Vec3 NormalizedVec3(float x, float y, float z)
    {
        return glm::normalize(Vec3(x, y, z));
    }

    Vec2 Normalize(const Vec2 &v)
    {
        return glm::normalize(v);
    }

    Vec3 Normalize(const Vec3 &v)
    {
        return glm::normalize(v);
    }

    Vec3 Lerp(const Vec3 &a, const Vec3 &b, float t)
    {
        return glm::mix(a, b, t);
    }

    float Lerp(float a, float b, float t)
    {
        return glm::mix(a, b, t);
    }

    float Clamp(float val, float min, float max)
    {
        return glm::clamp(val, min, max);
    }

    float Clamp01(float val)
    {
        return glm::clamp(val, 0.0f, 1.0f);
    }

    float CloseTo(float a, float b, float epsilon)
    {
        return glm::epsilonEqual(a, b, epsilon);
    }

    float Abs(float val)
    {
        return glm::abs(val);
    }

    Quat QuatAngles(const Vec3 &angles)
    {
        return glm::quat(
            glm::vec3(
                glm::radians(angles.x),
                glm::radians(angles.y),
                glm::radians(angles.z)));
    }

    Quat Rotate(const Quat &q, float deg, const Vec3 &axes)
    {
        return glm::rotate(q, glm::radians(deg), axes);
    }

    bool RayTriangleIntersection(const Ray &ray, const Vec3 &a, const Vec3 &b, const Vec3 &c, float &distance)
    {
        const float EPSILON = 0.0000001f;
        Vec3 edge1 = b - a;
        Vec3 edge2 = c - a;
        Vec3 h = glm::cross(ray.Direction, edge2);
        float det = glm::dot(edge1, h);

        if (det > -EPSILON && det < EPSILON)
            return false; // This ray is parallel to this triangle.

        float invDet = 1.0f / det;
        Vec3 s = ray.Origin - a;
        float u = glm::dot(s, h) * invDet;

        if (u < 0.0f || u > 1.0f)
            return false;

        Vec3 q = glm::cross(s, edge1);
        float v = glm::dot(ray.Direction, q) * invDet;

        if (v < 0.0f || u + v > 1.0f)
            return false;

        float t = glm::dot(edge2, q) * invDet;

        if (t > EPSILON) // ray intersection
        {
            distance = t;
            return true;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return false;
    }
};