#include "transform.hpp"

Transform::Transform() : position(Vec3(0, 0, 0)),
						 rotation(Quat(0, 0, 0, 1)),
						 scale(Vec3(1, 1, 1))
{
	Update();
}

Transform::Transform(const Transform &transform)
{
	position = transform.position;
	rotation = transform.rotation;
	scale = transform.scale;

	Update();
}

Transform &Transform::operator=(const Transform &transform)
{
	position = transform.position;
	rotation = transform.rotation;
	scale = transform.scale;

	Update();

	return *this;
}

void Transform::Update()
{
	Mat4 translationMat(1);
	translationMat = glm::translate(translationMat, position);

	Mat4 rotationMat = glm::mat4_cast(rotation);

	Mat4 scaleMat(1);
	scaleMat = glm::scale(scaleMat, scale);

	// apply in order: scale, rotate, translate
	matrix = translationMat * rotationMat * scaleMat;
}