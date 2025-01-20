#include "GLError.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>

namespace gl::error
{
    GLenum CheckDriverError()
    {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::string error;
            switch (errorCode)
            {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << error << std::endl;
        }
        return errorCode;
    }

    void APIENTRY DriverDebugOutput(GLenum source,
        GLenum type,
        unsigned int id,
        GLenum severity,
        GLsizei length,
        const char* message,
        const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		auto GetSourcMessage = [&]() {
			switch (source)
			{
			case GL_DEBUG_SOURCE_API:             return "SOURCE:API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "SOURCE:WINDOW_SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SOURCE:SHADER_COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY:     return "SOURCE:THIRD_PARTY";
			case GL_DEBUG_SOURCE_APPLICATION:     return "SOURCE:APPLICATION";
			case GL_DEBUG_SOURCE_OTHER:           return "SOURCE:OTHER";
			};
			return "";
		};

		auto GetTypeMessage = [&]() {
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR:               return "TYPE:ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "TYPE:DEPRECATED_BEHAVIOUR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "TYPE:UNDEFINED_BEHAVIOUR";
			case GL_DEBUG_TYPE_PORTABILITY:         return "TYPE:PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE:         return "TYPE:PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER:              return "TYPE:MARKER";
			case GL_DEBUG_TYPE_PUSH_GROUP:          return "TYPE:PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP:           return "TYPE:POP_GROUP";
			case GL_DEBUG_TYPE_OTHER:               return "TYPE:OTHER";
			};
			return "";
		};

		auto GetSeverityMessage = [&]() {
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:         return "SEVERITY:HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM:       return "SEVERITY:MEDIUM";
			case GL_DEBUG_SEVERITY_LOW:          return "SEVERITY:LOW";
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "SEVERITY:NOTIFICATION";
			};
			return "";
		};

		SPDLOG_ERROR("OpenGL Debug: {} : {} : {} : {} : {}",
			id, message, 
			GetSourcMessage(), GetTypeMessage(), GetSeverityMessage());
    }

	namespace
	{
		void CheckShaderCompileErrors(uint32_t shader)
		{
			int success;
			char infoLog[1024];
			if (glIsShader(shader))
			{
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					spdlog::error("SHADER_COMPILATION_ERROR: {}", infoLog);
				}
			}
			else if (glIsProgram(shader))
			{
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success)
				{
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					spdlog::error("PROGRAM_LINKING_ERROR: {}", infoLog);
				}
			}
		}
	}

	const char* GetFrameBufferMessage(GLenum status)
	{
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:						return "GL_FRAMEBUFFER_COMPLETE";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:					return "GL_FRAMEBUFFER_UNSUPPORTED";
		}
		return "FRAMEBUFFER:UNKNOWN";
	}	
}