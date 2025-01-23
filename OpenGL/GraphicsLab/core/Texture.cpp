#include "Texture.h"
#include <glad/gl.h>
#include "Utils.h"
#include "Image.h"

#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용


namespace gl
{
    void Texture::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_texID);
    }

    std::unique_ptr<Texture> Texture::Create(Image* image)
    {
        assert(image && image->GetData());
        std::unique_ptr<Texture> tex;
        GLuint texID{ 0 };

        int w = image->GetWidth();
        int h = image->GetHeight();
        int ch = image->GetChannels();
        const uint8_t* data = image->GetData();

        GLenum dataFormat = GL_RGB;
        switch (ch)
        {
        case 1: dataFormat = GL_RED; break;
        case 2: dataFormat = GL_RG; break;
        case 3: dataFormat = GL_RGB; break;
        case 4: dataFormat = GL_RGBA; break;
        }

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        // 텍스처 필터링 및 반복 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        tex = std::make_unique<Texture>();
        tex->m_width = w;
        tex->m_height = h;
        tex->m_depth = 1;
        tex->m_texID = texID;

        return tex;
    }
}
