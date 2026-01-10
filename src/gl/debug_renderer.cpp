#include "debug_renderer.hpp"

#include "../deps/fmt.hpp"

#define IMDD_IMPLEMENTATION
#include "../vendor/imdd/imdd.h"
#include "../vendor/imdd/imdd_draw_vulkan.h"

#define IMDD_VERIFY(STMT)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(STMT))                                                                                                   \
        {                                                                                                              \
            fprintf(stderr, "failed: %s\n", #STMT);                                                                    \
            exit(-1);                                                                                                  \
        }                                                                                                              \
    } while (0)

#define IMDD_VK_VERIFY(STMT)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        VkResult res = STMT;                                                                                           \
        if (res != VK_SUCCESS)                                                                                         \
        {                                                                                                              \
            fprintf(stderr, "%s failed: %d\n", #STMT, res);                                                            \
            exit(-1);                                                                                                  \
        }                                                                                                              \
    } while (0)

static void imdd_vk_verify(VkResult imdd_vk_result) { IMDD_VK_VERIFY(imdd_vk_result); }

namespace gl
{
DebugRenderer::DebugRenderer() : store(nullptr), ctx(nullptr), memory(nullptr), device(nullptr), style(Style::Filled) {}

DebugRenderer::~DebugRenderer() { Shutdown(); }

bool DebugRenderer::Init(const Device &device, const PhysicalDevice &physicalDevice, const RenderPass &renderPass)
{
    this->device = &device;

    ctx = new imdd_vulkan_context_t;

    auto shapeCount       = 1024;
    uint32_t shapeMemSize = IMDD_APPROX_SHAPE_SIZE_IN_BYTES * shapeCount;
    memory                = malloc(shapeMemSize);
    if (memory == nullptr)
    {
        fmtx::Error("Failed to allocate memory for debug renderer");
        return false;
    }
    fmtx::Debug("Memory allocated for debug renderer");

    store = imdd_init(memory, shapeMemSize);
    if (store == nullptr)
    {
        fmtx::Error("Failed to init debug renderer");
        return false;
    }
    fmtx::Debug("Debug renderer initialized");

    {
        imdd_vulkan_fp_t fp;
        IMDD_VULKAN_SET_GLOBAL_FP(&fp);

        imdd_vulkan_init(
            ctx, shapeCount, shapeCount, shapeCount, &fp, imdd_vk_verify, physicalDevice.handle, device.handle, 0
        );
        fmtx::Debug("Debug render Vulkan backend initialized");
    }

    imdd_vulkan_create_pipelines(ctx, device.handle, renderPass.handle, VK_SAMPLE_COUNT_1_BIT);

    vkDestroyShaderModule(device.handle, ctx->instance_filled_vert, nullptr);
    vkDestroyShaderModule(device.handle, ctx->instance_wire_vert, nullptr);
    vkDestroyShaderModule(device.handle, ctx->array_filled_vert, nullptr);
    vkDestroyShaderModule(device.handle, ctx->array_wire_vert, nullptr);
    vkDestroyShaderModule(device.handle, ctx->filled_frag, nullptr);
    vkDestroyShaderModule(device.handle, ctx->wire_frag, nullptr);

    return true;
}

void DebugRenderer::Shutdown()
{
    if (store != nullptr)
    {
        fmtx::Info("Shutting down debug renderer");
        device->WaitIdle();

        for (int i = 0; i < IMDD_STYLE_COUNT; ++i)
        {
            vkDestroyBuffer(device->handle, ctx->mesh_buffers[i].index_staging_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->mesh_buffers[i].vertex_staging_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->mesh_buffers[i].index_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->mesh_buffers[i].vertex_buffer, nullptr);
        }
        for (int i = 0; i < IMDD_VULKAN_FRAME_COUNT; ++i)
        {
            vkDestroyBuffer(device->handle, ctx->frames[i].instance_transform_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->frames[i].instance_color_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->frames[i].filled_vertex_buffer, nullptr);
            vkDestroyBuffer(device->handle, ctx->frames[i].wire_vertex_buffer, nullptr);
        }
        for (int i = 0; i < IMDD_VULKAN_DESCRIPTOR_COUNT; ++i)
        {
            vkDestroyBuffer(device->handle, ctx->descriptors[i].common_uniform_buffer, nullptr);
        }

        vkFreeMemory(device->handle, ctx->device_memory, nullptr);
        vkFreeMemory(device->handle, ctx->host_memory, nullptr);

        for (int i = 0; i < IMDD_VULKAN_PIPELINE_COUNT; ++i)
        {
            vkDestroyPipeline(device->handle, ctx->pipelines[i], nullptr);
        }
        vkDestroyPipelineLayout(device->handle, ctx->common_pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(device->handle, ctx->common_descriptor_set_layout, nullptr);
        vkDestroyDescriptorPool(device->handle, ctx->descriptor_pool, nullptr);

        delete ctx;
        free(memory);

        memory = nullptr;
        ctx    = nullptr;
        store  = nullptr;
    }
}

void DebugRenderer::Begin()
{
    imdd_reset(store);
    style = Style::Filled;
}

void DebugRenderer::End(VkCommandBuffer commandBuffer)
{
    imdd_vulkan_update(ctx, &store, 1, device->handle, commandBuffer);
}

void DebugRenderer::CmdDraw(const Camera &camera, VkCommandBuffer commandBuffer)
{
    auto P = camera.ViewProjection();

    imdd_vulkan_draw(ctx, glm::value_ptr(P), device->handle, commandBuffer);
}

void DebugRenderer::Filled(bool filled) { style = filled ? Style::Filled : Style::Wire; }

void DebugRenderer::Point(const Vec3 &p, const Vec3 &color, float size) const
{
    imdd_v4 min = {p.x - size / 2, p.y - size / 2, p.z - size / 2, 0};
    imdd_v4 max = {p.x + size / 2, p.y + size / 2, p.z + size / 2, 0};
    imdd_aabb(store, imdd_style_enum_t(style), IMDD_ZMODE_TEST, min, max, toColor(color));
}

void DebugRenderer::Circle(const Vec3 &center, const Vec3 &planeNormal, float radius, const Vec3 &color) const
{
    Cylinder(center, planeNormal, color, radius, 0);
}

void DebugRenderer::Sphere(const Vec3 &p, float radius, const Vec3 &color) const
{
    imdd_sphere(store, imdd_style_enum_t(style), IMDD_ZMODE_TEST, imdd_v4({p.x, p.y, p.z, radius}), toColor(color));
}

void DebugRenderer::Plane(
    const Vec3 &origin,
    const Vec3 &normal,
    float size,
    const Vec3 &color,
    float normalSize,
    const Vec3 &normalColor
) const
{
    auto d       = Mathf::Normalize(normal);
    auto rotAxis = UP;
    if (fabs(d.y) > fabs(d.z)) rotAxis = FORWARD;
    Quat q = Mathf::Rotate(Quat(1, 0, 0, 0), 90, rotAxis);
    Vec3 a = Mathf::Normalize(d * q);
    Vec3 b = Mathf::Normalize(Mathf::Cross(d, a));

    auto halfSize = size * 0.5f;

    Line(origin + a * halfSize + b * halfSize, origin + a * halfSize - b * halfSize, color);
    Line(origin + a * halfSize - b * halfSize, origin - a * halfSize - b * halfSize, color);
    Line(origin - a * halfSize - b * halfSize, origin - a * halfSize + b * halfSize, color);
    Line(origin - a * halfSize + b * halfSize, origin + a * halfSize + b * halfSize, color);

    if (normalSize > 0) Line(origin, origin + normal * normalSize, normalColor);
}

void DebugRenderer::Arrow(const Vec3 &from, const Vec3 &to, const Vec3 &color) const
{
    Cylinder(from, to - from, color, 0.003f, Mathf::Distance(from, to) - 0.05f);
    Cone(to - Mathf::Normalize(to - from) * 0.05f, to - from, color, 0.02f, 0.05f);
}

void DebugRenderer::Line(const Vec3 &from, const Vec3 &to, const Vec3 &color) const
{
    imdd_v4 start = {from.x, from.y, from.z, 0};
    imdd_v4 end   = {to.x, to.y, to.z, 0};
    imdd_line(store, IMDD_ZMODE_TEST, start, end, toColor(color));
}

void DebugRenderer::Box(const Vec3 &center, const Vec3 &size, const Vec3 &color) const
{
    imdd_v4 min = {center.x - size.x / 2, center.y - size.y / 2, center.z - size.z / 2, 0};
    imdd_v4 max = {center.x + size.x / 2, center.y + size.y / 2, center.z + size.z / 2, 0};
    imdd_aabb(store, imdd_style_enum_t(style), IMDD_ZMODE_TEST, min, max, toColor(color));
}

void DebugRenderer::AxisTriad(const Mat4 &transform, float size) const
{
    Vec4 from = transform * Vec4(0, 0, 0, 1);
    Vec4 to   = transform * Vec4(1, 0, 0, 1);
    Vec3 dir  = Vec3(to - from);
    Line(Vec3(from), Vec3(from) + dir * size, RED);

    from = transform * Vec4(0, 0, 0, 1);
    to   = transform * Vec4(0, 1, 0, 1);
    dir  = Vec3(to - from);
    Line(Vec3(from), Vec3(from) + dir * size, GREEN);

    from = transform * Vec4(0, 0, 0, 1);
    to   = transform * Vec4(0, 0, 1, 1);
    dir  = Vec3(to - from);
    Line(Vec3(from), Vec3(from) + dir * size, BLUE);
}

void DebugRenderer::Frustum(const Camera &camera, const Vec3 &color) const
{
    float coneSize = 0.2f;
    auto dir       = camera.ViewDir();
    Cone(camera.Position() + dir * coneSize, -dir, color, 0.1f, coneSize);
    Point(camera.Position(), WHITE, 0.01f);
}

void DebugRenderer::Grid(float mins, float maxs, float y, float step, const Vec3 &color) const
{
    Vec3 from, to;
    for (float i = mins; i <= maxs; i += step)
    {
        // Horizontal line (along the X)
        from.x = mins;
        from.y = y;
        from.z = i;
        to.x   = maxs;
        to.y   = y;
        to.z   = i;
        Line(from, to, color);

        // Vertical line (along the Z)
        from.x = i;
        from.y = y;
        from.z = mins;
        to.x   = i;
        to.y   = y;
        to.z   = maxs;
        Line(from, to, color);
    }
}

void DebugRenderer::GridSimple(float mins, float maxs, float y) const
{
    Grid(mins, maxs, y, 0.25f, BLACK);
    Grid(mins, maxs, y, 1.f, GRAY);
}

void DebugRenderer::Cone(const Vec3 &origin, const Vec3 &dir, const Vec3 &color, float radius, float length) const
{
    auto d       = Mathf::Normalize(dir);
    auto rotAxis = UP;
    if (fabs(d.y) > fabs(d.z)) rotAxis = FORWARD;
    Quat q = Mathf::Rotate(Quat(1, 0, 0, 0), 90, rotAxis);
    Vec3 a = Mathf::Normalize(d * q);
    Vec3 b = Mathf::Normalize(Mathf::Cross(d, a));

    imdd_v4 x    = toVec(a * radius);
    imdd_v4 y    = toVec(b * radius);
    imdd_v4 z    = toVec(d * -length);
    imdd_v4 apex = toVec(origin + d * length);

    imdd_cone(store, imdd_style_enum_t(style), IMDD_ZMODE_TEST, x, y, z, apex, toColor(color));
}

void DebugRenderer::Cylinder(const Vec3 &origin, const Vec3 &dir, const Vec3 &color, float radius, float length) const
{
    auto d       = Mathf::Normalize(dir);
    auto rotAxis = UP;
    if (fabs(d.y) > fabs(d.z)) rotAxis = FORWARD;
    Quat q = Mathf::Rotate(Quat(1, 0, 0, 0), 90, rotAxis);
    Vec3 a = Mathf::Normalize(d * q);
    Vec3 b = Mathf::Normalize(Mathf::Cross(d, a));

    imdd_v4 x    = toVec(a * radius);
    imdd_v4 y    = toVec(b * radius);
    imdd_v4 z    = toVec(d * -length * 0.5f);
    imdd_v4 apex = toVec(origin + d * length * 0.5f);

    imdd_cylinder(store, imdd_style_enum_t(style), IMDD_ZMODE_TEST, x, y, z, apex, toColor(color));
}

uint32_t DebugRenderer::toColor(const Vec3 &color) const
{
    return 255 << 24 | (uint32_t(color.z * 255) << 16) | (uint32_t(color.y * 255) << 8) | uint32_t(color.x * 255);
}

imdd_v4 DebugRenderer::toVec(const Vec3 &v, float w) const { return imdd_v4({v.x, v.y, v.z, w}); }
} // namespace gl