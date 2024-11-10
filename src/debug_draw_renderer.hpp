#pragma once

#include "deps/debug_draw.hpp"
#include "core/all.hpp"
#include "gl.hpp"

class DebugDrawRenderer : public dd::RenderInterface
{
public:
    DebugDrawRenderer();

    void Begin(const Dimension &size, const Camera &camera);
    void Point(const Vec3 &p, const Vec3 &color = WHITE, float size = 15.0f) const;
    void Sphere(const Vec3 &p, float radius = 1.0f, const Vec3 &color = GREEN) const;
    void Arrow(const Vec3 &from, const Vec3 &to, const Vec3 &color = RED) const;
    void Line(const Vec3 &from, const Vec3 &to, const Vec3 &color = WHITE) const;
    void Box(const Vec3 &center, const Vec3 &size, const Vec3 &color = CYAN) const;
    void AxisTriad(const Mat4 &transform) const;
    void Frustum(const Camera &camera, const Vec3 &color = PURPLE) const;
    void Grid(float from = -50.f, float to = 50.f, float y = 0, float step = 1.f, const Vec3 &color = GRAY) const;
    void Circle(const Vec3 &center, const Vec3 &planeNormal = UP, float radius = 0.5f, const Vec3 &color = ORANGE, float segments = 16.0f) const;
    void Plane(const Vec3 &origin, const Vec3 &normal = UP, float size = 1.0f, const Vec3 &color = YELLOW, float normalSize = 0, const Vec3 &normalColor = CYAN) const;
    void Cone(const Vec3 &origin, const Vec3 &dir, const Vec3 &color = MAGENTA, float radius = 0.5f, float length = 1.0f) const;
    void Cylinder(const Vec3 &origin, const Vec3 &dir, const Vec3 &color = LIME, float radius = 0.5f, float length = 1.0f) const;
    void End();
    //
    // These are called by dd::flush() before any drawing and after drawing is finished.
    // User can override these to perform any common setup for subsequent draws and to
    // cleanup afterwards. By default, no-ops stubs are provided.
    //
    void beginDraw() override;
    void endDraw() override;

    //
    // Create/free the glyph bitmap texture used by the debug text drawing functions.
    // The debug renderer currently only creates one of those on startup.
    //
    // You're not required to implement these two if you don't care about debug text drawing.
    // Default no-op stubs are provided by default, which disable debug text rendering.
    //
    // Texture dimensions are in pixels, data format is always 8-bits per pixel (Grayscale/GL_RED).
    // The pixel values range from 255 for a pixel within a glyph to 0 for a transparent pixel.
    // If createGlyphTexture() returns null, the renderer will disable all text drawing functions.
    //
    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void *pixels) override;
    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override;

    //
    // Batch drawing methods for the primitives used by the debug renderer.
    // If you don't wish to support a given primitive type, don't override the method.
    //
    void drawPointList(const dd::DrawVertex *points, int count, bool depthEnabled) override;
    void drawLineList(const dd::DrawVertex *lines, int count, bool depthEnabled) override;
    void drawGlyphList(const dd::DrawVertex *glyphs, int count, dd::GlyphTextureHandle glyphTex) override;

    // User defined cleanup. Nothing by default.
    virtual ~DebugDrawRenderer();

private:
    int WindowWidth;
    int WindowHeight;
    glm::mat4 mvpMatrix;

    static GLuint handleToGL(dd::GlyphTextureHandle handle)
    {
        const std::size_t temp = reinterpret_cast<std::size_t>(handle);
        return static_cast<GLuint>(temp);
    }

    static dd::GlyphTextureHandle GLToHandle(const GLuint id)
    {
        const std::size_t temp = static_cast<std::size_t>(id);
        return reinterpret_cast<dd::GlyphTextureHandle>(temp);
    }
    GLuint linePointProgram;
    GLint linePointProgram_MvpMatrixLocation;

    GLuint textProgram;
    GLint textProgram_GlyphTextureLocation;
    GLint textProgram_ScreenDimensions;

    GLuint linePointVAO;
    GLuint linePointVBO;

    GLuint textVAO;
    GLuint textVBO;


    static const char *linePointVertShaderSrc;
    static const char *linePointFragShaderSrc;

    static const char *textVertShaderSrc;
    static const char *textFragShaderSrc;

    GL gl;
    SDLTicks ticks;
};
