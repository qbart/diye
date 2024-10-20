#pragma once

#include "glm.hpp"
#include <vector>

using Geometry = struct
{
	std::vector<Vec3> Vertices;
	std::vector<Vec3> Colors;
	std::vector<Vec2> UVs;
	std::vector<uint32> Indices;
};

auto CubeMesh(float size)
{
	Geometry mesh;

	float half = size * 0.5f;
	float top = size;
	float bottom = 0;

	mesh.Vertices = {
		// Face 1
		// left bottom
		Vec3(-half, bottom, -half),
		Vec3(-half, bottom, half),
		Vec3(-half, top, half),
		// left top
		Vec3(-half, bottom, -half),
		Vec3(-half, top, half),
		Vec3(-half, top, -half),
		// Face 2
		// front bottom
		Vec3(-half, bottom, half),
		Vec3(half, bottom, half),
		Vec3(-half, top, half),
		// front top
		Vec3(half, bottom, half),
		Vec3(half, top, half),
		Vec3(-half, top, half),
		// Face 3
		// right bottom
		Vec3(half, bottom, half),
		Vec3(half, bottom, -half),
		Vec3(half, top, half),
		// right top
		Vec3(half, bottom, -half),
		Vec3(half, top, -half),
		Vec3(half, top, half),
		// Face 4
		// back bottom
		Vec3(half, bottom, -half),
		Vec3(-half, bottom, -half),
		Vec3(half, top, -half),
		// back top
		Vec3(-half, bottom, -half),
		Vec3(-half, top, -half),
		Vec3(half, top, -half),
		// Face 5
		// up bottom
		Vec3(half, top, -half),
		Vec3(-half, top, -half),
		Vec3(half, top, half),
		// up top
		Vec3(-half, top, -half),
		Vec3(-half, top, half),
		Vec3(half, top, half),
		// Face 6
		// down bottom
		Vec3(-half, bottom, half),
		Vec3(-half, bottom, -half),
		Vec3(half, bottom, half),
		// down top
		Vec3(-half, bottom, -half),
		Vec3(half, bottom, -half),
		Vec3(half, bottom, half)};

	mesh.Indices = {
		// Face 1
		// left bottom
		0, 1, 2,
		// left top
		3, 4, 5,
		// Face 2
		// front bottom
		6, 7, 8,
		// front top
		9, 10, 11,
		// Face 3
		// right bottom
		12, 13, 14,
		// right top
		15, 16, 17,
		// Face 4
		// back bottom
		18, 19, 20,
		// back top
		21, 22, 23,
		// Face 5
		// up bottom
		24, 25, 26,
		// up top
		27, 28, 29,
		// Face 6
		// down bottom
		30, 31, 32,
		// down top
		33, 34, 35};

	for (auto i = 0; i < 6; ++i)
	{
		mesh.Colors.push_back(Vec3(1, 0, 0));
		mesh.Colors.push_back(Vec3(1, 1, 0));
		mesh.Colors.push_back(Vec3(0, 1, 0));
		mesh.Colors.push_back(Vec3(0, 1, 1));
		mesh.Colors.push_back(Vec3(0, 0, 1));
		mesh.Colors.push_back(Vec3(1, 0, 1));
	}

	return std::move(mesh);
}

auto QuadMesh(float size)
{
	Geometry mesh;

	float half = size * 0.5f;
	float top = size;
	float bottom = 0;

	mesh.Vertices = {
		// front bottom
		Vec3(-half, bottom, half),
		Vec3(half, bottom, half),
		Vec3(-half, top, half),
		// front top
		Vec3(half, bottom, half),
		Vec3(half, top, half),
		Vec3(-half, top, half),
	};

	mesh.Indices = {0, 1, 2, 3, 4, 5};

	mesh.Colors.push_back(Vec3(1, 0, 0));
	mesh.Colors.push_back(Vec3(1, 1, 0));
	mesh.Colors.push_back(Vec3(0, 1, 0));
	mesh.Colors.push_back(Vec3(0, 1, 1));
	mesh.Colors.push_back(Vec3(0, 0, 1));
	mesh.Colors.push_back(Vec3(1, 0, 1));

	mesh.UVs = {
		Vec2(0, 0),
		Vec2(1, 0),
		Vec2(0, 1),
		Vec2(1, 0),
		Vec2(1, 1),
		Vec2(0, 1)};

	return std::move(mesh);
}