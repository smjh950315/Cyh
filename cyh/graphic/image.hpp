#pragma once
#include "graphic_.hpp"
#include <filesystem>
namespace cyh::graphic
{
	class image : public container::matrix<uint8>
	{
	public:
		enum COLOR {
			AUTO,
			RGB = 3,
			RGBA = 4,
		};
		enum class FORMAT {
			BMP,
			JPG,
			PNG,
			TGA,
			WEBP,
			AVIF,
			HEIF
		};
		image();
		image(int width, int height, int channels);
		image(const image& img);
		image(image&& img) noexcept;
		image(const container::matrix<uint8>& mat);
		image(container::matrix<uint8>&& mat) noexcept;
		image& operator=(const image& img);
		image& operator=(image&& img) noexcept;
		image& operator=(const container::matrix<uint8>& mat);
		image& operator=(container::matrix<uint8>&& mat) noexcept;
		size_type width() const;
		size_type height() const;
		size_type channels() const;
		void read(const void* data, size_t length, COLOR _colortype = AUTO);
		void read(const std::filesystem::path& _path, COLOR _colortype = AUTO);
		void write(const std::filesystem::path& _path, FORMAT format, int quality) const;
		void write(obstream& _obstream, FORMAT format, int quality) const;
		void resize_fixed_aspect(uint max_width, uint max_height, bool _sampling);
		image to_gray() const;
		matrix<double> get_gray_scale() const;

		static image from_memory(const void* data, size_t length, COLOR _colortype = AUTO);
		static image from_file(const std::filesystem::path& _path, COLOR _colortype = AUTO);
		static bool read_info(image_info& img_info, const void* data, size_t length);
	};

	double get_ssim(const image& lhs, const image& rhs, int64 _width = 0, int64 _height = 0, double l = 1.0, double k1 = .01, double k2 = .03);
};

