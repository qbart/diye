#pragma once

#include "glm.hpp"

class Transform
{
public:
	Transform();
	Transform(const Transform &transform);
	Transform &operator=(const Transform &transform);
	~Transform() = default;

	void Update();
	const Mat4 &GetModelMatrix() const { return matrix; };

public:
	Vec3 position;
	Quat rotation;
	Vec3 scale;

private:
	Mat4 matrix;
};