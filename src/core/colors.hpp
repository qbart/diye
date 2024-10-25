#pragma once

#include "types.hpp"

const Vec3 RED = Vec3(1, 0, 0);
const Vec3 GREEN = Vec3(0, 1, 0);
const Vec3 BLUE = Vec3(0, 0, 1);
const Vec3 WHITE = Vec3(1, 1, 1);
const Vec3 BLACK = Vec3(0, 0, 0);
const Vec3 YELLOW = Vec3(1, 1, 0);
const Vec3 CYAN = Vec3(0, 1, 1);
const Vec3 MAGENTA = Vec3(1, 0, 1);
const Vec3 ORANGE = Vec3(1, 0.5, 0);
const Vec3 PURPLE = Vec3(0.5, 0, 1);
const Vec3 PINK = Vec3(1, 0, 0.5);
const Vec3 LIME = Vec3(0.5, 1, 0);
const Vec3 TEAL = Vec3(0, 1, 0.5);
const Vec3 BROWN = Vec3(0.5, 0.25, 0);
const Vec3 GRAY = Vec3(0.5, 0.5, 0.5);
const Vec3 LIGHT_GRAY = Vec3(0.75, 0.75, 0.75);
const Vec3 DARK_GRAY = Vec3(0.25, 0.25, 0.25);

inline Vec4 rgb(byte r, byte g, byte b, byte a = 1)
{
	return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}