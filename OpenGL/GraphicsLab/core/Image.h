#pragma once

#include "Common.h"

class Image
{   
private: 
    Image() = default;

public:
    static std::unique_ptr<Image> Create(std::string_view filepath, bool flipVertical = true);
    static std::unique_ptr<Image> Create(int width, int height, int channelCount = 4);
    static std::unique_ptr<Image> CreateSingleColorImage(int width, int height, const glm::vec4& color);   
    static size_t CompAlignedRowSize(size_t rowSize, size_t alignment) {
        size_t alignedRowSize = (rowSize + (alignment - 1)) & ~(alignment - 1);
        return alignedRowSize;
    }

    const uint8_t* GetData() const { return m_data.data(); }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }

    void SetCheckImage(int gridX, int gridY);

private:
    bool LoadWithStb(std::string_view filepath, bool flipVertical);
    bool Allocate(int width, int height, int channelCount);

    int m_width{ 0 };
    int m_height{ 0 };
    int m_channels{ 0 };
    std::vector<uint8_t> m_data;
};