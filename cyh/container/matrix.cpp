#include "matrix.hpp"
#include <cyh/reference.hpp>
namespace cyh::container::details
{
	template<class T>
	class matrix_reader
	{
		matrix<T>* m_matrix;
		std::vector<T*> m_rows;
		void create_map()
		{
			T* ptr = this->m_matrix->raw_pointer();
			auto cols = this->m_matrix->cols();
			auto rows = this->m_matrix->rows();
			auto dep = this->m_matrix->chnn();
			this->m_rows.reserve(static_cast<size_t>(rows));
			for (size_t h = 0; h < rows; ++h)
			{
				this->m_rows.push_back(ptr + (cols * h * dep));
			}
		}
	public:
		matrix_reader(matrix<T>& out_ref)
		{
			this->m_matrix = &out_ref;
			this->create_map();
		}
		inline T* operator[](size_t index) const
		{
			if (index >= m_matrix->rows())
				throw cyh::exception::out_of_range_exception().src(__POSITION__);
			return this->m_rows[index];
		}
	};

	// Sampling function using bilinear interpolation
	template<class T>
	inline double sampleBilinear(size_t col, size_t row, size_t dep, const matrix_reader<T>& reader, double x, double y, size_t d)
	{
		// Get the integer parts of x and y
		auto x0 = static_cast<size_t>(std::floor(x));
		auto y0 = static_cast<size_t>(std::floor(y));

		// Get the fractional parts of x and y
		double x_diff = x - x0;
		double y_diff = y - y0;

		// Clamp indices to be within the valid range
		x0 = std::max(size_t{ 0 }, std::min(x0, col - 1));
		y0 = std::max(size_t{ 0 }, std::min(y0, row - 1));

		size_t x1 = std::min(x0 + 1, col - 1);
		size_t y1 = std::min(y0 + 1, row - 1);

		// Get the four neighboring pixels
		double I00 = static_cast<double>(reader[y0][x0 * dep + d]);
		double I01 = static_cast<double>(reader[y1][x0 * dep + d]);
		double I10 = static_cast<double>(reader[y0][x1 * dep + d]);
		double I11 = static_cast<double>(reader[y1][x1 * dep + d]);

		// Perform bilinear interpolation
		double I = (I00 * (1 - x_diff) * (1 - y_diff)) +
			(I10 * x_diff * (1 - y_diff)) +
			(I01 * (1 - x_diff) * y_diff) +
			(I11 * x_diff * y_diff);

		return I;
	}

	struct matrix_resize_info
	{
		size_t w_in;
		size_t h_in;
		size_t w_out;
		size_t h_out;
		size_t d_in;
		size_t d_out;
		double scale_x;
		double scale_y;
		matrix_resize_info(matrix_base* pdst, const matrix_base* psrc)
		{
			this->w_in = psrc->cols();
			this->h_in = psrc->rows();
			this->d_in = psrc->chnn();
			this->w_out = pdst->cols();
			this->h_out = pdst->rows();
			this->d_out = pdst->chnn();
			this->scale_x = static_cast<double>(w_in) / static_cast<double>(w_out);
			this->scale_y = static_cast<double>(h_in) / static_cast<double>(h_out);
		}
	};

	template<class T>
	inline void resize_single_channel(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		for (size_t height = 0; height < rzInfo.h_out; ++height)
		{
			auto r_in_d = static_cast<double>(height) * rzInfo.scale_y;
			auto r_in = static_cast<size_t>(r_in_d);
			if (r_in >= rzInfo.h_in) r_in = rzInfo.h_in - 1;

			for (size_t width = 0; width < rzInfo.w_out; ++width)
			{
				auto c_in_d = static_cast<double>(width) * rzInfo.scale_x;
				auto c_in = static_cast<size_t>(c_in_d);
				if (c_in >= rzInfo.w_in) c_in = rzInfo.w_in - 1;
				dst[height][width] = src[r_in][c_in];
			}
		}
	}

	template<class T>
	inline void resize_multi_channel(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		for (size_t height = 0; height < rzInfo.h_out; ++height)
		{
			auto r_in_d = static_cast<double>(height) * rzInfo.scale_y;
			auto r_in = static_cast<size_t>(r_in_d);
			if (r_in >= rzInfo.h_in) r_in = rzInfo.h_in - 1;

			auto r_in_beg = src[r_in];
			auto r_out_beg = dst[height];

			for (size_t width = 0; width < rzInfo.w_out; ++width)
			{
				auto c_in_d = static_cast<double>(width) * rzInfo.scale_x;
				auto c_in = static_cast<size_t>(c_in_d);
				if (c_in >= rzInfo.w_in) c_in = rzInfo.w_in - 1;

				auto pbeg_i = r_in_beg + c_in * rzInfo.d_in;
				auto pbeg_o = r_out_beg + width * rzInfo.d_out;

				for (size_t d = 0; d < rzInfo.d_in; ++d)
				{
					pbeg_o[d] = pbeg_i[d];
				}
			}
		}
	}

	template<class T>
	inline void resize_single_channel_with_sampling(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		for (size_t height = 0; height < rzInfo.h_out; ++height)
		{
			auto r_in_d = static_cast<double>(height) * rzInfo.scale_y;
			auto r_in = static_cast<size_t>(r_in_d);
			if (r_in >= rzInfo.h_in) r_in = rzInfo.h_in - 1;

			for (size_t width = 0; width < rzInfo.w_out; ++width)
			{
				auto c_in_d = static_cast<double>(width) * rzInfo.scale_x;
				auto c_in = static_cast<size_t>(c_in_d);
				if (c_in >= rzInfo.w_in) c_in = rzInfo.w_in - 1;

				auto val = sampleBilinear(rzInfo.w_in, rzInfo.h_in, rzInfo.d_in, src, c_in_d, r_in_d, 1);
				dst[height][width] = static_cast<T>(std::round(val));
			}
		}
	}

	template<class T>
	inline void resize_multi_channel_with_sampling(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		for (size_t height = 0; height < rzInfo.h_out; ++height)
		{
			auto r_in_d = static_cast<double>(height) * rzInfo.scale_y;
			auto r_in = static_cast<size_t>(r_in_d);
			if (r_in >= rzInfo.h_in) r_in = rzInfo.h_in - 1;

			auto r_in_beg = src[r_in];
			auto r_out_beg = dst[height];

			for (size_t width = 0; width < rzInfo.w_out; ++width)
			{
				auto c_in_d = static_cast<double>(width) * rzInfo.scale_x;
				auto c_in = static_cast<size_t>(c_in_d);
				if (c_in >= rzInfo.w_in) c_in = rzInfo.w_in - 1;

				auto pbeg_i = r_in_beg + c_in * rzInfo.d_in;
				auto pbeg_o = r_out_beg + width * rzInfo.d_out;

				for (size_t d = 0; d < rzInfo.d_in; ++d)
				{
					auto val = sampleBilinear(rzInfo.w_in, rzInfo.h_in, rzInfo.d_in, src, c_in_d, r_in_d, d);
					pbeg_o[d] = static_cast<T>(std::round(val));
				}
			}
		}
	}

	template<class T>
	inline void resize_mat_without_sampling(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		if (rzInfo.d_in == 1) {
			resize_single_channel(rzInfo, dst, src);
		} else {
			resize_multi_channel(rzInfo, dst, src);
		}
	}

	template<class T>
	inline void resize_mat_with_sampling(const matrix_resize_info& rzInfo, matrix_reader<T>& dst, const matrix_reader<T>& src)
	{
		if (rzInfo.d_in == 1) {
			resize_single_channel_with_sampling(rzInfo, dst, src);
		} else {
			resize_multi_channel_with_sampling(rzInfo, dst, src);
		}
	}

	template<class T>
	void resize_mat(matrix<T>& out, const matrix<T>& in)
	{
		matrix<T> _in = in;
		if (!in.is_full_range())
		{
			_in.to_uniqe();
		}
		matrix_reader<T> src{ _in };
		matrix_reader<T> dst{ out };

		matrix_resize_info rzInfo { &out, &in };

		resize_mat_without_sampling(rzInfo, dst, src);
	}

	template<class T>
	void resize_mat_with_sampling(matrix<T>& out, const matrix<T>& in)
	{
		matrix<T> _in = in;
		if (!in.is_full_range())
		{
			_in.to_uniqe();
		}
		matrix_reader<T> src{ _in };
		matrix_reader<T> dst{ out };

		matrix_resize_info rzInfo{ &out, &in };

		resize_mat_with_sampling(rzInfo, dst, src);
	}

	template void resize_mat<uint8>(matrix<uint8>& out, const matrix<uint8>& in);
	template void resize_mat<uint16>(matrix<uint16>& out, const matrix<uint16>& in);
	template void resize_mat<uint32>(matrix<uint32>& out, const matrix<uint32>& in);
	template void resize_mat<uint64>(matrix<uint64>& out, const matrix<uint64>& in);
	template void resize_mat<float>(matrix<float>& out, const matrix<float>& in);
	template void resize_mat<double>(matrix<double>& out, const matrix<double>& in);

	template void resize_mat_with_sampling<uint8>(matrix<uint8>& out, const matrix<uint8>& in);
	template void resize_mat_with_sampling<uint16>(matrix<uint16>& out, const matrix<uint16>& in);
	template void resize_mat_with_sampling<uint32>(matrix<uint32>& out, const matrix<uint32>& in);
	template void resize_mat_with_sampling<uint64>(matrix<uint64>& out, const matrix<uint64>& in);
	template void resize_mat_with_sampling<float>(matrix<float>& out, const matrix<float>& in);
	template void resize_mat_with_sampling<double>(matrix<double>& out, const matrix<double>& in);
};
