#pragma once

#include "Common.h"

namespace gl
{
	class Texture
	{
	public:
		static std::unique_ptr<Texture> CreateTexture(
			std::filesystem::path relativePath);

        static std::unique_ptr<Texture> CreateSingleColorImage(
            int width, int height, const glm::vec4& color);

		void Bind(uint32_t slot) const;

	public:
		uint32_t m_texID{ 0 };
		uint32_t m_width{ 0 };
		uint32_t m_height{ 0 };
		uint32_t m_depth{ 0 };
	};
}
