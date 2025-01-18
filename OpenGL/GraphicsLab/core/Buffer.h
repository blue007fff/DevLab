#pragma once

#include "Common.h"

namespace gl
{
    class Buffer
    {
	public:
		enum class Type { ARRAY, ELEMENT_ARRAY };
		enum class Usage { STATIC, DYNAMIC, STREAM };

        static std::unique_ptr<Buffer> CreateWithData(
            Type bufferType, Usage usage,
            const void* data, size_t stride, size_t count);
        ~Buffer();

        uint32_t Get() const { return m_buffer; }
        size_t GetStride() const { return m_stride; }
        size_t GetCount() const { return m_count; }
        void Bind() const;

    private:
        Buffer() {}
        bool Init(Type bufferType, Usage usage,
            const void* data, size_t stride, size_t count);

        uint32_t m_buffer{ 0 };
        Type m_bufferType{ 0 };
        Usage m_usage{ 0 };
        size_t m_stride{ 0 };
        size_t m_count{ 0 };
    };

}
