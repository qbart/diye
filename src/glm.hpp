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
#include <glm/gtx/transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

using int32 = std::int32_t;
using int64 = std::int64_t;
using uint = unsigned int;
using byte = std::uint8_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

const Vec3 UP = Vec3(0, 1, 0);


using Dimension = struct
{
    int w, h;
};

class Mathf
{
public:
    Mat4 static Fov(float degAngle, int w, int h, float zNear = 0.001f, float zFar = 1000.0f)
    {
        return glm::perspectiveFov(glm::radians(degAngle), (float)w, (float)h, zNear, zFar);
    }

    Mat4 static Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        return glm::ortho(left, right, bottom, top, zNear, zFar);
    }

    float static Clamp(float val, float min, float max)
    {
        return glm::clamp(val, min, max);
    }

    float static Clamp01(float val)
    {
        return glm::clamp(val, 0.0f, 1.0f);
    }

    float static CloseTo(float a, float b, float epsilon = 0.0001f)
    {
        return glm::epsilonEqual(a, b, epsilon);
    }

    float static Abs(float val)
    {
        return glm::abs(val);
    }
};
