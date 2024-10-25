#pragma once

#include "types.hpp"

class Transform
{
public:
	enum class Space
	{
		Local,
		World,
		WorldOnly
	};

public:
	Transform();
	Transform(const Transform &transform);
	Transform &operator=(const Transform &transform);
	~Transform() = default;

	void Update();
	const Mat4 &GetModelMatrix() const { return matrix; };
	Mat4 ModelMatrix(Space mode = Space::World) const;

public:
	Vec3 localPosition;
	Vec3 position;
	Quat rotation;
	Vec3 scale;

private:
	Mat4 matrix;
};