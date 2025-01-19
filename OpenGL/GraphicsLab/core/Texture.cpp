#include "Texture.h"
#include <glad/gl.h>
#include "Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
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

    std::unique_ptr<Texture> Texture::CreateTexture(std::filesystem::path relativePath)
    {
        std::unique_ptr<Texture> tex;
        GLuint texID{ 0 };

        namespace fs = std::filesystem;
        fs::path exeFolderPath = utils::GetExecutablePath().parent_path();
        fs::path filePath = exeFolderPath / relativePath;

        // 이미지 로드
        stbi_set_flip_vertically_on_load(1);

        int width, height, nrChannels;
        uint8_t* data = stbi_load(filePath.string().c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum dataFormat = GL_RGB;
            if (nrChannels == 3) dataFormat = GL_RGB;
            else if (nrChannels == 4) dataFormat = GL_RGBA;

            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);

            // 텍스처 필터링 및 반복 설정
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            tex = std::make_unique<Texture>();
            tex->m_width = width;
            tex->m_height = height;
            tex->m_depth = 1;
            tex->m_texID = texID;
        }
        else
        {
            spdlog::error("Failed to load texture");
        }
        stbi_image_free(data);

        return tex;
    }
}
