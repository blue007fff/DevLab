#include "Buffer.h"
#include <glad/gl.h>
#include <windows.h>

namespace gl
{
    uint32_t TO_GLFormat(Buffer::Type bufferType)
    {
        switch (bufferType)
        {
        case Buffer::Type::ARRAY: return GL_ARRAY_BUFFER;
        case Buffer::Type::ELEMENT_ARRAY: return GL_ELEMENT_ARRAY_BUFFER;
        }
        assert(false);
        return GL_ARRAY_BUFFER;
    }
    uint32_t TO_GLFormat(Buffer::Usage bufferUsage)
    {
        switch (bufferUsage)
        {
        case Buffer::Usage::STATIC: return GL_STATIC_DRAW;
        case Buffer::Usage::DYNAMIC: return GL_DYNAMIC_DRAW;
        case Buffer::Usage::STREAM: return GL_STREAM_DRAW;
        }
        assert(false);
        return GL_STATIC_DRAW;
    }

    std::unique_ptr<Buffer> Buffer::CreateWithData(
        Type bufferType, Usage usage,
        const void* data, size_t stride, size_t count)
    {
        auto buffer = std::unique_ptr<Buffer>(new Buffer());
        //auto buffer = std::make_unique<Buffer>();
        if (!buffer->Init(bufferType, usage, data, stride, count))
            return nullptr;
        return std::move(buffer);
    }

    Buffer::~Buffer()
    {
        if (m_buffer)
        {
            glDeleteBuffers(1, &m_buffer);
        }
    }

    void Buffer::Bind() const
    {
        glBindBuffer(TO_GLFormat(m_bufferType), m_buffer);
    }

    bool Buffer::Init(
        Type bufferType, Usage usage,
        const void* data, size_t stride, size_t count)
    {
        m_bufferType = bufferType;
        m_usage = usage;
        m_stride = stride;
        m_count = count;
        glGenBuffers(1, &m_buffer);
        Bind();
        glBufferData(TO_GLFormat(m_bufferType), stride * count, data,
            TO_GLFormat(m_usage));
        return true;
    }
}