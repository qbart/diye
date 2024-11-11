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
        editableMesh = HalfEdgeMesh::NewPlane();

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
        auto eachVertex = [&](const HalfEdge::Vertex::Ptr &v)
        {
            g.Point(v->P, RED);
        };
        editableMesh->EachVertex(eachVertex);

        auto eachFace = [&](const HalfEdge::Face::Ptr &f)
        {
            auto center = f->Center();
            auto dot = Mathf::Dot(-camera.ViewDir(), f->Normal());
            if (dot > 0)
                g.Point(center, BLACK, 5);
            auto e = f->Edge;
            do
            {
                auto start = Mathf::Normalize(center - e->Origin->P) * 0.05f + e->Origin->P;
                auto end = Mathf::Normalize(center - e->Next->Origin->P) * 0.05f + e->Next->Origin->P;
                g.Line(start, end, BLUE);
                e = e->Next;
            } while (e != f->Edge);
        };
        editableMesh->EachFace(eachFace);

        auto eachHalfEdge = [&](const HalfEdge::Ptr &he)
        {
            if (he->IncidentFace == nullptr)
            {
                g.Line(he->Origin->P, he->Next->Origin->P, GREEN);
            }
        };

        editableMesh->EachHalfEdge(eachHalfEdge);
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
