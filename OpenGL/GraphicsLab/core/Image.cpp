#include "Image.h"
#include "Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h> // 파일 싱크 사용


std::unique_ptr<Image> Image::Create(std::string_view filepath, bool flipVertical /*= true*/) 
{
    auto image = std::unique_ptr<Image>(new Image());
    if (!image->LoadWithStb(filepath, flipVertical))
        return nullptr;
    return std::move(image);
}

std::unique_ptr<Image> Image::Create(int width, int height, int channelCount) 
{
    auto image = std::unique_ptr<Image>(new Image());
    if (!image->Allocate(width, height, channelCount))
        return nullptr;
    return std::move(image);
}

std::unique_ptr<Image> Image::CreateSingleColorImage(
	int width, int height, const glm::vec4& color)
{
	glm::vec4 clamped = glm::clamp(color * 255.0f, 0.0f, 255.0f);
	uint8_t rgba[4] = {
		(uint8_t)clamped.r,
		(uint8_t)clamped.g,
		(uint8_t)clamped.b,
		(uint8_t)clamped.a,
	};
	auto image = Create(width, height, 4);
	for (int i = 0; i < width * height; i++)
	{
		memcpy(image->m_data.data() + 4 * i, rgba, 4);
	}
	return std::move(image);
}

bool Image::Allocate(int width, int height, int channelCount) 
{   
	size_t rowSize = width * channelCount;
	size_t alignedRowSize = CompAlignedRowSize(rowSize, 4);
	size_t totalSize = alignedRowSize * height;

	m_width = width;
	m_height = height;
	m_channels = channelCount;
	m_data.resize(alignedRowSize * height);
    return !m_data.empty();
}

void Image::SetCheckImage(int gridX, int gridY)
{
    for (int j = 0; j < m_height; j++) {
        for (int i = 0; i < m_width; i++) {
            int pos = (j * m_width + i) * m_channels;
            bool even = ((i / gridX) + (j / gridY)) % 2 == 0;
            uint8_t value = even ? 255 : 0;
            for (int k = 0; k < m_channels; k++)
                m_data[pos + k] = value;
            if (m_channels > 3)
                m_data[3] = 255;
        }
    }
}

bool Image::LoadWithStb(std::string_view filepath, bool flipVertical)
{
	stbi_set_flip_vertically_on_load(flipVertical);
	// 1 byte alignment
	int w, h, channels;
	uint8_t* data = stbi_load(filepath.data(), &w, &h, &channels, 0);
	if (!data)
	{
		SPDLOG_ERROR("failed to load image: {}", filepath);
		return false;
	}

	assert(m_data.empty());
	size_t rowSize = w * channels;
	size_t alignedRowSize = CompAlignedRowSize(rowSize, 4);
	size_t totalSize = alignedRowSize * h;

	// 새로 얼라인된 메모리를 생성
	std::vector<uint8_t> imageData(totalSize, 0);
	for (size_t y = 0; y < h; ++y)
	{
		std::memcpy(imageData.data() + (y * alignedRowSize), data + (y * rowSize), rowSize);
	}
	STBI_FREE(data);
	data = nullptr;

	m_width = w;
	m_height = h;
	m_channels = channels;
	m_data = std::move(imageData);

	return true;
}

#if 0
namespace RawImageGenerator
{
	class Color {
	public:
		Color() = default;
		Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
			: r{ _r }, g{ _g }, b{ _b }, a{ _a } {};
		uint8_t r = 0, g = 0, b = 0, a = 255;
	};

	class ImagePattern
	{
	public:
		ImagePattern(int w, int h) : m_w(w), m_h(h) {}
		virtual ~ImagePattern() = default;
		int GetWidth() const { return m_w; }
		int GetHeight() const { return m_w; }
		Color operator()(int x, int y) const { return GetColor(x, y); }
		virtual Color GetColor(int x, int y) const = 0;
	private:
		int m_w, m_h;
	};

	class CirclePattern : public ImagePattern
	{
	public:
		CirclePattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth(), height = GetHeight();

			/*// 픽셀 좌표 기준 계산
			int centerX = width / 2, centerY = height / 2;
			float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
			float len = std::min(width/2, height/2);
			if (static_cast<int>(distance) < static_cast<int>(len))*/

			// 공간 좌표 기준 계산.
			float xx = x + 0.5;
			float yy = y + 0.5;
			float centerX = width / 2.0, centerY = height / 2.0;
			float distance = sqrt(pow(xx - centerX, 2) + pow(yy - centerY, 2));
			float len = std::min(width / 2.0, height / 2.0);

			bool check = static_cast<int>(distance) < static_cast<int>(len);
			return check ? m_color1 : m_color2;
		}
		int m_patternSize = 10;
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 255, 0, 255 };
	};

	class CheckBoardPattern : public ImagePattern
	{
	public:
		CheckBoardPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			return (x / m_patternSize + y / m_patternSize) % 2 == 0
				? m_color1 : m_color2;
		}

		int m_patternSize = 10;
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 255, 0, 255 };
	};

	class StripPattern : public ImagePattern
	{
	public:
		StripPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			const int& pos = m_vertical ? y : x;
			bool check = (pos / m_patternSize) % 2 == 0;
			return check ? m_color1 : m_color2;
		}

		bool m_vertical = false;
		unsigned short m_patternSize = 10;
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 255, 0, 255 };
	};

	class GradiantPattern : public ImagePattern
	{
	public:
		GradiantPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			float len = m_vertical ? GetHeight() : GetWidth();
			len -= 1.0f;
			float pos = m_vertical ? y : x;
			//pos += 0.5;
			float t = pos / len;

			Color color;
			color.r = (1.0f - t) * m_color1.r + t * m_color2.r;
			color.g = (1.0f - t) * m_color1.g + t * m_color2.g;
			color.b = (1.0f - t) * m_color1.b + t * m_color2.b;
			return color;
		}

		bool m_vertical = false;
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 0, 255, 255 };
	};

	class DiamondPattern : public ImagePattern
	{
	public:
		DiamondPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			return (abs(x - y) / m_patternSize) % 2 == 0 ? m_color1 : m_color2;
		}
		unsigned short m_patternSize = 10;
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 0, 255, 255 };
	};

	class RadialGradiantPattern : public ImagePattern
	{
	public:
		RadialGradiantPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int centerX = GetWidth() / 2, centerY = GetHeight() / 2;
			float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
			float t = distance / (sqrt(centerX * centerX + centerY * centerY));
			Color color;
			color.r = (1.0f - t) * m_color1.r + t * m_color2.r;
			color.g = (1.0f - t) * m_color1.g + t * m_color2.g;
			color.b = (1.0f - t) * m_color1.b + t * m_color2.b;
			return color;
		}
		Color m_color1{ 255, 0, 0, 255 };
		Color m_color2{ 0, 0, 255, 255 };
	};

	class DigonalGradientPattern : public ImagePattern
	{
	public:
		DigonalGradientPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			Color color;
			color.r = static_cast<uint8_t>((x + y) * 255 / (width + height));
			color.g = 0;
			color.b = static_cast<uint8_t>(255 - (x + y) * 255 / (width + height));
			return color;
		}
	};

	class GradientSquarePattern : public ImagePattern
	{
	public:
		GradientSquarePattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			Color color;
			color.r = static_cast<uint8_t>((x * 255) / width);
			color.g = 128;
			color.b = static_cast<uint8_t>((y * 255) / height);
			return color;
		}
	};
	class WavePattern : public ImagePattern
	{
	public:
		WavePattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			Color color;
			color.r = 0;
			color.g = 0;
			color.b = static_cast<uint8_t>((sin(x * 0.2) + 1) * 127);
			return color;
		}
	};
	class SpiralPattern : public ImagePattern
	{
	public:
		SpiralPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			int centerX = width / 2, centerY = height / 2;
			float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
			Color color;
			color.r = static_cast<uint8_t>(255 * (1 - distance / (width / 2)));
			color.g = 0;
			color.b = static_cast<uint8_t>(255 * (distance / (width / 2)));
			return color;
		}
	};
	class ExpandingCirclesPattern : public ImagePattern
	{
	public:
		ExpandingCirclesPattern(int w, int h) : ImagePattern(w, h) {};
		virtual Color GetColor(int x, int y) const override
		{
			int width = GetWidth();
			int height = GetHeight();
			int centerX = width / 2, centerY = height / 2;
			float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
			if (static_cast<int>(distance) % 20 == 0)
			{
				return Color{ 255, 0, 0, 255 };
			}
			else
			{
				return Color{ 0, 0, 255, 255 };
			}
		}
	};

	void Generate(const ImagePattern& pattern, std::vector<byte>& pixels)
	{
		int width = pattern.GetWidth();
		int height = pattern.GetHeight();
		pixels.resize(width * height * 3);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				int index = (y * width + x) * 3;
				Color color = pattern(x, y);
				pixels[index + 0] = color.r;
				pixels[index + 1] = color.g;
				pixels[index + 2] = color.b;
			}
		}
	}

	std::vector<uint8_t> ScaleImage(int w, int h, const std::vector<uint8_t>& pixels, int scale)
	{
		std::vector<uint8_t> scaledpixels;
		scaledpixels.resize((size_t)w * h * scale * scale * 3);

		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				int index = (y * w + x) * 3;

				int xx0 = x * scale;
				int yy0 = y * scale;
				int xx1 = (x + 1) * scale;
				int yy1 = (y + 1) * scale;
				for (int yy = yy0; yy < yy1; ++yy)
				{
					for (int xx = xx0; xx < xx1; ++xx)
					{
						int ii = (yy * w * scale + xx) * 3;
						scaledpixels[ii + 0] = pixels[index + 0];
						scaledpixels[ii + 1] = pixels[index + 1];
						scaledpixels[ii + 2] = pixels[index + 2];
					}
				}
			}
		}

		return scaledpixels;
	}
}
#endif
