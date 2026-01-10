#pragma once

#include "math.hpp"
#include "transform.hpp"
#include "types.hpp"

class Camera
{
public:
    enum Mode
    {
        Perspective,
        Orthogonal
    };

public:
    Camera();
    Camera(const Camera &)            = delete;
    Camera &operator=(const Camera &) = delete;
    Camera(Camera &&)                 = delete;
    Camera &operator=(Camera &&)      = delete;
    ~Camera();

    Vec3 ViewDir() const;
    void SetPerspective(float fov, uint32 width, uint32 height, float zNear = 0.01f, float zFar = 100.f);
    void UpdatePerspective(const Dimension &size);
    void SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
    void SetOrtho(uint32 width, uint32 height, float pixelsPerUnit = 100.f, float zNear = 0.01f, float zFar = 100.f);
    void UpdateOrtho(const Dimension &size);
    void SetPosition(const Vec3 &position);
    void SetOrientation(const Quat &quat);
    void RotateAround(const Vec3 &axis, float angle);
    void LookAt(const Vec3 &targetPosition);
    void OrbitAround(const Vec3 &axis, const Vec3 &around, float angle);
    void MoveAndLookAt(const Vec3 &position, const Vec3 &targetPosition);
    void LookAround(float pitch, float yaw);
    void MoveForward(float speed);
    void MoveBackward(float speed);
    void MoveLeft(float speed);
    void MoveRight(float speed);
    Ray ScreenToRay(const Vec2 &screenPos, const Vec2 &screenSize) const;
    const Mat4 &Projection() const { return projection; }
    const Vec3 &Position() const { return transform.position; }
    const Quat &Orientation() const { return transform.rotation; }
    Mat4 View() const { return view; }
    Mat4 MVP(const Mat4 &model) const
    {
        // MVP = P * V * M
        // P*V is cached to avoid unnecessary calculations
        return viewProjection * model;
    }
    Mat4 InverseViewProjection() const { return glm::inverse(viewProjection); }
    const Mat4 &ViewProjection() const { return viewProjection; }
    float ZNear() const { return zNear; }
    float ZFar() const { return zFar; }

private:
    void UpdateMatrix();

private:
    float zNear, zFar;
    float fov;
    float pixelsPerUnit;
    Mode mode;
    Transform transform;
    Mat4 view;
    Mat4 viewProjection;
    Mat4 projection;
};