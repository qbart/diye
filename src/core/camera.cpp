#include "camera.hpp"

Camera::Camera() : zNear(0.1f),
                   zFar(1000.0f),
                   mode(Perspective),
                   pixelsPerUnit(100),
                   fov(60) {}

Camera::~Camera() {}

Vec3 Camera::ViewDir() const
{
    return Mathf::Normalize(glm::conjugate(transform.rotation) * FORWARD);
}

void Camera::SetPerspective(float fov, uint32 width, uint32 height, float zNear, float zFar)
{
    this->fov = fov;
    this->zFar = zFar;
    this->zNear = zNear;
    mode = Perspective;

    projection = glm::perspectiveFov(glm::radians(fov), (float)width, (float)height, zNear, zFar);
    projection[1][1] *= -1; // Vulkan flip-Y

    UpdateMatrix();
}

void Camera::UpdatePerspective(const Dimension &size)
{
    SetPerspective(fov, size.w, size.h, zNear, zFar);
}

void Camera::SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    projection = glm::ortho(left, right, bottom, top, zNear, zFar);
    UpdateMatrix();
}

void Camera::SetOrtho(uint32 width, uint32 height, float pixelsPerUnit, float zNear, float zFar)
{
    this->zFar = zFar;
    this->zNear = zNear;
    this->pixelsPerUnit = pixelsPerUnit;
    mode = Orthogonal;

    SetOrtho(-(width / pixelsPerUnit), +(width / pixelsPerUnit), -(height / pixelsPerUnit), +(height / pixelsPerUnit), zNear, zFar);
}

void Camera::UpdateOrtho(const Dimension &size)
{
    SetOrtho(size.w, size.h, pixelsPerUnit, zNear, zFar);
}

void Camera::SetPosition(const Vec3 &position)
{
    transform.position = position;
    UpdateMatrix();
}

void Camera::SetOrientation(const Quat &quat)
{
    transform.rotation = quat;
    UpdateMatrix();
}

void Camera::RotateAround(const Vec3 &axis, float angle)
{
    transform.rotation = glm::normalize(glm::angleAxis(angle, axis) * transform.rotation);
    UpdateMatrix();
}

void Camera::LookAt(const Vec3 &targetPosition)
{
    Mat4 view(glm::lookAt(transform.position, targetPosition, UP));
    transform.rotation = glm::normalize(glm::quat_cast(view));
    UpdateMatrix();
}

void Camera::OrbitAround(const Vec3 &axis, const Vec3 &around, float angle)
{
    Quat quat = transform.rotation;
    Vec3 direction = glm::conjugate(quat) * FORWARD;
    direction = glm::cross(direction, axis);
    transform.position -= direction * angle;

    LookAt(around); // this will update matrix
}

void Camera::MoveAndLookAt(const Vec3 &position, const Vec3 &targetPosition)
{
    Mat4 view(glm::lookAt(position, targetPosition, UP));
    transform.position = position;
    transform.rotation = glm::normalize(glm::quat_cast(view));
    UpdateMatrix();
}

void Camera::LookAround(float pitch, float yaw)
{
    Quat quat = Mathf::QuatAngles(Vec3(pitch, 0, 0)) * transform.rotation * Mathf::QuatAngles(Vec3(0, yaw, 0));
    transform.rotation = glm::normalize(quat);

    UpdateMatrix();
}

void Camera::MoveForward(float speed)
{
    Quat quat = transform.rotation;
    Vec3 direction = glm::conjugate(quat) * FORWARD;
    transform.position += direction * speed;
    UpdateMatrix();
}

void Camera::MoveBackward(float speed)
{
    Quat quat = transform.rotation;
    Vec3 direction = glm::conjugate(quat) * FORWARD;
    transform.position -= direction * speed;
    UpdateMatrix();
}

void Camera::MoveLeft(float speed)
{
    Quat quat = transform.rotation;
    Vec3 direction = glm::conjugate(quat) * FORWARD;
    direction = glm::cross(direction, UP);
    transform.position -= direction * speed;
    UpdateMatrix();
}

void Camera::MoveRight(float speed)
{
    Quat quat = transform.rotation;
    Vec3 direction = glm::conjugate(quat) * FORWARD;
    direction = glm::cross(direction, UP);
    transform.position += direction * speed;
    UpdateMatrix();
}

Ray Camera::ScreenToRay(const Vec2 &screenPos, const Vec2 &screenSize) const
{
    Vec4 clipCoords = Vec4(
        (2.0f * screenPos.x) / screenSize.x - 1.0f,
        1.0f - (2.0f * screenPos.y) / screenSize.y,
        -1.0f, 1.0f);

    Vec4 eyeCoords = glm::inverse(projection) * clipCoords;
    eyeCoords.z = FORWARD.z;
    eyeCoords.w = 0.0f;

    Ray ray;
    ray.Origin = transform.position + transform.localPosition;
    ray.Direction = glm::normalize(Vec3(glm::inverse(view) * eyeCoords));
    return ray;
}

void Camera::UpdateMatrix()
{
    Mat4 translationMat(1);
    translationMat = glm::translate(translationMat, -transform.position);

    Mat4 rotationMat = glm::mat4_cast(transform.rotation);
    // Mat4 rotationMat = glm::mat4_cast(glm::inverse(transform.rotation));

    view = rotationMat * translationMat;
    viewProjection = projection * view;
}
