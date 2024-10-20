#include "gl.hpp"
#include <fmt/core.h>

const std::unordered_map<int, std::string> GL::Types = {
	{GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER"},
	{GL_VERTEX_SHADER, "GL_VERTEX_SHADER"},
	{GL_TESS_CONTROL_SHADER, "GL_TESS_CONTROL_SHADER"},
	{GL_TESS_EVALUATION_SHADER, "GL_TESS_EVALUATION_SHADER"},
	{GL_GEOMETRY_SHADER, "GL_GEOMETRY_SHADER"},
	{GL_FRAGMENT_SHADER, "GL_FRAGMENT_SHADER"},
};

void GL::Defaults()
{
	Enable(GL::Capability::DepthTest);
	Enable(GL::Capability::CullFace);
	Enable(GL::Capability::Blend);
	FrontFace(GL::FrontFace::CounterClockwise);
	CullFace(GL::Face::Back);
	BlendFunc(GL::BlendFactor::SrcAlpha, GL::BlendFactor::OneMinusSrcAlpha);
}

GL::Shader GL::CreateAndCompileShader(ShaderType type, const std::string &src)
{
	auto shader = CreateShader((uint)type);
	ShaderSource(shader, src);
	CompileShader(shader);
	auto compiled = GetShaderInt(shader, ShaderParameter::CompileStatus) == GL_TRUE;
	if (!compiled)
	{
		auto len = GetShaderInt(shader, ShaderParameter::InfoLogLength);
		auto str = GetShaderInfoLog(shader, len);
		fmt::print("Shader compilation failed:\n{}\n", str);
		return 0;
	}

	return shader;
}

GL::Program GL::CreateDefaultProgram(const std::string &vertex, const std::string &frag)
{
	auto vertexShader = CreateAndCompileShader(ShaderType::VertexShader, vertex);
	auto fragShader = CreateAndCompileShader(ShaderType::FragmentShader, frag);

	auto program = CreateProgram();
	AttachShader(program, vertexShader);
	AttachShader(program, fragShader);
	LinkProgram(program);

	auto linked = GetProgramInt(program, ProgramParameter::LinkStatus) == GL_TRUE;
	if (!linked)
	{
		auto len = GetProgramInt(program, ProgramParameter::InfoLogLength);
		auto str = GetProgramInfoLog(program, len);
		fmt::print("Program linking failed:\n{}\n", str);
		return 0;
	}

	ValidateProgram(program);
	if (GLEW_ARB_debug_output)
	{
		auto valid = GetProgramInt(program, ProgramParameter::ValidateStatus) == GL_TRUE;
		if (!valid)
		{
			auto len = GetProgramInt(program, ProgramParameter::InfoLogLength);
			auto str = GetProgramInfoLog(program, len);
			fmt::print("Program validation failed:\n{}\n", str);
			return 0;
		}
	}
	DetachShader(program, vertexShader);
	DetachShader(program, fragShader);
	DeleteShader(vertexShader);
	DeleteShader(fragShader);

	return program;
}
