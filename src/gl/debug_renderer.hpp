#pragma once

#include "../core/all.hpp"
#include "../gl/vulkan.hpp"
#include "../vendor/imdd/imdd_simd.h"

struct imdd_vulkan_context_t;
struct imdd_shape_store_tag;

static void imdd_vk_verify(VkResult imdd_vk_result);

namespace gl
{
    class DebugRenderer
    {
    public:
        enum class Style
        {
            Filled,
            Wire
        };

    public:
        DebugRenderer();
        ~DebugRenderer();

        bool Init(const Device &device, const PhysicalDevice &physicalDevice, const RenderPass &renderPass);
        void Shutdown();
        void Begin();
        void End(VkCommandBuffer commandBuffer);
        void CmdDraw(const Camera &camera, VkCommandBuffer commandBuffer);

        void Filled(bool filled = true);
        void Point(const Vec3 &p, const Vec3 &color = WHITE, float size = 0.02f) const;
        void Circle(const Vec3 &center, const Vec3 &planeNormal = UP, float radius = 0.5f, const Vec3 &color = ORANGE) const;
        void Sphere(const Vec3 &p, float radius = 1.0f, const Vec3 &color = GREEN) const;
        void Plane(const Vec3 &origin, const Vec3 &normal = UP, float size = 1.0f, const Vec3 &color = YELLOW, float normalSize = 0, const Vec3 &normalColor = CYAN) const;
        void Arrow(const Vec3 &from, const Vec3 &to, const Vec3 &color = RED) const;
        void Line(const Vec3 &from, const Vec3 &to, const Vec3 &color = WHITE) const;
        void Box(const Vec3 &center, const Vec3 &size, const Vec3 &color = CYAN) const;
        void AxisTriad(const Mat4 &transform, float size = 1.f) const;
        void Frustum(const Camera &camera, const Vec3 &color = PURPLE) const;
        void Grid(float mins = -50.f, float maxs = 50.f, float y = 0, float step = 1.f, const Vec3 &color = GRAY) const;
        void GridSimple(float mins = -5.f, float maxs = 5.f, float y = 0) const;
        void Cone(const Vec3 &origin, const Vec3 &dir, const Vec3 &color = MAGENTA, float radius = 0.5f, float length = 1.0f) const;
        void Cylinder(const Vec3 &origin, const Vec3 &dir, const Vec3 &color = LIME, float radius = 0.5f, float length = 1.0f) const;
        // void Text(const Camera &camera, const Vec3 &pos, const std::string &text, float scale = 1.0f, const Vec3 &color = WHITE, const Vec2 &offset = Vec2(0, 0)) const;

    private:
        uint32_t toColor(const Vec3 &color) const;
        imdd_v4 toVec(const Vec3 &v, float w = 0) const;

    private:
        Style style;
        const Device *device;
        void *memory;
        imdd_shape_store_tag *store;
        imdd_vulkan_context_t *ctx;
    };
}