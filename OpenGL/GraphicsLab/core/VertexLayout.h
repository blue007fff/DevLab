#pragma once

#include "Common.h"

namespace gl
{
    class VertexLayout
    {
    public:
        struct Attrib
        {
            uint32_t m_type{ 0 }; // float, int,...
            uint32_t m_count{ 0 };
            uint32_t m_stride{ 0 };
            uint32_t m_offset{ 0 };
            bool m_normalized{ false };
            bool m_enable{ false };
        };

        void Reset() {
            for (auto& attrib : m_attributes)
                attrib.m_enable = false;
        }

        void SetAttrib(
            uint32_t index, uint32_t type, uint32_t count,
            uint32_t stride, uint32_t offset,
            bool normalized = false) {
            assert(index < 16);
            auto& attrib = m_attributes[index];
            attrib.m_type = type;
            attrib.m_count = count;
            attrib.m_stride = stride;
            attrib.m_offset = offset;
            attrib.m_normalized = normalized;
            attrib.m_enable = true;
        }

        void Enable();

        Attrib m_attributes[16];
    };

}
