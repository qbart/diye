#include "transform.hpp"
#include <stdexcept>

Transform::Transform() : localPosition(Vec3(0, 0, 0)),
						 position(Vec3(0, 0, 0)),
						 rotation(Quat(1, 0, 0, 0)),
						 scale(Vec3(1, 1, 1))
{
	Update();
}

Transform::Transform(const Transform &transform)
{
	localPosition = transform.localPosition;
	position = transform.position;
	rotation = transform.rotation;
	scale = transform.scale;

	Update();
}

Transform &Transform::operator=(const Transform &transform)
{
	localPosition = transform.localPosition;
	position = transform.position;
	rotation = transform.rotation;
	scale = transform.scale;

	Update();

	return *this;
}

void Transform::Update()
{
	matrix = ModelMatrix(Space::World);
}

Mat4 Transform::ModelMatrix(Space mode) const
{
	Mat4 translationMat(1);
	switch (mode)
	{
	case Space::Local:
		translationMat = glm::translate(translationMat, localPosition);
		break;

	case Space::World:
		translationMat = glm::translate(translationMat, position + localPosition);
		break;

	case Space::WorldOnly:
		translationMat = glm::translate(translationMat, position);
		break;

	default:
		throw std::runtime_error("Invalid space mode in Transform::ModelMatrix");
	}

	Mat4 rotationMat = glm::mat4_cast(rotation);

	Mat4 scaleMat(1);
	scaleMat = glm::scale(scaleMat, scale);

	// apply in order: scale, rotate, translate
	return translationMat * rotationMat * scaleMat;
}