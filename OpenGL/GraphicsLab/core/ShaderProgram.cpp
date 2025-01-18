#include "ShaderProgram.h"
#include <glad/gl.h>
#include "Utils.h"

namespace gl
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

    std::unique_ptr<ShaderProgram> ShaderProgram::Create(
        std::string_view vsCode, std::string_view fsCode)
	{
		// 셰이더 컴파일
		const char* vs = !vsCode.empty() ? vsCode.data() : nullptr;
		const char* fs = !fsCode.empty() ? fsCode.data() : nullptr;

		GLuint vsID = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vsID, 1, &vs, nullptr);
		glCompileShader(vsID);
		CheckShaderCompileErrors(vsID);

		GLuint fsID = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fsID, 1, &fs, nullptr);
		glCompileShader(fsID);
		CheckShaderCompileErrors(fsID);

        // 셰이더 프로그램 생성 및 링크
        GLuint progID = glCreateProgram();
        glAttachShader(progID, vsID);
        glAttachShader(progID, fsID);
        glLinkProgram(progID);
        CheckShaderCompileErrors(progID);

        // 셰이더 삭제
        glDeleteShader(vsID);
        glDeleteShader(fsID);

        auto program = std::unique_ptr<ShaderProgram>(new ShaderProgram);
        program->m_programID = progID;
        return program;
    }

    ShaderProgram::~ShaderProgram()
    {
        if (m_programID)
        {
            glDeleteProgram(m_programID);
        }
    }

    uint32_t ShaderProgram::Get() const
    { 
        return m_programID;
    }

    void ShaderProgram::Use() const
    {
        glUseProgram(m_programID);
    }
    
    void ShaderProgram::SetUniform(int32_t loc, int value) const
    {
        glUniform1i(loc, value);
    }
    void ShaderProgram::SetUniform(int32_t loc, float value) const
    {
        glUniform1f(loc, value);
    }
    void ShaderProgram::SetUniform(int32_t loc, const glm::vec2& value) const
    {
        glUniform2fv(loc, 1, glm::value_ptr(value));
    }
    void ShaderProgram::SetUniform(int32_t loc, const glm::vec3& value) const
    {
        glUniform3fv(loc, 1, glm::value_ptr(value));

    }
    void ShaderProgram::SetUniform(int32_t loc, const glm::vec4& value) const
    {
        glUniform4fv(loc, 1, glm::value_ptr(value));
    }
    void ShaderProgram::SetUniform(int32_t loc, const glm::mat4& value) const
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }
}


