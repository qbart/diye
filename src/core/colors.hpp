#pragma once

#include "types.hpp"

const Vec4 RED = Vec4(1, 0, 0, 1);
const Vec4 GREEN = Vec4(0, 1, 0, 1);
const Vec4 BLUE = Vec4(0, 0, 1, 1);
const Vec4 WHITE = Vec4(1, 1, 1, 1);
const Vec4 BLACK = Vec4(0, 0, 0, 1);
const Vec4 YELLOW = Vec4(1, 1, 0, 1);
const Vec4 CYAN = Vec4(0, 1, 1, 1);
const Vec4 MAGENTA = Vec4(1, 0, 1, 1);
const Vec4 ORANGE = Vec4(1, 0.5, 0, 1);
const Vec4 PURPLE = Vec4(0.5, 0, 1, 1);
const Vec4 PINK = Vec4(1, 0, 0.5, 1);
const Vec4 LIME = Vec4(0.5, 1, 0, 1);
const Vec4 TEAL = Vec4(0, 1, 0.5, 1);
const Vec4 BROWN = Vec4(0.5, 0.25, 0, 1);
const Vec4 GRAY = Vec4(0.5, 0.5, 0.5, 1);
const Vec4 LIGHT_GRAY = Vec4(0.75, 0.75, 0.75, 1);
const Vec4 DARK_GRAY = Vec4(0.25, 0.25, 0.25, 1);

inline Vec4 rgb(byte r, byte g, byte b, byte a = 255)
{
	return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}