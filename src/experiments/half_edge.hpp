#pragma once

#include "../core/all.hpp"
#include "../geometry/half_edge_mesh.hpp"
#include "../gl.hpp"
#include "../shader.hpp"
#include "experiment.hpp"

class HalfEdgeExperiment : public Experiment
{
public:
    HalfEdgeMesh::Ptr editableMesh;
    Mesh mesh;

    uint vao;
    GL::Buffer vbo;
    GL::Buffer uvbo;
    GL::Buffer ibo;
    GL::Buffer cbo;
    GL::Program program;
    GL gl;
    GL::Texture tilesetID;
    GL::Location modelLoc;
    GL::Location viewLoc;
    GL::Location projLoc;

public:
    int Init() override
    {
        fmt::println("Initializing HalfEdgeExperiment");
        // editableMesh = HalfEdgeMesh::NewPlane();
        editableMesh = HalfEdgeMesh::NewCube();

        // fmt::println("Generating mesh");
        // mesh = std::move(halfEdge->GenerateMesh());

        if (mesh.Vertices.size() > 0)
        {
            fmt::println("Init rendering");
            vao = gl.CreateVertexArray();
            vbo = gl.GenBuffer();
            uvbo = gl.GenBuffer();
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

                gl.BindBuffer(GL::BufferType::ElementArray, ibo);
                gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);

                gl.BindVertexArray(0);
                gl.BindBuffer(GL::BufferType::Array, 0);
                gl.BindBuffer(GL::BufferType::ElementArray, 0);
            }

            program = gl.CreateDefaultProgram(VertexShaderSource, FragmentShaderSource);
            if (program == 0)
            {
                fmt::print("Failed to create program\n");
                return -1;
            }
            modelLoc = gl.GetUniformLocation(program, "model");
            viewLoc = gl.GetUniformLocation(program, "view");
            projLoc = gl.GetUniformLocation(program, "projection");
        }

        return 0;
    }

    void Update(float dt) override
    {
    }

    void Debug(const Camera &camera, const DebugDrawRenderer &g)
    {
        auto onDebugDrawLine = [&g](const HalfEdgeMesh::DrawLine &line)
        {
            if (!line.Visible)
                return;

            g.Line(line.From, line.To, line.Boundary ? CYAN : BLUE);
        };
        auto onDebugDrawPoint = [&g](const HalfEdgeMesh::DrawPoint &point)
        {
            if (!point.Visible)
                return;

            if (point.Center)
                g.Point(point.Position, BLACK, 5);
            else
                g.Point(point.Position, GREEN);
        };
        auto onDebugDrawNormal = [&g](const HalfEdgeMesh::DrawNormal &normal)
        {
            if (!normal.Visible)
                return;

            g.Line(normal.From, normal.From + normal.Direction * 0.05f, CYAN);
        };

        auto viewDir = camera.ViewDir();
        editableMesh->OnDebugDrawLine(onDebugDrawLine, viewDir);
        editableMesh->OnDebugDrawPoint(onDebugDrawPoint, viewDir);
        editableMesh->OnDebugDrawNormal(onDebugDrawNormal, viewDir);
    }

    void Render(const Camera &camera, const DebugDrawRenderer &g) override
    {
        Debug(camera, g);

        if (mesh.Vertices.size() > 0)
        {
            auto view = camera.GetViewMatrix();
            auto proj = camera.GetProjection();
            auto mat = Mat4(1);
            gl.UseProgram(program);
            gl.Uniform(modelLoc, mat);
            gl.Uniform(viewLoc, view);
            gl.Uniform(projLoc, proj);
            gl.BindVertexArray(vao);
            gl.BindBuffer(GL::BufferType::Array, vbo);
            gl.BufferData(GL::BufferType::Array, mesh.Vertices, GL::BufferUsage::Stream);
            gl.BindBuffer(GL::BufferType::Array, cbo);
            gl.BufferData(GL::BufferType::Array, mesh.Colors, GL::BufferUsage::Stream);
            gl.BindBuffer(GL::BufferType::ElementArray, ibo);
            gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);
            gl.DrawElements(GL::DrawMode::Triangles, mesh.Indices.size());
            gl.BindBuffer(GL::BufferType::Array, 0);
            gl.BindBuffer(GL::BufferType::ElementArray, 0);
            gl.BindVertexArray(0);
        }
    }

    void RenderUI(UI &ui) override
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
            gl.DeleteVertexArray(vao);
            gl.DeleteProgram(program);
        }
    }
};
