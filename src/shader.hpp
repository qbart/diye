#pragma once

#include "shader__apple.hpp"

#ifdef WIN32

const char *VertexShaderSource = R"glsl(
#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

out vec3 vertColor;
out vec2 vertUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
  gl_Position = projection * view * model * vec4(position, 1.0);
  vertColor = color;
  vertUV = uv;
}
)glsl";

// Fragment shader for rendering with texture
const char *FragmentShaderSource = R"glsl(
#version 450 core

in vec3 vertColor;
in vec2 vertUV;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D tileset;

void main(void)
{
  FragColor = texture(tileset, vertUV);
  // FragColor = texture(tileset, vertUV) * vec4(vertColor, 1.0);
  // FragColor = vec4(vertColor, 1.0);
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