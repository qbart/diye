#pragma once

#include "core/all.hpp"
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

	mesh.UVs = {
		// Face 1 - Left
		Vec2(0, 0), Vec2(1, 0), Vec2(1, 1),
		Vec2(0, 0), Vec2(1, 1), Vec2(0, 1),

		// Face 2 - Front
		Vec2(0, 0), Vec2(1, 0), Vec2(0, 1),
		Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),

		// Face 3 - Right
		Vec2(0, 0), Vec2(1, 0), Vec2(0, 1),
		Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),

		// Face 4 - Back
		Vec2(0, 0), Vec2(1, 0), Vec2(1, 1),
		Vec2(0, 0), Vec2(1, 1), Vec2(0, 1),

		// Face 5 - Top
		Vec2(0, 0), Vec2(1, 0), Vec2(0, 1),
		Vec2(1, 0), Vec2(1, 1), Vec2(0, 1),

		// Face 6 - Bottom
		Vec2(0, 0), Vec2(1, 0), Vec2(0, 1),
		Vec2(1, 0), Vec2(1, 1), Vec2(0, 1)};

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

auto TiledMesh(float w, float h)
{
	float tileW = 16.0f / 272.0f;
	float tileH = 16.0f / 288.0f;
	float z = 0;
	Geometry mesh;
	int capacity = w * h * 6;	
	mesh.Vertices.reserve(capacity);
	mesh.Indices.reserve(capacity);
	mesh.UVs.reserve(capacity);
	mesh.Colors.reserve(capacity);
	float tile = 0;

	for (int y = 0; y < h; ++y)
	{
		for (int cellx = 0; cellx < w; ++cellx)
		{
			float x = cellx - w * 0.5f;
			// 0
			int i1 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x, y, z));
			// 1
			int i2 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x + 1, y, z));
			// 2
			int i3 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x, y + 1, z));
			// 3
			int i4 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x + 1, y + 1, z));
			// 2
			int i5 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x, y + 1, z));
			// 1
			int i6 = mesh.Vertices.size();
			mesh.Vertices.push_back(Vec3(x + 1, y, z));

			mesh.Indices.push_back(i1);
			mesh.Indices.push_back(i2);
			mesh.Indices.push_back(i3);
			mesh.Indices.push_back(i4);
			mesh.Indices.push_back(i5);
			mesh.Indices.push_back(i6);

			float u1 = tile * tileW;
			float u2 = u1 + tileW;
			float v1 = tile * tileH + tileH;
			float v2 = v1 - tileH;

    // float u2 = TileX * tileW;  // left
    // float v2 = TileY * tileH; // top
    // float u1 = u2 + tileW;     // right
    // float v1 = v2 + tileH;    // bottom

			mesh.UVs.push_back(Vec2(u1, v2)); // 0
			mesh.UVs.push_back(Vec2(u2, v2)); // 1
			mesh.UVs.push_back(Vec2(u1, v1)); // 2
			mesh.UVs.push_back(Vec2(u2, v1)); // 3
			mesh.UVs.push_back(Vec2(u1, v1)); // 2
			mesh.UVs.push_back(Vec2(u2, v2)); // 1

			mesh.Colors.push_back(Vec3(1, 1, 1));
			mesh.Colors.push_back(Vec3(1, 1, 1));
			mesh.Colors.push_back(Vec3(1, 1, 1));
			mesh.Colors.push_back(Vec3(1, 1, 1));
			mesh.Colors.push_back(Vec3(1, 1, 1));
			mesh.Colors.push_back(Vec3(1, 1, 1));

			tile++;
		}
	}

	return std::move(mesh);
}