#pragma once

#include "core/all.hpp"
#include "image.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <iostream>
#include <unordered_map>

struct GL
{
	using Texture = uint;
	using Buffer = uint;
	using Framebuffer = uint;
	using VertexArrayObject = uint;
	using Location = uint;
	using Shader = uint;
	using Program = uint;
	using AttribLocation = uint32;
	using FramebufferStatus = int;
	using UniformLocation = int;

	enum class Face
	{
		Back = GL_BACK,
		Front = GL_FRONT
	};

	enum class FrontFace
	{
		Clockwise = GL_CW,
		CounterClockwise = GL_CCW
	};

	enum class DrawMode
	{
		Triangles = GL_TRIANGLES,
	};
	enum class TextureType
	{
		Texture2D = GL_TEXTURE_2D
	};
	enum class TextureParameterName
	{
		MinFilter = GL_TEXTURE_MIN_FILTER,
		MagFilter = GL_TEXTURE_MAG_FILTER,
		WrapS = GL_TEXTURE_WRAP_S,
		WrapT = GL_TEXTURE_WRAP_T
	};
	enum class TextureParameter
	{
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR,
		LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
		LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
		ClampToEdge = GL_CLAMP_TO_EDGE
	};
	enum class TextureUnit
	{
		Texture0 = GL_TEXTURE0
	};
	enum class FramebufferTarget
	{
		Draw = GL_DRAW_FRAMEBUFFER,
		Read = GL_READ_FRAMEBUFFER,
		Framebuffer = GL_FRAMEBUFFER
	};
	enum class FramebufferAttachment
	{
		Depth = GL_DEPTH_ATTACHMENT,
		Stencil = GL_STENCIL_ATTACHMENT,
		DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
		Color0 = GL_COLOR_ATTACHMENT0,
		Color1 = GL_COLOR_ATTACHMENT1,
		Color2 = GL_COLOR_ATTACHMENT2,
		Color3 = GL_COLOR_ATTACHMENT3,
		Color4 = GL_COLOR_ATTACHMENT4,
		Color5 = GL_COLOR_ATTACHMENT5,
		Color6 = GL_COLOR_ATTACHMENT6,
		Color7 = GL_COLOR_ATTACHMENT7,
		Color8 = GL_COLOR_ATTACHMENT8,
		Color9 = GL_COLOR_ATTACHMENT9,
		Color10 = GL_COLOR_ATTACHMENT10,
		Color11 = GL_COLOR_ATTACHMENT11,
		Color12 = GL_COLOR_ATTACHMENT12,
		Color13 = GL_COLOR_ATTACHMENT13,
		Color14 = GL_COLOR_ATTACHMENT14,
		Color15 = GL_COLOR_ATTACHMENT15
	};
	enum class BufferUsage
	{
		Static = GL_STATIC_DRAW,
		Dynamic = GL_DYNAMIC_DRAW,
		Stream = GL_STREAM_DRAW
	};
	enum class BufferType
	{
		Array = GL_ARRAY_BUFFER,
		ElementArray = GL_ELEMENT_ARRAY_BUFFER,
		Uniform = GL_UNIFORM_BUFFER
	};
	enum class PixelFormat
	{
		RGB = GL_RGB,
		RGBA = GL_RGBA
	};
	enum class ShaderType
	{
		VertexShader = GL_VERTEX_SHADER,
		FragmentShader = GL_FRAGMENT_SHADER
	};
	enum class Capability : uint
	{
		DepthTest = GL_DEPTH_TEST,
		CullFace = GL_CULL_FACE,
		Blend = GL_BLEND
	};
	enum class Type
	{
		Float = GL_FLOAT,
		UInt = GL_UNSIGNED_INT,
		Byte = GL_BYTE
	};

	enum class BufferMask
	{
		ColorBufferBit = GL_COLOR_BUFFER_BIT,
		DepthBufferBit = GL_DEPTH_BUFFER_BIT,
		StencilBufferBit = GL_STENCIL_BUFFER_BIT
	};
	enum class BlendFactor
	{
		SrcAlpha = GL_SRC_ALPHA,
		OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA
	};
	enum class ShaderParameter
	{
		CompileStatus = GL_COMPILE_STATUS,
		InfoLogLength = GL_INFO_LOG_LENGTH
	};
	enum class ProgramParameter
	{
		LinkStatus = GL_LINK_STATUS,
		ValidateStatus = GL_VALIDATE_STATUS,
		InfoLogLength = GL_INFO_LOG_LENGTH
	};
	enum class Parameter
	{
		MaxCombinedTextureImageUnits = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		MaxUniformBlockSize = GL_MAX_UNIFORM_BLOCK_SIZE,
		MaxShaderStorageBlockSize = GL_MAX_SHADER_STORAGE_BLOCK_SIZE,
		MaxTextureImageUnits = GL_MAX_TEXTURE_IMAGE_UNITS,
		MaxVertexTextureImageUnits = GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		MaxTextureSize = GL_MAX_TEXTURE_SIZE,
		MaxRenderbufferSize = GL_MAX_RENDERBUFFER_SIZE,
		MaxVertexAttribs = GL_MAX_VERTEX_ATTRIBS,
		MaxVaryingVectors = GL_MAX_VARYING_VECTORS,
		MaxVertexUniformVectors = GL_MAX_VERTEX_UNIFORM_VECTORS,
		MaxFragmentUniformVectors = GL_MAX_FRAGMENT_UNIFORM_VECTORS,
		MaxFramebufferHeight = GL_MAX_FRAMEBUFFER_HEIGHT,
		MaxFramebufferWidth = GL_MAX_FRAMEBUFFER_WIDTH,
		MaxFramebufferSamples = GL_MAX_FRAMEBUFFER_SAMPLES,
		MaxFramebufferLayers = GL_MAX_FRAMEBUFFER_LAYERS,
		MaxViewport = GL_MAX_VIEWPORTS,
		MaxCubeMapTextureSize = GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		MaxTextureBufferSize = GL_MAX_TEXTURE_BUFFER_SIZE,
		MaxDepthTextureSamples = GL_MAX_DEPTH_TEXTURE_SAMPLES,
		MaxRectangleTextureSize = GL_MAX_RECTANGLE_TEXTURE_SIZE,
		MaxTextureMaxAnisotropy = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
	};

	static const std::unordered_map<int, std::string> Types;

	std::string GetString(uint name)
	{
		return std::string(reinterpret_cast<const char *>(glGetString(name)));
	}

	int GetInt(uint name)
	{
		int val;
		glGetIntegerv(name, &val);
		return val;
	}

	float GetFloat(uint name)
	{
		float val;
		glGetFloatv(name, &val);
		return val;
	}

	void Enable(Capability cap)
	{
		glEnable((uint)cap);
	}

	void Disable(Capability cap)
	{
		glDisable((uint)cap);
	}

	void BlendFunc(BlendFactor src, BlendFactor dst)
	{
		glBlendFunc((uint)src, (uint)dst);
	}

	void Clear(BufferMask mask)
	{
		glClear((uint)mask);
	}

	void ColorColorBuffer(const Vec3 &floats, int drawBuffer = 0)
	{
		Vec4 f(floats, 1);
		glClearBufferfv(GL_COLOR, drawBuffer, &f[0]);
	}

	void ClearDepthBuffer(float val = 1)
	{
		glClearBufferfv(GL_DEPTH, 0, &val);
	}

	Texture GenTexture()
	{
		uint tex;
		glGenTextures(1, &tex);
		return tex;
	}

	void DeleteTexture(Texture tex)
	{
		glDeleteTextures(1, &tex);
	}

	void ActiveTexture(TextureUnit unit)
	{
		glActiveTexture((uint)unit);
	}

	void BindTexture(TextureType target, Texture tex)
	{
		glBindTexture((uint)target, tex);
	}

	void GenerateMipmap(TextureType target)
	{
		glGenerateMipmap((uint)target);
	}

	void TextureParameter(TextureType target, TextureParameterName name, TextureParameter param)
	{
		glTexParameteri((uint)target, (uint)name, (uint)param);
	}

	void TextureImage2D(TextureType target, const Image &image)
	{
		PixelFormat format = PixelFormat::RGBA;
		if (image.Channels == 3)
		{
			format = PixelFormat::RGB;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		glTexImage2D(
			(uint)target,
			0,
			(int)format,
			image.Width, image.Height,
			0,
			(uint)format,
			GL_UNSIGNED_BYTE,
			image.GetPixelData());
	}

	Framebuffer CreateFramebuffer()
	{
		uint fb;
		glGenFramebuffers(1, &fb);
		return fb;
	}

	void Viewport(int x, int y, int w, int h)
	{
		glViewport(x, y, w, h);
	}

	void Viewport(int w, int h)
	{
		glViewport(0, 0, w, h);
	}

	Shader CreateShader(uint type)
	{
		return glCreateShader(type);
	}

	void ShaderSource(Shader shader, const std::string &src)
	{
		auto c_str = src.c_str();
		glShaderSource(shader, 1, &c_str, nullptr);
	}

	void CompileShader(Shader shader)
	{
		glCompileShader(shader);
	}

	int GetShaderInt(Shader shader, ShaderParameter param) const
	{
		int val;
		glGetShaderiv(shader, (uint)param, &val);
		return val;
	}

	std::string GetShaderInfoLog(Shader shader, int bufSize) const
	{
		std::vector<char> log;
		if (bufSize > 0)
		{
			log.resize(bufSize);
			int len;
			glGetShaderInfoLog(shader, bufSize, &len, &log[0]);
		}
		return std::string(std::begin(log), std::end(log));
	}

	void AttachShader(Program program, Shader shader)
	{
		glAttachShader(program, shader);
	}

	void DetachShader(Program program, Shader shader)
	{
		glDetachShader(program, shader);
	}

	void DeleteShader(Shader shader)
	{
		glDeleteShader(shader);
	}

	Program CreateProgram()
	{
		return glCreateProgram();
	}

	void DeleteProgram(Program program)
	{
		glDeleteProgram(program);
	}

	void UseProgram(Program program)
	{
		glUseProgram(program);
	}

	void LinkProgram(Program program)
	{
		glLinkProgram(program);
	}

	void ValidateProgram(Program program)
	{
		glValidateProgram(program);
	}

	int GetProgramInt(Program program, ProgramParameter param)
	{
		int val;
		glGetProgramiv(program, (uint)param, &val);
		return val;
	}

	std::string GetProgramInfoLog(Program program, int bufSize)
	{
		std::vector<char> log;
		if (bufSize > 0)
		{
			log.resize(bufSize);
			glGetProgramInfoLog(program, bufSize, nullptr, log.data());
		}
		return std::string(std::begin(log), std::end(log));
	}

	UniformLocation GetUniformLocation(Program program, const std::string &name)
	{
		return glGetUniformLocation(program, name.c_str());
	}

	void Uniform(uint loc, const Vec3 &v)
	{
		glUniform3fv(loc, 1, glm::value_ptr(v));
	}

	void Uniform(uint loc, const Mat4 &mat)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
	}

	uint CreateVertexArray()
	{
		uint va;
		glCreateVertexArrays(1, &va);
		return va;
	}

	void BindVertexArray(uint va)
	{
		glBindVertexArray(va);
	}

	void DeleteVertexArray(uint va)
	{
		glDeleteVertexArrays(1, &va);
	}

	void EnableVertexAttribArray(uint index)
	{
		glEnableVertexAttribArray(index);
	}

	void DisableVertexAttribArray(uint index)
	{
		glDisableVertexAttribArray(index);
	}

	void VertexAttribPointer(uint index, int size, uint type)
	{
		glVertexAttribPointer(index, size, type, GL_FALSE, 0, (void *)0);
	}

	void VertexAttribPointer(uint index, int size)
	{
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, (void *)0);
	}

	Buffer GenBuffer()
	{
		uint buf;
		glGenBuffers(1, &buf);
		return buf;
	}

	void DeleteBuffer(uint buf)
	{
		glDeleteBuffers(1, &buf);
	}

	void BindBuffer(BufferType target, Buffer buf)
	{
		glBindBuffer((uint)target, buf);
	}

	void BufferData(BufferType target, const std::vector<Vec3> &vv, BufferUsage usage = BufferUsage::Static)
	{
		glBufferData((uint)target, sizeof(Vec3) * vv.size(), &vv[0], (uint)usage);
	}

	void BufferData(BufferType target, const std::vector<Vec2> &vv, BufferUsage usage = BufferUsage::Static)
	{
		glBufferData((uint)target, sizeof(Vec2) * vv.size(), &vv[0], (uint)usage);
	}

	void BufferData(BufferType target, const std::vector<uint32> &vv, BufferUsage usage = BufferUsage::Static)
	{
		glBufferData((uint)target, sizeof(uint32) * vv.size(), &vv[0], (uint)usage);
	}

	void DrawArrays(DrawMode mode, int offset, int count)
	{
		glDrawArrays((uint)mode, offset, count);
	}

	void DrawElements(DrawMode mode, int count)
	{
		glDrawElements((uint)mode, sizeof(uint32) * count, GL_UNSIGNED_INT, nullptr);
	}

	void DrawElementsBaseVertex(DrawMode mode, int count, int offset = 0)
	{
		glDrawElementsBaseVertex((uint)mode, sizeof(uint32) * count, GL_UNSIGNED_INT, nullptr, offset);
	}

	void CullFace(Face face = Face::Back)
	{
		glCullFace((uint)face);
	}

	void FrontFace(FrontFace val)
	{
		glFrontFace((uint)val);
	}

	// extended

	void Defaults();
	Shader CreateAndCompileShader(ShaderType type, const std::string &src);
	Program CreateDefaultProgram(const std::string &vertex, const std::string &frag);
	Texture CreateDefaultTexture(const Image &image);
};

struct Bytes
{
public:
	explicit Bytes(int _bytes) : bytes(_bytes) {}

	int KiB() const
	{
		return bytes / 1024;
	}

	int MiB() const
	{
		return bytes / 1024 / 1024;
	}

	std::string mib_s() const
	{
		std::stringstream ss;
		ss << MiB();
		ss << " MB";
		return ss.str();
	}

	std::string kib_s() const
	{
		std::stringstream ss;
		ss << KiB();
		ss << " KB";
		return ss.str();
	}

private:
	int bytes;
};

static void gl_printInfo()
{
	GL gl;

	std::cout << "gl:            " << gl.GetString(GL_VERSION) << "\n";
	std::cout << "glsl.version:  " << gl.GetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
	std::cout << "ssbo.max:      " << Bytes(gl.GetInt(GL_MAX_SHADER_STORAGE_BLOCK_SIZE)).mib_s() << "\n";
	std::cout << "ubo.max:       " << Bytes(gl.GetInt(GL_MAX_UNIFORM_BLOCK_SIZE)).kib_s() << "\n";
	std::cout << "viewports.max: " << gl.GetInt(GL_MAX_VIEWPORTS) << "\n";
	std::cout << "\n";
	std::cout << "framebuffer.width.max:   " << gl.GetInt(GL_MAX_FRAMEBUFFER_WIDTH) << "\n";
	std::cout << "framebuffer.height.max:  " << gl.GetInt(GL_MAX_FRAMEBUFFER_HEIGHT) << "\n";
	std::cout << "framebuffer.layers.max:  " << gl.GetInt(GL_MAX_FRAMEBUFFER_LAYERS) << "\n";
	std::cout << "framebuffer.samples.max: " << gl.GetInt(GL_MAX_FRAMEBUFFER_SAMPLES) << "\n";
	std::cout << "\n";
	std::cout << "texture.max.size: " << gl.GetInt(GL_MAX_TEXTURE_SIZE) << "\n";
	std::cout << "texture_image.max.units: " << gl.GetInt(GL_MAX_TEXTURE_IMAGE_UNITS) << "\n";
	std::cout << "texture_buffer.max.size: " << gl.GetInt(GL_MAX_TEXTURE_BUFFER_SIZE) << "\n";
	std::cout << "texture.max.rectangle_size: " << gl.GetInt(GL_MAX_RECTANGLE_TEXTURE_SIZE) << "\n";
	std::cout << "depth_texture_samples.max: " << gl.GetInt(GL_MAX_DEPTH_TEXTURE_SAMPLES) << "\n";
	std::cout << "renderbuffer.max.size: " << gl.GetInt(GL_MAX_RENDERBUFFER_SIZE) << "\n";
	std::cout << "cube_map.texture.max.size: " << gl.GetInt(GL_MAX_CUBE_MAP_TEXTURE_SIZE) << "\n";
	if (GLEW_EXT_texture_filter_anisotropic)
		std::cout << "texture.anisotropy.max: " << gl.GetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT) << "\n";
	else
		std::cout << "texture.anisotropy.max: not supported\n";
	std::cout << "\n";
}

static const char *gl_debugSourceAttr(GLenum source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API_ARB:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		return "WINDOW_SYSTEM";
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		return "SHADER_COMPILER";
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		return "THIRD_PARTY";
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		return "APPLICATION";
	case GL_DEBUG_SOURCE_OTHER_ARB:
		return "OTHER";
	default:
		return "Unknown source";
	}
}

static const char *gl_debugTypeAttr(GLenum type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR_ARB:
		return "ERROR";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		return "DEPRECATED_BEHAVIOR";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		return "UNDEFINED_BEHAVIOR";
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		return "PORTABILITY";
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		return "PERFORMANCE";
	case GL_DEBUG_TYPE_OTHER_ARB:
		return "OTHER";
	default:
		return "Unknown type";
	}
}

static const char *gl_debugServerityAttr(GLenum severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		return "HIGH";
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		return "MEDIUM";
	case GL_DEBUG_SEVERITY_LOW_ARB:
		return "LOW";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "NOTIFY";
	default:
		return "Unknown severity";
	}
}

static void GLAPIENTRY gl_debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		std::cerr << gl_debugSourceAttr(source) << ", " << gl_debugTypeAttr(type) << ", " << gl_debugServerityAttr(severity) << ", "
				  << message;
	}
	else
	{
		std::cout << gl_debugSourceAttr(source) << ", " << gl_debugTypeAttr(type) << ", " << gl_debugServerityAttr(severity) << ", "
				  << message;
	}
}

static void gl_bindDebugCallback()
{
	if (GLEW_ARB_debug_output)
		glDebugMessageCallbackARB(&gl_debugCallback, NULL);
	else
		std::cerr << "Failed to enable ARB_debug_output";
}