
#define DEBUG_DRAW_IMPLEMENTATION
#include "deps/debug_draw.hpp"
#include "debug_draw_renderer.hpp"
#include "image.hpp"
#include "camera.hpp"

DebugDrawRenderer::DebugDrawRenderer() : mvpMatrix(1.0f), linePointProgram(0), linePointProgram_MvpMatrixLocation(-1), textProgram(0), textProgram_GlyphTextureLocation(-1), textProgram_ScreenDimensions(-1), linePointVAO(0), linePointVBO(0), textVAO(0), textVBO(0)
{
    std::printf("\n");
    std::printf("GL_VENDOR    : %s\n", glGetString(GL_VENDOR));
    std::printf("GL_RENDERER  : %s\n", glGetString(GL_RENDERER));
    std::printf("GL_VERSION   : %s\n", glGetString(GL_VERSION));
    std::printf("GLSL_VERSION : %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::printf("DDRenderInterfaceCoreGL initializing ...\n");

    // Default OpenGL states:
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // This has to be enabled since the point drawing shader will use gl_PointSize.
    glEnable(GL_PROGRAM_POINT_SIZE);

    std::printf("> DDRenderInterfaceCoreGL::setupShaderPrograms()\n");

    //
    // Line/point drawing shader:
    //
    {
        linePointProgram = gl.CreateDefaultProgram(linePointVertShaderSrc, linePointFragShaderSrc);
        gl.UseProgram(linePointProgram);
        glBindAttribLocation(linePointProgram, 0, "in_Position");
        glBindAttribLocation(linePointProgram, 1, "in_ColorPointSize");

        linePointProgram_MvpMatrixLocation = glGetUniformLocation(linePointProgram, "u_MvpMatrix");
        if (linePointProgram_MvpMatrixLocation < 0)
        {
            std::cerr << "Unable to get u_MvpMatrix uniform location!";
        }
    }

    //
    // Text rendering shader:
    //
    {
        textProgram = gl.CreateDefaultProgram(textVertShaderSrc, textFragShaderSrc);
        gl.UseProgram(textProgram);
        glBindAttribLocation(textProgram, 0, "in_Position");
        glBindAttribLocation(textProgram, 1, "in_TexCoords");
        glBindAttribLocation(textProgram, 2, "in_Color");

        textProgram_GlyphTextureLocation = glGetUniformLocation(textProgram, "u_glyphTexture");
        if (textProgram_GlyphTextureLocation < 0)
        {
            std::cerr << "Unable to get u_glyphTexture uniform location!";
        }

        textProgram_ScreenDimensions = glGetUniformLocation(textProgram, "u_screenDimensions");
        if (textProgram_ScreenDimensions < 0)
        {
            std::cerr << "Unable to get u_screenDimensions uniform location!";
        }
    }
    std::printf("> DDRenderInterfaceCoreGL::setupVertexBuffers()\n");

    //
    // Lines/points vertex buffer:
    //
    {
        glGenVertexArrays(1, &linePointVAO);
        glGenBuffers(1, &linePointVBO);

        glBindVertexArray(linePointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

        // RenderInterface will never be called with a batch larger than
        // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
        glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);

        // Set the vertex format expected by 3D points and lines:
        std::size_t offset = 0;

        glEnableVertexAttribArray(0); // in_Position (vec3)
        glVertexAttribPointer(
            /* index     = */ 0,
            /* size      = */ 3,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void *>(offset));
        offset += sizeof(float) * 3;

        glEnableVertexAttribArray(1); // in_ColorPointSize (vec4)
        glVertexAttribPointer(
            /* index     = */ 1,
            /* size      = */ 4,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void *>(offset));

        // VAOs can be a pain in the neck if left enabled...
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //
    // Text rendering vertex buffer:
    //
    {
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);

        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);

        // NOTE: A more optimized implementation might consider combining
        // both the lines/points and text buffers to save some memory!
        glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);

        // Set the vertex format expected by the 2D text:
        std::size_t offset = 0;

        glEnableVertexAttribArray(0); // in_Position (vec2)
        glVertexAttribPointer(
            /* index     = */ 0,
            /* size      = */ 2,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void *>(offset));
        offset += sizeof(float) * 2;

        glEnableVertexAttribArray(1); // in_TexCoords (vec2)
        glVertexAttribPointer(
            /* index     = */ 1,
            /* size      = */ 2,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void *>(offset));
        offset += sizeof(float) * 2;

        glEnableVertexAttribArray(2); // in_Color (vec4)
        glVertexAttribPointer(
            /* index     = */ 2,
            /* size      = */ 4,
            /* type      = */ GL_FLOAT,
            /* normalize = */ GL_FALSE,
            /* stride    = */ sizeof(dd::DrawVertex),
            /* offset    = */ reinterpret_cast<void *>(offset));

        // Ditto.
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    std::printf("DDRenderInterfaceCoreGL ready!\n\n");

    dd::initialize(this);
}

void DebugDrawRenderer::Begin(const Dimension &size, const Camera &camera)
{
    WindowWidth = size.w;
    WindowHeight = size.h;
    mvpMatrix = camera.MVP(glm::mat4(1.0f));
}

void DebugDrawRenderer::Point(const Vec3 &p, const Vec3 &color, float size)
{
    dd::point(glm::value_ptr(p), glm::value_ptr(color), size);
}

void DebugDrawRenderer::Sphere(const Vec3 &p, float radius, const Vec3 &color)
{
    dd::sphere(glm::value_ptr(p), glm::value_ptr(color), radius);
}

void DebugDrawRenderer::Arrow(const Vec3 &from, const Vec3 &to, const Vec3 &color)
{
    dd::arrow(glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), 0.1f, 0.01f);
}

void DebugDrawRenderer::Line(const Vec3 &from, const Vec3 &to, const Vec3 &color)
{
    dd::line(glm::value_ptr(from), glm::value_ptr(to), glm::value_ptr(color), 0.01f);
}

void DebugDrawRenderer::Box(const Vec3 &center, const Vec3 &size, const Vec3 &color)
{
    dd::box(glm::value_ptr(center), glm::value_ptr(color), size.x, size.y, size.z);
}

void DebugDrawRenderer::AxisTriad(const Mat4 &transform)
{
    dd::axisTriad(glm::value_ptr(transform), 0.1f, 1.0f);
}

void DebugDrawRenderer::Frustum(const Camera &camera, const Vec3 &color)
{
    dd::frustum(glm::value_ptr(camera.Clip()), glm::value_ptr(color));
}

void DebugDrawRenderer::Grid(float from, float to, float y, float step, const Vec3 &color)
{
    dd::xzSquareGrid(from, to, y, step, glm::value_ptr(color));
}

void DebugDrawRenderer::Circle(const Vec3 &center, const Vec3 &planeNormal, float radius, const Vec3 &color, float segments)
{
    dd::circle(glm::value_ptr(center), glm::value_ptr(planeNormal), glm::value_ptr(color), radius, segments);
}

void DebugDrawRenderer::Plane(const Vec3 &origin, const Vec3 &normal, float size, const Vec3 &color, float normalSize, const Vec3 &normalColor)
{
    dd::plane(glm::value_ptr(origin), glm::value_ptr(normal), glm::value_ptr(color), glm::value_ptr(normalColor), size * 0.5f, normalSize);
}

void DebugDrawRenderer::Cone(const Vec3 &origin, const Vec3 &dir, const Vec3 &color, float radius, float length)
{
    dd::cone(glm::value_ptr(origin), glm::value_ptr(dir*length), glm::value_ptr(color), 0, radius);
}

void DebugDrawRenderer::Cylinder(const Vec3 &origin, const Vec3 &dir, const Vec3 &color, float radius, float length)
{
    dd::cone(glm::value_ptr(origin), glm::value_ptr(dir*length), glm::value_ptr(color), radius, radius);
}

void DebugDrawRenderer::End()
{
    dd::flush(glfwGetTime());
}

dd::GlyphTextureHandle DebugDrawRenderer::createGlyphTexture(int width, int height, const void *pixels)
{
    assert(width > 0 && height > 0);
    assert(pixels != nullptr);

    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return GLToHandle(textureId);
}

void DebugDrawRenderer::destroyGlyphTexture(dd::GlyphTextureHandle glyphTex)
{
    if (glyphTex == nullptr)
    {
        return;
    }

    const GLuint textureId = handleToGL(glyphTex);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &textureId);
}

void DebugDrawRenderer::drawPointList(const dd::DrawVertex *points, int count, bool depthEnabled)
{
    assert(points != nullptr);
    assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

    glBindVertexArray(linePointVAO);
    glUseProgram(linePointProgram);

    glUniformMatrix4fv(linePointProgram_MvpMatrixLocation,
                       1, GL_FALSE, glm::value_ptr(mvpMatrix));

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), points);

    // Issue the draw call:
    glDrawArrays(GL_POINTS, 0, count);

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugDrawRenderer::drawLineList(const dd::DrawVertex *lines, int count, bool depthEnabled)
{
    assert(lines != nullptr);
    assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

    glBindVertexArray(linePointVAO);
    glUseProgram(linePointProgram);

    glUniformMatrix4fv(linePointProgram_MvpMatrixLocation,
                       1, GL_FALSE, glm::value_ptr(mvpMatrix));

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), lines);

    // Issue the draw call:
    glDrawArrays(GL_LINES, 0, count);

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugDrawRenderer::drawGlyphList(const dd::DrawVertex *glyphs, int count, dd::GlyphTextureHandle glyphTex)
{
    assert(glyphs != nullptr);
    assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

    glBindVertexArray(textVAO);
    glUseProgram(textProgram);

    // These doesn't have to be reset every draw call, I'm just being lazy ;)
    glUniform1i(textProgram_GlyphTextureLocation, 0);
    glUniform2f(textProgram_ScreenDimensions,
                static_cast<GLfloat>(WindowWidth),
                static_cast<GLfloat>(WindowHeight));

    if (glyphTex != nullptr)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, handleToGL(glyphTex));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), glyphs);

    glDrawArrays(GL_TRIANGLES, 0, count); // Issue the draw call

    glDisable(GL_BLEND);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

DebugDrawRenderer::~DebugDrawRenderer()
{
    glDeleteProgram(linePointProgram);
    glDeleteProgram(textProgram);

    glDeleteVertexArrays(1, &linePointVAO);
    glDeleteBuffers(1, &linePointVBO);

    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);

    dd::shutdown();
}

void DebugDrawRenderer::beginDraw()
{
}

void DebugDrawRenderer::endDraw()
{
}

const char *DebugDrawRenderer::linePointVertShaderSrc = "\n"
                                                        "#version 150\n"
                                                        "\n"
                                                        "in vec3 in_Position;\n"
                                                        "in vec4 in_ColorPointSize;\n"
                                                        "\n"
                                                        "out vec4 v_Color;\n"
                                                        "uniform mat4 u_MvpMatrix;\n"
                                                        "\n"
                                                        "void main()\n"
                                                        "{\n"
                                                        "    gl_Position  = u_MvpMatrix * vec4(in_Position, 1.0);\n"
                                                        "    gl_PointSize = in_ColorPointSize.w;\n"
                                                        "    v_Color      = vec4(in_ColorPointSize.xyz, 1.0);\n"
                                                        "}\n";

const char *DebugDrawRenderer::linePointFragShaderSrc = "\n"
                                                        "#version 150\n"
                                                        "\n"
                                                        "in  vec4 v_Color;\n"
                                                        "out vec4 out_FragColor;\n"
                                                        "\n"
                                                        "void main()\n"
                                                        "{\n"
                                                        "    out_FragColor = v_Color;\n"
                                                        "}\n";

const char *DebugDrawRenderer::textVertShaderSrc = "\n"
                                                   "#version 150\n"
                                                   "\n"
                                                   "in vec2 in_Position;\n"
                                                   "in vec2 in_TexCoords;\n"
                                                   "in vec3 in_Color;\n"
                                                   "\n"
                                                   "uniform vec2 u_screenDimensions;\n"
                                                   "\n"
                                                   "out vec2 v_TexCoords;\n"
                                                   "out vec4 v_Color;\n"
                                                   "\n"
                                                   "void main()\n"
                                                   "{\n"
                                                   "    // Map to normalized clip coordinates:\n"
                                                   "    float x = ((2.0 * (in_Position.x - 0.5)) / u_screenDimensions.x) - 1.0;\n"
                                                   "    float y = 1.0 - ((2.0 * (in_Position.y - 0.5)) / u_screenDimensions.y);\n"
                                                   "\n"
                                                   "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
                                                   "    v_TexCoords = in_TexCoords;\n"
                                                   "    v_Color     = vec4(in_Color, 1.0);\n"
                                                   "}\n";

const char *DebugDrawRenderer::textFragShaderSrc = "\n"
                                                   "#version 150\n"
                                                   "\n"
                                                   "in vec2 v_TexCoords;\n"
                                                   "in vec4 v_Color;\n"
                                                   "\n"
                                                   "uniform sampler2D u_glyphTexture;\n"
                                                   "out vec4 out_FragColor;\n"
                                                   "\n"
                                                   "void main()\n"
                                                   "{\n"
                                                   "    out_FragColor = v_Color;\n"
                                                   "    out_FragColor.a = texture(u_glyphTexture, v_TexCoords).r;\n"
                                                   "}\n";
