#pragma once

#include "Common.h"

class Image;
namespace gl
{
	class Texture
	{
	public:
		static std::unique_ptr<Texture> Create(Image* image);
		void Bind(uint32_t slot) const;

	public:
		uint32_t m_texID{ 0 };
		uint32_t m_width{ 0 };
		uint32_t m_height{ 0 };
		uint32_t m_depth{ 0 };
	};
}
