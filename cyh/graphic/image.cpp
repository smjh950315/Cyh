#include "image.hpp"
#include "stb_callback.hpp"
#include <cmath>      // sqrt, round, exp
using cyh::container::matrix;
namespace cyh::graphic
{
	static constexpr double rgb_2_gray[3] = { 0.2989, 0.5870, 0.1140 };

	template<class Dst, class Src>
	void get_gray_matrix_t(matrix<Dst>* dst, const matrix<Src>* src) {
		if (src->empty()) return;
		if (src->chnn() == 1) {
			if (typeid(Dst) == typeid(Src)) {
				(*dst) = *((matrix<Dst>*)src);
			} else {
				(*dst) = src->template convert_type<Dst>();
			}
			return;
		}
		auto chs = src->chnn();
		if (chs == 3 || chs == 4) {

			matrix<double> temp(src->cols(), src->rows(), 1);

			for (size_t ch = 0; ch < 3; ++ch) {
				auto currentCh = (*src)[ch];
				auto it_beg = currentCh.begin();
				auto it_end = currentCh.end();
				double* data_temp = temp.raw_pointer();
				while (it_beg != it_end)
				{
					*data_temp += static_cast<double>(*it_beg) * rgb_2_gray[ch];
					it_beg++;
					data_temp++;
				}
			}

			if (typeid(Dst) == typeid(double)) {
				(*dst) = (*((matrix<Dst>*)(&temp)));
			} else {
				(*dst) = temp.convert_type<Dst>();
			}
		}
		return;
	}

	image::image() = default;
	image::image(int width, int height, int channels) : container::matrix<uint8>(width, height, channels) {
	}
	image::image(const image& img) : container::matrix<uint8>(img) {
	}
	image::image(image&& img) noexcept : container::matrix<uint8>(std::move(img)) {
	}
	image::image(const container::matrix<uint8>& mat) {
		container::matrix<uint8>& bThis = *this;
		bThis = mat;
	}
	image::image(container::matrix<uint8>&& mat) noexcept {
		container::matrix<uint8>& bThis = *this;
		bThis = std::move(mat);
	}
	image& image::operator=(const image& img) {
		container::matrix<uint8>::operator=(img);
		return *this;
	}
	image& image::operator=(image&& img) noexcept {
		container::matrix<uint8>::operator=(std::move(img));
		return *this;
	}
	image& image::operator=(const container::matrix<uint8>& mat) {
		container::matrix<uint8>::operator=(mat);
		return *this;
	}
	image& image::operator=(container::matrix<uint8>&& mat) noexcept {
		container::matrix<uint8>::operator=(std::move(mat));
		return *this;
	}
	matrix<uint8>::size_type image::width() const {
		return this->cols();
	}
	matrix<uint8>::size_type image::height() const {
		return this->rows();
	}
	matrix<uint8>::size_type image::channels() const {
		return this->chnn();
	}
	void image::read(const void* data, size_t length, COLOR _colortype) {
		binary_data binary;
		binary.data = (void*)data;
		binary.length = static_cast<decltype(std::declval<binary_data>().length)>(length);
		image_data img;
		if (stb_callback::load_image(&img, &binary, _colortype))
		{
			this->set_data_by_move(img.width, img.height, img.channels, (cyh::uint8*)img.data);
		}
	}
	void image::read(const std::filesystem::path& _path, COLOR _colortype) {
		image_data img;
		auto u8path = _path.u8string();
		if (stb_callback::load_image_file(&img, (const char*)u8path.c_str(), _colortype))
		{
			this->set_data_by_move(img.width, img.height, img.channels, (cyh::uint8*)img.data);
		}
	}
	void image::write(const std::filesystem::path& _path, FORMAT format, int quality) const {
		image_data img{};
		using imdata_size_type = decltype(std::declval<image_data>().width);
		img.width = static_cast<imdata_size_type>(this->cols());
		img.height = static_cast<imdata_size_type>(this->rows());
		img.channels = static_cast<imdata_size_type>(this->chnn());
		matrix<uint8> temp = *this;
		if (!this->is_full_range())
		{
			temp.to_uniqe();
		}
		img.data = temp.raw_pointer();
		auto u8path = _path.u8string();
		stb_callback::write_image_file(&img, (const char*)u8path.c_str(), (int)format, quality);
	}
	void image::write(obstream& stream, FORMAT format, int quality) const {
		image_data img{};
		using imdata_size_type = decltype(std::declval<image_data>().width);
		img.width = static_cast<imdata_size_type>(this->cols());
		img.height = static_cast<imdata_size_type>(this->rows());
		img.channels = static_cast<imdata_size_type>(this->chnn());
		matrix<uint8> temp = *this;
		if (!this->is_full_range())
		{
			temp.to_uniqe();
		}
		img.data = temp.raw_pointer();
		stb_callback::write_image_to_stream(stream, &img, (int)format, quality);
	}
	void image::resize_fixed_aspect(uint max_width, uint max_height, bool _sampling) {
		if (this->width() == 0 || this->height() == 0)
			return;
		if (max_width == 0 && max_height == 0)
			return;
		auto rateToMaxWidth = static_cast<double>(max_width) / static_cast<double>(this->width());
		auto rateToMaxHeight = static_cast<double>(max_height) / static_cast<double>(this->height());
		double resizeRatio{};
		if (max_width == 0 || max_height == 0) {
			if (max_width == 0) {
				resizeRatio = rateToMaxHeight;
			} else {
				resizeRatio = rateToMaxWidth;
			}
		} else {
			resizeRatio = std::min(rateToMaxWidth, rateToMaxHeight);
		}
		auto newWidth = static_cast<matrix<uint8>::size_type>(resizeRatio * static_cast<double>(this->width()));
		auto newHeight = static_cast<matrix<uint8>::size_type>(resizeRatio * static_cast<double>(this->height()));
		this->resize(newWidth, newHeight, this->chnn(), true, _sampling);
	}
	image image::to_gray() const {
		image result;
		get_gray_matrix_t(&result, this);
		return result;
	}
	container::matrix<double> image::get_gray_scale() const {
		matrix<double> result;
		get_gray_matrix_t(&result, this);
		result /= 255.0;
		return result;
	}

	image image::from_memory(const void* data, size_t length, COLOR _colortype) {
		image result;
		result.read(data, length, _colortype);
		return result;
	}
	image image::from_file(const std::filesystem::path& _path, COLOR _colortype) {
		image result;
		result.read(_path, _colortype);
		return result;
	}
	bool image::read_info(image_info& img_info, const void* data, size_t length) {
		binary_data temp_data{};
		temp_data.data = (void*)data;
		temp_data.length = static_cast<int>(length);
		return stb_callback::read_image_info(&img_info, &temp_data);
	}
};
#include <cmath>      // sqrt, round, exp
#include <algorithm>  // max, min
namespace cyh::graphic {
	template<class T>
	struct single_channel_matrix_operator {
		int64 width, height;
		std::vector<T*> row_begins;

		T** begin() { return this->row_begins.data(); }
		T** end() { return this->row_begins.data() + this->row_begins.size(); }
		single_channel_matrix_operator() : width(0), height(0) {}
		single_channel_matrix_operator(matrix<T>& mat) : width(mat.cols()), height(mat.rows()) {
			this->row_begins.resize(this->height);
			auto ptr = mat.raw_pointer();
			for (int64 i = 0; i < this->height; ++i) {
				this->row_begins[i] = ptr;
				ptr += this->width;
			}
		}
		single_channel_matrix_operator(const matrix<T>* pmat) : width(pmat->cols()), height(pmat->rows()) {
			this->row_begins.resize(this->height);
			auto ptr = pmat->raw_pointer();
			for (int64 i = 0; i < this->height; ++i) {
				this->row_begins[i] = ptr;
				ptr += this->width;
			}
		}
		T* operator[](int64 i) const { return this->row_begins[i]; }
	};
	template<class T>
	struct const_single_channel_matrix_operator {
		int64 width, height;
		std::vector<T*> row_begins;

		T** begin() { return this->row_begins.data(); }
		T** end() { return this->row_begins.data() + this->row_begins.size(); }
		const_single_channel_matrix_operator() : width(0), height(0) {}
		const_single_channel_matrix_operator(const matrix<T>* pmat) : width(pmat->cols()), height(pmat->rows()) {
			this->row_begins.resize(this->height);
			auto ptr = pmat->raw_pointer();
			for (int64 i = 0; i < this->height; ++i) {
				this->row_begins[i] = ptr;
				ptr += this->width;
			}
		}
		const T* operator[](int64 i) const { return this->row_begins[i]; }
	};

	static inline int64 _reflect(int64 val, int64 max) {
		if (val < 0) val = (-val) - 1;
		if (val >= max) val = 2 * max - val - 1;
		return val;
	}
	static inline void apply_filter(matrix<double>& _result, const matrix<double>& _signal, const matrix<double>& _filter) {
		int64 signalW = _signal.cols(), signalH = _signal.rows();
		int64 filterW = _filter.cols(), filterH = _filter.rows();

		int64 resultW = signalW - filterW + 1, resultH = signalH - filterH + 1;
		_result.resize(resultW, resultH, 1);

		single_channel_matrix_operator opr(_result);
		const_single_channel_matrix_operator ops(&_signal);
		const_single_channel_matrix_operator opf(&_filter);

		// loop for result
		for (int64 y = 0; y < resultH; ++y) {
			for (int64 x = 0; x < resultW; ++x) {
				double _sum = .0;

				// loop for filter
				for (int64 fy = 0; fy < filterH; ++fy) {
					for (int64 fx = 0; fx < filterW; ++fx) {
						int64 sx = x + fx;
						int64 sy = y + fy;

						_sum += ops[sy][sx] * opf[fy][fx];
					}
				}

				opr[y][x] = _sum;
			}
		}
	}
	static inline void sub_simple(matrix<double>& _result, const matrix<double>& _gray_scale, int64 _size) {
		int64 oriW = _gray_scale.cols(), oriH = _gray_scale.rows();
		int64 w = oriW / _size, h = oriH / _size;
		double scale = 1.0 / static_cast<double>(_size * _size);
		_result.resize(w, h, 1);

		int64 fa = -_size / 2, fb = _size / 2;
		if ((_size & 1) == 0) ++fa;

		single_channel_matrix_operator op(_result);
		const_single_channel_matrix_operator opImg(&_gray_scale);

		// loop over dest
		for (int64 y = 0; y < h; ++y) {
			for (int64 x = 0; x < w; ++x) {
				double _sum = .0;

				// loop over size
				for (int64 fy = fa; fy <= fb; ++fy) {
					for (int64 fx = fa; fx <= fb; ++fx) {

						// symmetric across border, the edge pixel is repeated
						auto xx = _reflect(fx + x * _size, oriW);
						auto yy = _reflect(fy + y * _size, oriH);
						_sum += opImg[yy][xx];
					}
				}

				op[y][x] = _sum * scale;
			}
		}
	}
	static inline void gaussian(matrix<double>& filter, int _size, double _sigma) {
		filter.resize(_size, _size, 1);

		double s2 = _sigma * _sigma * 2.0;
		int c = _size / 2;

		auto op = single_channel_matrix_operator(filter);

		for (int y = 0; y < _size; ++y) {
			for (int x = 0; x < _size; ++x) {
				double dx = x - c;
				double dy = y - c;
				op[y][x] = std::exp(-(dx * dx + dy * dy) / s2);
			}
		}

		double _sum = filter.sum();
		double _rat = 1.0 / _sum;
		filter *= _rat;
	}
	double get_ssim(const image& lhs, const image& rhs, int64 _width, int64 _height, double l, double k1, double k2) {
		if (lhs.cols() != rhs.cols() || lhs.rows() != rhs.rows()) return -1.0;

		matrix<double> window;
		gaussian(window, 11, 1.5);

		int64 width = _width == 0 ? lhs.cols() : _width;
		int64 height = _height == 0 ? lhs.rows() : _height;

		matrix<double> img1 = lhs.get_gray_scale(), img2 = rhs.get_gray_scale();
		auto f = static_cast<int>(std::max(1.0, std::round(std::min(width, height) / 256.0)));

		if (f > 1) {
			sub_simple(img1, img1.clone(), f);
			sub_simple(img2, img2.clone(), f);
		}

		matrix<double> mu1, mu2, sigmal12, sigma1SQ, sigma2SQ;

		apply_filter(mu1, img1, window);
		apply_filter(mu2, img2, window);

		auto mu1mu2 = mu1 * mu2;
		auto mu1SQ = mu1 * mu1;
		auto mu2SQ = mu2 * mu2;

		apply_filter(sigmal12, img1 * img2, window);
		sigmal12 -= mu1mu2;
		apply_filter(sigma1SQ, img1 * img1, window);
		sigma1SQ -= mu1SQ;
		apply_filter(sigma2SQ, img2 * img2, window);
		sigma2SQ -= mu2SQ;

		double C1 = k1 * l; C1 *= C1;
		double C2 = k2 * l; C2 *= C2;

		double map_total = .0;

		auto mu12Data = mu1mu2.raw_pointer();
		auto sig12Data = sigmal12.raw_pointer();
		auto mu1SQData = mu1SQ.raw_pointer();
		auto mu2SQData = mu2SQ.raw_pointer();
		auto sig1SQData = sigma1SQ.raw_pointer();
		auto sig2SQData = sigma2SQ.raw_pointer();

		auto count = static_cast<size_t>(mu1mu2.elements());
		for (size_t i = 0; i < count; ++i) {
			map_total += (mu12Data[i] * 2 + C1) * (sig12Data[i] * 2 + C2) /
				((mu1SQData[i] + mu2SQData[i] + C1) * (sig1SQData[i] + sig2SQData[i] + C2));
		}

		return map_total / static_cast<double>(count);
	}
};

