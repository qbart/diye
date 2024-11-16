#pragma once

#include "types.hpp"

namespace Mathf
{
    Mat4 Fov(float degAngle, int w, int h, float zNear = 0.001f, float zFar = 1000.0f);
    Mat4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
    Vec3 Cross(const Vec3 &a, const Vec3 &b);
    float Dot(const Vec3 &a, const Vec3 &b);
    float Distance(const Vec3 &a, const Vec3 &b);
    float Distance(const Vec2 &a, const Vec2 &b);
    Vec2 NormalizedVec2(float x, float y);
    Vec3 NormalizedVec3(float x, float y, float z);
    Vec2 Normalize(const Vec2 &v);
    Vec3 Normalize(const Vec3 &v);
    Vec3 Lerp(const Vec3 &a, const Vec3 &b, float t);
    float Lerp(float a, float b, float t);
    float Clamp(float val, float min, float max);
    float Clamp01(float val);
    float CloseTo(float a, float b, float epsilon = 0.0001f);
    float Abs(float val);
    Quat QuatAngles(const Vec3 &angles);
    Quat Rotate(const Quat &q, float deg, const Vec3 &axes = UP);
    bool RayTriangleIntersection(const Ray &ray, const Vec3 &a, const Vec3 &b, const Vec3 &c, float &distance);
};