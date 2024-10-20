#pragma once

#include "glm.hpp"
#include "transform.hpp"

class Camera {
public:
	enum Mode { Perspective, Orthogonal };

public:
	Camera();
	Camera(const Camera &) = delete;
	Camera & operator=(const Camera &) = delete;
	Camera(Camera &&) = delete;
	Camera & operator=(Camera &&) = delete;
	~Camera();

	void SetPerspective(float fov, uint32 width, uint32 height, float zNear = 0.01f, float zFar = 100.f);
	void UpdatePerspective(const Dimension& size);
	void SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
	void SetOrtho(uint32 width, uint32 height, float pixelsPerUnit = 100.f, float zNear = 0.01f, float zFar = 100.f);
	void UpdateOrtho(const Dimension& size);
	void SetPosition(const Vec3& position);
	void SetOrientation(const Quat& quat);
	void LookAt(const Vec3& targetPosition);
	void MoveAndLookAt(const Vec3& position, const Vec3& targetPosition);
	void LookAround(float pitch, float yaw);
	void MoveForward(float speed);
	void MoveBackward(float speed);
	void MoveLeft(float speed);
	void MoveRight(float speed);
	inline const Mat4& GetProjection() const { return projection; }
	inline const Vec3& GetPosition() const { return transform.position; }
	inline const Quat& GetOrientation() const { return transform.rotation; }
	inline Mat4 GetViewMatrix() const { return view; }
	inline Mat4 MVP(const Mat4& model) const
	{
		// MVP = P * V * M
		// P*V is cached to avoid unnecessary calculations
		return viewProjection * model;
	}

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