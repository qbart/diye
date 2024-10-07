#pragma once

#include "shader__apple.hpp"

#ifdef WIN32

const char *VertexShaderSource = R"glsl(
#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 vUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
  gl_Position = projection * view * model * vec4(position, 1.0);
  vUV = uv;
}
)glsl";

// Fragment shader for rendering with texture
const char *FragmentShaderSource = R"glsl(
#version 450 core

in vec2 vUV;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D tileset;

void main(void)
{
  FragColor = texture(tileset, vUV);
}
)glsl";

// Vertex shader for quad
const char *QuadVertexShaderSource = R"glsl(
attribute vec3 position;
attribute vec2 uv;

varying vec2 vUV;

void main(void)
{
    gl_Position = vec4(position, 1.0); 
    vUV = uv;
}
)glsl";

// Fragment shader for rendering texture on quad
const char *QuadFragmentShaderSource = R"glsl(
precision mediump float;

varying vec2 vUV;

uniform sampler2D quad;

void main()
{
    gl_FragData[0] = texture2D(quad, vUV);
}
)glsl";

#endif