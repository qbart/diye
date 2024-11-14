#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vector_relational.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>
#include <vector>
#include <fmt/format.h>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

using uint8 = std::uint8_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint = unsigned int;
using byte = std::uint8_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

const Vec3 ZERO = Vec3(0, 0, 0);
const Vec3 UP = Vec3(0, 1, 0);
const Vec3 FORWARD = Vec3(0, 0, -1);
const Vec3 LEFT = Vec3(-1, 0, 0);

using Dimension = struct
{
    int w, h;
};

static std::string ToString(const Vec3 &v)
{
    return fmt::format("({},{},{})", v.x, v.y, v.z);
}