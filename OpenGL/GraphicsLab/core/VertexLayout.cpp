#include "VertexLayout.h"
#include <glad/gl.h>

namespace gl
{
	void VertexLayout::Enable()
	{
		for (uint32_t i = 0; i < 16u; ++i)
		{
			const auto& attrib = m_attributes[i];
			if (attrib.m_enable)
			{
				glVertexAttribPointer(
					i, attrib.m_count, attrib.m_type,
					!attrib.m_normalized ? GL_FALSE : GL_TRUE,
					attrib.m_stride, (void*)attrib.m_offset);
				glEnableVertexAttribArray(i);
			}
			else
			{
				glDisableVertexAttribArray(i);
			}
		}
	}
}