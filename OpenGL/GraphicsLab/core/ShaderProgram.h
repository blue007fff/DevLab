#pragma once

#include "Common.h"
#include <glad/gl.h>

namespace gl
{
	class ShaderProgram
	{
	public:
        static std::unique_ptr<ShaderProgram> Create(
            std::string_view vsCode, std::string_view fsCode);

        static std::unique_ptr<ShaderProgram> CreateFromFile(
            std::string_view vsPath, std::string_view fsPath);

        ~ShaderProgram();
        uint32_t Get() const;
        void Use() const;

        template<typename T>
        void SetUniform(std::string_view name, T value) const {
            auto loc = glGetUniformLocation(m_programID, name.data());
            SetUniform(loc, value);
        };

        void SetUniform(int32_t loc, int value) const;
        void SetUniform(int32_t loc, float value) const;
        void SetUniform(int32_t loc, const glm::vec2& value) const;
        void SetUniform(int32_t loc, const glm::vec3& value) const;
        void SetUniform(int32_t loc, const glm::vec4& value) const;
        void SetUniform(int32_t loc, const glm::mat4& value) const;

    private:
        ShaderProgram() {}
        uint32_t m_programID{ 0 };
	};
}
