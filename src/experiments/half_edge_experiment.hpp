#pragma once

#include "../core/all.hpp"
#include "../geometry/half_edge_mesh.hpp"
#include "../gl.hpp"
#include "experiment.hpp"

std::string HalfEdgeVertexShaderSource();
std::string HalfEdgeFragmentShaderSource();

class HalfEdgeExperiment : public Experiment
{
public:
    HalfEdgeMesh::Ptr editableMesh;
    HalfEdgeMeshSelection selection;
    Mesh mesh;

    uint vao;
    GL::Buffer vbo;
    GL::Buffer nbo;
    GL::Buffer ibo;
    GL::Buffer cbo;
    GL::Program program;
    GL gl;
    GL::Texture tilesetID;
    GL::Location mvpLoc;
    GL::Location modelLoc;
    GL::Location viewDirLoc;
    GL::Location camPositionLoc;
    GL::Location camDistanceLoc;
    Transform transform;
    Window *window;

    bool debug = false;
    Ray ray;

    Transform extrudeTransform;

public:
    int Init(Window *window) override
    {
        this->window = window;
        fmt::println("Initializing HalfEdgeExperiment");
        // editableMesh = HalfEdgeMesh::NewPlane();
        editableMesh = HalfEdgeMesh::NewCube();

        fmt::println("Generating mesh");
        mesh = editableMesh->GenerateMesh(false);

        if (mesh.Vertices.size() > 0)
        {
            fmt::println("Init rendering");
            vao = gl.CreateVertexArray();
            vbo = gl.GenBuffer();
            nbo = gl.GenBuffer();
            ibo = gl.GenBuffer();
            cbo = gl.GenBuffer();

            {
                gl.BindVertexArray(vao);

                gl.EnableVertexAttribArray(0);
                gl.BindBuffer(GL::BufferType::Array, vbo);
                gl.BufferData(GL::BufferType::Array, mesh.Vertices, GL::BufferUsage::Stream);
                gl.VertexAttribPointer(0, 3);

                gl.EnableVertexAttribArray(1);
                gl.BindBuffer(GL::BufferType::Array, cbo);
                gl.BufferData(GL::BufferType::Array, mesh.Colors, GL::BufferUsage::Stream);
                gl.VertexAttribPointer(1, 3);

                gl.EnableVertexAttribArray(2);
                gl.BindBuffer(GL::BufferType::Array, nbo);
                gl.BufferData(GL::BufferType::Array, mesh.Normals, GL::BufferUsage::Stream);
                gl.VertexAttribPointer(2, 3);

                gl.BindBuffer(GL::BufferType::ElementArray, ibo);
                gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);

                gl.BindVertexArray(0);
                gl.BindBuffer(GL::BufferType::Array, 0);
                gl.BindBuffer(GL::BufferType::ElementArray, 0);
            }

            program = gl.CreateDefaultProgram(HalfEdgeVertexShaderSource(), HalfEdgeFragmentShaderSource());
            if (program == 0)
            {
                fmt::print("Failed to create program\n");
                return -1;
            }
            mvpLoc = gl.GetUniformLocation(program, "MVP");
            modelLoc = gl.GetUniformLocation(program, "model");
            viewDirLoc = gl.GetUniformLocation(program, "viewDir");
            camPositionLoc = gl.GetUniformLocation(program, "camPosition");
            camDistanceLoc = gl.GetUniformLocation(program, "camDistance");
        }

        return 0;
    }

    void Update(float dt) override
    {
        transform.Update();
        if (window->KeyJustReleased(SDLK_TAB))
            debug = !debug;

        if (window->KeyJustReleased(SDLK_1))
        {
            editableMesh = HalfEdgeMesh::NewPlane();
            mesh = editableMesh->GenerateMesh(false);
        }

        if (window->KeyJustReleased(SDLK_2))
        {
            editableMesh = HalfEdgeMesh::NewCube();
            mesh = editableMesh->GenerateMesh(false);
        }
    }

    void RenderDebug(const Camera &camera, const DebugDrawRenderer &g) override
    {
        if (!debug)
            return;

        if (selection.IsSelected())
        {
            // auto face = selection.SelectedFaces[0];
            // g.Arrow(extrudeTransform.position, extrudeTransform.position + face->Normal() * 0.25f, ORANGE);
            // editableMesh->Extrude(face, 0.1f);
            // mesh = editableMesh->GenerateMesh(true);
        }

        auto camPos = camera.Position();

        auto onDebugDrawLine = [&](const HalfEdgeMesh::DrawLine &line)
        {
            if (!line.Visible)
                return;

            g.Line(line.From, line.To, line.Boundary ? CYAN : BLUE);
        };
        auto onDebugDrawPoint = [&](const HalfEdgeMesh::DrawPoint &point)
        {
            if (!point.Visible)
                return;

            if (point.Center)
                g.Point(point.Position, BLACK, 5);
            else
            {
                g.Point(point.Position, GREEN, 10);
                // g.Text(camera, point.Position, ToString(point.Position), 0.5, GREEN, Vec2(0, 30));
            }
        };
        auto onDebugDrawNormal = [&](const HalfEdgeMesh::DrawNormal &normal)
        {
            if (!normal.Visible)
                return;

            g.Line(normal.From, normal.From + normal.Direction * 0.05f, CYAN);
        };

        editableMesh->DebugDrawLine(onDebugDrawLine, camPos);
        editableMesh->DebugDrawPoint(onDebugDrawPoint, camPos);
        editableMesh->DebugDrawNormal(onDebugDrawNormal, camPos);

        auto eachEdgeOfSelectedFace = [&](const HalfEdgeMesh::DrawLine &line)
        {
            g.Line(line.From, line.To, ORANGE);
        };
        selection.DrawLine(eachEdgeOfSelectedFace, camPos);
    }

    void Render(const Camera &camera) override
    {
        if (window->KeyJustReleased(SDLK_TAB))
            debug = !debug;

        if (window->MouseButtonUp(SDL_BUTTON_LEFT))
        {
            ray = camera.ScreenToRay(window->MousePosition(), window->Size());
            auto hit = editableMesh->Raycast(ray);
            if (hit.Hit())
            {
                selection.Select(hit.Face);
                extrudeTransform.position = hit.Center;
                extrudeTransform.Update();
            }
            
            // test extrude
            // editableMesh->Extrude(hit.Face, 0.5f);
            selection.Clear();
            editableMesh->DeleteFace(hit.Face);
            mesh = editableMesh->GenerateMesh(false);
        }

        if (debug)
            return;

        if (mesh.Vertices.size() > 0)
        {
            auto model = transform.GetModelMatrix();
            auto mvp = camera.MVP(model);
            gl.UseProgram(program);
            gl.Uniform(mvpLoc, mvp);
            gl.Uniform(viewDirLoc, camera.ViewDir());
            gl.Uniform(camPositionLoc, camera.Position());
            gl.Uniform(camDistanceLoc, Mathf::Distance(camera.Position(), transform.position));
            gl.Uniform(modelLoc, model);
            gl.BindVertexArray(vao);
            gl.BindBuffer(GL::BufferType::Array, vbo);
            gl.BufferData(GL::BufferType::Array, mesh.Vertices, GL::BufferUsage::Stream);
            gl.BindBuffer(GL::BufferType::Array, cbo);
            gl.BufferData(GL::BufferType::Array, mesh.Colors, GL::BufferUsage::Stream);
            gl.BindBuffer(GL::BufferType::Array, nbo);
            gl.BufferData(GL::BufferType::Array, mesh.Normals, GL::BufferUsage::Stream);
            gl.BindBuffer(GL::BufferType::ElementArray, ibo);
            gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);
            gl.DrawElements(GL::DrawMode::Triangles, mesh.Indices.size());
            gl.BindBuffer(GL::BufferType::Array, 0);
            gl.BindBuffer(GL::BufferType::ElementArray, 0);
            gl.BindVertexArray(0);
        }
    }

    void RenderUI(const Camera &camera, UI &ui) override
    {
    }

    void Shutdown() override
    {
        if (mesh.Vertices.size() > 0)
        {
            gl.BindBuffer(GL::BufferType::Array, 0);
            gl.BindBuffer(GL::BufferType::ElementArray, 0);
            gl.BindVertexArray(0);
            gl.DeleteTexture(tilesetID);
            gl.DeleteBuffer(vbo);
            gl.DeleteBuffer(ibo);
            gl.DeleteBuffer(cbo);
            gl.DeleteBuffer(nbo);
            gl.DeleteVertexArray(vao);
            gl.DeleteProgram(program);
        }
    }
};

std::string HalfEdgeVertexShaderSource()
{
    return R"glsl(

#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

out vec3 vertColor;
out vec3 vertPos;
out vec3 normalDir;

uniform vec3 viewDir;
uniform vec3 camPosition;
uniform float camDistance;
uniform mat4 MVP;
uniform mat4 model;

void main(void) {
  vec4 worldPos = model * vec4(position, 1.0);
  gl_Position = MVP * vec4(position, 1.0);
  vertColor = color;
  normalDir = normal;
  vertPos = worldPos.xyz;
}

)glsl";
}

std::string HalfEdgeFragmentShaderSource()
{
    return R"glsl(

#version 450 core

in vec3 vertColor;
in vec3 vertPos;
in vec3 normalDir;

uniform vec3 viewDir;
uniform vec3 camPosition;
uniform float camDistance;

out vec4 FragColor;

float remap(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main(void)
{
  vec3 color = vertColor;
  
  // Distance-based shading
  float dist = distance(camPosition, vertPos);
  float distanceFactor = remap(camDistance-dist, 0.0, camDistance, 0,1);
//   distanceFactor = remap(clamp(distanceFactor, 0.0, 1.0), 0.0, 1.0, 0.0, 0.4);
  distanceFactor = clamp(distanceFactor, 0.0, 1.0);
  distanceFactor = pow(distanceFactor, 2.0);
  
  // Apply distance effect to color
  color *= distanceFactor;
//   float viewFactor = dot(normalize(-viewDir), normalize(normalDir));
//   viewFactor = remap(viewFactor, -1.0, 1.0, 0.0, 1.0);
  float viewFactor = 1;

  FragColor = vec4(clamp(color * viewFactor, 0.0, 1.0), 1.0);

    // FragColor = vec4(vertPos, 1.0);
    FragColor = vec4(vertColor, 1.0);
}

)glsl";
}