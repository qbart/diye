#pragma once

#define IM_VEC2_CLASS_EXTRA                            \
    constexpr ImVec2(glm::vec2 &f) : x(f.x), y(f.y) {} \
    operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC3_CLASS_EXTRA                                          \
    constexpr ImVec3(const glm::vec3 &f) : x(f.x), y(f.y), z(f.z) {} \
    operator glm::vec3() const { return glm::vec3(x, y, z); }

#define IM_VEC4_CLASS_EXTRA                                                  \
    constexpr ImVec4(const glm::vec4 &f) : x(f.x), y(f.y), z(f.z), w(f.w) {} \
    operator glm::vec4() const { return glm::vec4(x, y, z, w); }

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>