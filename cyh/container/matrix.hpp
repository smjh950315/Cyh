#pragma once
#include "my_vec.hpp"
#include "size_rect.hpp"
#include <cyh/console.hpp>
//#define PRECOMPILE_NUMERIC_MATRIX
namespace cyh::container {

	template <class T>
	class matrix;

	namespace details {
		template<class T>
		struct mat_iterator {
			using value_type = T;
			using size_type = size_t;
			using iterator = mat_iterator<T>;
			T* data{};
			struct {
				// 單 step 寬度
				size_type step_width{};
				// 每個 row 的 step 數
				size_type row_steps{};
				// 每個 row 的 T 數
				size_type row_strides{};
				// 在每個 step 內的索引清單
				size_type* index_array{};
				// 索引清單內保存的索引數
				size_type index_count{};
			} iter_infos;
			struct {
				// 當前的 step 數
				size_type current_step_in_row{};
				// 當前索引在清單中的索引
				size_type current_index_in_arr{};
				bool is_full_range{};
			} iter_status;
			size_t type_size{};
			constexpr void set_iterator_information(const myvec<size_type, 3>& main, const myvec<size_type, 2>& sub, const myvec<size_type>& indexs_of_sub) {
				if (main.w == sub.w && main.h == sub.h && indexs_of_sub.size() == main.d) {
					this->iter_status.is_full_range = true;
				} else {
					this->iter_status.is_full_range = false;
					if (!indexs_of_sub.size()) {
						throw cyh::exception::out_of_range_exception("invalid matrix index!").src(__POSITION__);
					} else {
						if (this->data) {
							this->data += indexs_of_sub[0];
						}
					}
				}
				this->iter_infos.step_width = main.chnn;
				this->iter_infos.row_steps = sub.cols;
				this->iter_infos.row_strides = main.cols * main.chnn;
				this->iter_infos.index_count = static_cast<size_type>(indexs_of_sub.size());
				this->iter_infos.index_array = (size_type*)indexs_of_sub.data();
			}
			constexpr size_type region_wdith() {
				return this->iter_infos.step_width * this->iter_infos.row_steps;
			}
			constexpr size_type move_region() {
				return this->iter_infos.row_strides - this->region_wdith() + this->iter_infos.step_width;
			}
			constexpr size_type move_next() {
				if (this->iter_status.is_full_range) { return 1; }

				auto& iarr_index = this->iter_status.current_index_in_arr;
				auto& step_index = this->iter_status.current_step_in_row;
				size_type* iarr = this->iter_infos.index_array;
				size_type iarr_count = this->iter_infos.index_count;
				bool shouldMoveRegion{};
				bool shouldMoveStep = (iarr_index + 1 == iarr_count);

				if (shouldMoveStep) {
					shouldMoveRegion = (this->iter_status.current_step_in_row + 1 >= this->iter_infos.row_steps);
					iarr_index = 0;
					if (shouldMoveRegion) {
						this->iter_status.current_step_in_row = 0;
						return this->move_region() - (iarr[iarr_count - 1] - iarr[0]);
					} else {
						this->iter_status.current_step_in_row++;
						return this->iter_infos.step_width - (iarr[iarr_count - 1] - iarr[0]);
					}
				} else {
					iarr_index++;
					return iarr[iarr_index] - iarr[iarr_index - 1];
				}
			}
			constexpr mat_iterator() {}
			constexpr mat_iterator(T* _data, const myvec<size_type, 3>& main, const myvec<size_type, 2>& sub, const myvec<size_type>& indexs_of_sub) : data(_data) {
				this->set_iterator_information(main, sub, indexs_of_sub);
			}
			constexpr const T& operator*() const {
				return *this->data;
			}
			constexpr T& operator*() {
				return *this->data;
			}
			constexpr const T* operator->() const {
				return this->data;
			}
			constexpr T* operator->() {
				return this->data;
			}
			constexpr iterator& operator++() {
				this->data += this->move_next();
				return *this;
			}
			constexpr iterator operator++(int) {
				iterator retIter = *this;
				this->data += this->move_next();
				return retIter;
			}
			constexpr bool operator == (const mat_iterator<T>& rhs) const
			{
				return this->data == rhs.data;
			}
			constexpr bool operator != (const mat_iterator<T>& rhs) const
			{
				return this->data != rhs.data;
			}
		};

		template<class T>
		struct mat_instance {
			using mat_data_holder = myvec<T, 0>;
			using size_type = size_t;

			mat_data_holder m_data{};
			MyVec3<size_type> m_size{};
			~mat_instance(){ }
			constexpr T* begin() const { return data(); }
			constexpr T* end() const { return data() + elements(); }
			constexpr T* begin() { return data(); }
			constexpr T* end() { return data() + elements(); }
			constexpr T* data() const { return (T*)this->m_data.data(); }
			constexpr bool empty() const { return this->elements() == 0; }
			constexpr size_type elements() const { return this->m_size.rows * this->m_size.cols * this->m_size.chnn; }
			constexpr size_type rows() const { return this->m_size.rows; }
			constexpr size_type cols() const { return this->m_size.cols; }
			constexpr size_type chnn() const { return this->m_size.chnn; }
			constexpr size_type stride() const { return chnn() * this->m_size.cols; }
			constexpr void resize(size_type w, size_type h, size_type c) {
				this->m_size.rows = h;
				this->m_size.cols = w;
				this->m_size.chnn = c;
				this->m_data.resize(static_cast<size_t>(this->elements()));
			}
			constexpr void resize(size_type w, size_type h, size_type c, T* ready_buffer) {
				if (!ready_buffer) {
					this->resize(w, h, c);
				} else {
					this->m_size.rows = h;
					this->m_size.cols = w;
					this->m_size.chnn = c;
					this->m_data.replace_internal_buffer_t(ready_buffer, this->elements(), this->elements());
				}
			}
			constexpr void resize(const myvec<size_type, 3>& sz) { this->resize(sz.cols, sz.rows, sz.chnn); }
			constexpr void resize(const myvec<size_type, 3>& sz, T* ready_buffer) { this->resize(sz.cols, sz.rows, sz.chnn, ready_buffer); }
		};
	
		template<class T>
		void resize_mat(matrix<T>& out, const matrix<T>& in);

		template<class T>
		void resize_mat_with_sampling(matrix<T>& out, const matrix<T>& in);
	};

	class matrix_base
	{
	public:
		using size_type = size_t;
		virtual constexpr size_type rows() const = 0;
		virtual constexpr size_type cols() const = 0;
		virtual constexpr size_type chnn() const = 0;
		virtual constexpr bool is_full_range() const = 0;
	};

	template <class T>
	class matrix : public matrix_base {
	public:
		using iterator = cyh::container::details::mat_iterator<T>;
	private:
		ref<cyh::container::details::mat_instance<T>> m_instance{};
		myvec<size_type> m_indexes{};
		cyh::console::console_print_format m_print_configs{};
		Point2D_<size_type> m_beg{};
		Size2D_<size_type> m_size{};
		constexpr size_type get_offset_of_main(const Point2D_<size_type>& subPos) const {
			if (this->m_instance.empty()) return 0;
			auto& mainSize = this->m_instance->m_size;
			auto mainStride = mainSize.cols * mainSize.chnn;
			return mainStride * subPos.y + mainSize.chnn * subPos.x;
		}
	public:
		constexpr T* raw_pointer() const { return empty() ? nullptr : this->m_instance->data(); }
	protected:
		static constexpr void copy_value_to_left(matrix<T>& lhs, const std::initializer_list<size_type>& chsL, const matrix<T>& rhs, const std::initializer_list<size_type>& chsR) {
			if (lhs.elements() == 0 || rhs.elements() == 0) { return; }
			if (chsL.size() != chsR.size()) {
				throw cyh::exception::invalid_argument_exception("The size of channels should be the same!");
			}
			auto w1 = lhs.cols();
			auto h1 = lhs.rows();
			auto w2 = rhs.cols();
			auto h2 = rhs.rows();
			auto w = w1 < w2 ? w1 : w2;
			auto h = h1 < h2 ? h1 : h2;
			matrix<T> l_mat = lhs.crop({ 0,0,w,h }, chsL);
			matrix<T> r_mat = rhs.crop({ 0,0,w,h }, chsR);
			auto beg1 = l_mat.begin();
			auto end1 = l_mat.end();
			auto beg2 = r_mat.begin();
			auto end2 = r_mat.end();
			while (beg1 != end1 && beg2 != end2) {
				*beg1 = *beg2;
				++beg1; ++beg2;
			}
		}
		void __resize_instance_directly(size_type w, size_type h, size_type c) {
			if (!this->m_instance.is_uniqe() || this->m_instance.empty()) {
				this->m_instance.create_new();
			}
			this->m_instance->resize(w, h, c);
		}
		void __resize_instance_with_data(size_type w, size_type h, size_type c, bool _sampling) {
			matrix<T> temp = *this;
			if (!this->is_full_range()) {
				temp.to_uniqe();
			}

			this->__resize_instance_directly(w, h, c);
			this->m_beg.cols = 0;
			this->m_beg.rows = 0;
			this->m_size.w = w;
			this->m_size.h = h;
			this->m_indexes = myvec<size_type>::create_sequence(0, 1, static_cast<size_t>(c));
			if (_sampling) {
				cyh::container::details::resize_mat_with_sampling<T>(*this, temp);
			}
			else {
				cyh::container::details::resize_mat<T>(*this, temp);
			}
		}
		void _resize(size_type w, size_type h, size_type c, bool reserve_data, bool _sampling) {
			if (w == 0 && h == 0 && c == 0) {
				reserve_data = false;
			}

			if (reserve_data && !this->m_instance.empty() && this->elements() != 0) {
				__resize_instance_with_data(w, h, c, _sampling);
			} else {
				__resize_instance_directly(w, h, c);
				this->m_beg.cols = 0;
				this->m_beg.rows = 0;
				this->m_size.w = w;
				this->m_size.h = h;
				this->m_indexes = myvec<size_type>::create_sequence(0, 1, static_cast<size_t>(c));
			}
		}
		void _standardize() {
			auto mean = this->sum<double>() / this->elements();
			auto itBeg = this->begin();
			auto itEnd = this->end();
			while (itBeg != itEnd) {
				*itBeg -= mean;
				++itBeg;
			}
		}
		void _normalize() {
			auto sum = this->sum<double>();
			auto itBeg = this->begin();
			auto itEnd = this->end();
			while (itBeg != itEnd) {
				*itBeg /= sum;
				++itBeg;
			}
		}
	public:
		constexpr matrix() {}
		matrix(size_type width, size_type height, size_type chnn) {
			this->resize(width, height, chnn, false);
			(*this) = T{};
		}
		matrix(const matrix<T>&) = default;
		matrix(matrix<T>&&) = default;
		matrix<T>& operator=(const matrix<T>& other) {
			this->m_indexes = other.m_indexes;
			this->m_instance = other.m_instance;
			this->m_beg = other.m_beg;
			this->m_size = other.m_size;
			return *this;
		}
		matrix<T>& operator=(matrix<T>&& other) noexcept {
			this->m_indexes = std::move(other.m_indexes);
			this->m_instance = std::move(other.m_instance);
			this->m_beg = other.m_beg;
			this->m_size = other.m_size;
			return *this;
		}
		matrix<T>& operator=(const std::decay_t<T>& val) {
			auto& _this = *this;
			for (auto& _val : _this) {
				_val = val;
			}
			return *this;
		}
		virtual ~matrix() {}
		matrix<T> operator[](size_t index) const {
			if (index > this->m_indexes.size()) 
				throw cyh::exception::out_of_range_exception();
			matrix<T> result = *this;
			result.m_indexes = myvec<size_type>{};
			result.m_indexes.push_back(this->m_indexes[index]);
			return result;
		}
		constexpr iterator begin() {
			if (this->m_instance.empty()) {
				return iterator();
			} else {
				return iterator{ (this->raw_pointer() + this->get_offset_of_main(this->m_beg)), this->m_instance->m_size, this->m_size, this->m_indexes };
			}
		}
		constexpr iterator end() {
			if (this->m_instance.empty()) {
				return iterator();
			} else {
				return iterator{
					(this->raw_pointer() + this->get_offset_of_main(Point2D_<size_type>{ this->m_beg.x,(this->m_beg.y + this->m_size.h) })),
						this->m_instance->m_size,
						this->m_size,
						this->m_indexes
				};
			}
		}
		constexpr iterator begin() const {
			return ((matrix<T>*)this)->begin();
		}
		constexpr iterator end() const {
			return ((matrix<T>*)this)->end();
		}
		constexpr size_t size() const { return this->elements(); }
		constexpr bool is_uniqe() const { return this->m_instance.is_uniqe(); }
		constexpr bool is_full_range() const override {
			if (this->m_instance.empty()) return true;
			if (this->m_size != Size2D_<size_type>(this->m_instance->cols(), this->m_instance->rows())) {
				return false;
			}
			if (this->m_indexes.size() != this->m_instance->m_size.chnn) {
				return false;
			}
			return true;
		}
		constexpr bool empty() const {
			if (!this->m_instance.empty()) {
				return this->m_instance->empty();
			} else {
				return true;
			}
		}
		constexpr size_type elements() const {
			if (!this->m_instance.empty()) {
				return static_cast<size_type>(this->m_indexes.size() * this->m_size.w * this->m_size.h);
			} else {
				return 0;
			}
		}
		constexpr size_type rows() const override {
			if (!this->m_instance.empty()) {
				return this->m_size.y;
			} else {
				return 0;
			}
		}
		constexpr size_type cols() const override {
			if (!this->m_instance.empty()) {
				return this->m_size.x;
			} else {
				return 0;
			}
		}
		constexpr size_type chnn() const override {
			if (!this->m_instance.empty()) {
				return static_cast<size_type>(this->m_indexes.size());
			} else {
				return 0;
			}
		}
		constexpr size_type stride() const {
			return this->chnn() * this->cols();
		}
		constexpr float aspect() const {
			if (this->rows() == 0) {
				return numeric::max<float>();
			} else {
				return static_cast<float>(this->cols()) / static_cast<float>(this->rows());
			}
		}
		constexpr void resize(size_type w, size_type h, size_type c, bool resv_data = true, bool _sampling = false) {
			if (this->cols() == w && this->rows() == h && this->chnn() == c) { return; }
			this->_resize(w, h, c, resv_data, _sampling);
		}
		constexpr void print() const {
			size_type strides = this->stride();
			size_type counter = 0;
			auto pThis = (matrix<T>*)this;
			using print_config_data_type = decltype(std::declval<cyh::console::console_print_format>().item_width);
			pThis->m_print_configs.item_width = 5;
			pThis->m_print_configs.row_items = static_cast<print_config_data_type>(strides);
			pThis->m_print_configs.seperator = ",";
			console::details::print_utils::print_container(*this, this->m_print_configs);
		}
		constexpr void clone_to(matrix<T>& dst) const {
			dst = *this;
			dst.to_uniqe();
		}
		constexpr void copy_rect_to(matrix<T>& dst, size_type dstCh, size_type srcCh) const {
			copy_value_to_left(dst, { dstCh }, *this, { srcCh });
		}
		constexpr void copy_rect_to(matrix<T>& dst) const {
			if (this->chnn() != dst.chnn()) {
				throw cyh::exception::invalid_argument_exception("The channels of source and destination matrix should be the same!");
			}
			auto dstChs = myvec<size_type>::create_sequence(0, 1, dst.chnn());
			auto srcChs = myvec<size_type>::create_sequence(0, 1, this->chnn());
			copy_value_to_left(dst, std::initializer_list(dstChs.begin(), dstChs.end()), *this, std::initializer_list(srcChs.begin(), srcChs.end()));
		}
		constexpr void copy_rect_from(size_type dstCh, const matrix<T>& src, size_type srcCh) {
			src.copy_rect_to(*this, dstCh, srcCh);
		}
		constexpr void copy_rect_from(const matrix<T>& src) {
			src.copy_rect_to(*this);
		}
		constexpr Rect_<size_type> rect() const { return Rect_<size_type>{ this->m_beg, this->m_size }; }
		matrix<T> crop(const Rect_<size_type>& _rect, const std::initializer_list<size_type>& indexes) const {
			auto dep = this->chnn();
			for (auto i : indexes) {
				if (i >= dep) {
					throw cyh::exception::invalid_argument_exception("Input contains invalid index!");
				}
			}
			myvec<size_type> indexOnMain{};
			for (auto i : indexes) {
				indexOnMain.push_back(this->m_indexes[i]);
			}
			matrix<T> retMat{};
			Rect_<size_type> rectOnMain;

			if (this->is_full_range()) {
				rectOnMain = Rect_<size_type>::GetMaxRectOnMain(this->rect(), _rect);
			}
			else {
				Rect_<size_type> parsredRect = _rect;
				parsredRect.x += this->m_beg.x;
				parsredRect.y += this->m_beg.y;
				rectOnMain = Rect_<size_type>::GetMaxRectOnMain(this->rect(), parsredRect);
			}

			retMat.m_instance = this->m_instance;
			retMat.m_beg = Point2D_<size_type>(rectOnMain.x, rectOnMain.y);
			retMat.m_size = Size2D_<size_type>(rectOnMain.w, rectOnMain.h);
			retMat.m_indexes = std::move(indexOnMain);
			return retMat;
		}
		matrix<T> clone() const {
			matrix<T> result = *this;
			result.to_uniqe();
			return result;
		}
		void to_uniqe() {
			if (this->is_uniqe()) { return; }
			matrix<T> temp;
			temp.resize(this->m_size.w, this->m_size.h, this->m_indexes.size());
			auto itBeg1 = this->begin();
			auto itEnd1 = this->end();
			auto itBeg2 = temp.begin();
			auto itEnd2 = temp.end();
			while (itBeg1 != itEnd1 && itBeg2 != itEnd2) {
				*itBeg2 = *itBeg1;
				itBeg1++; itBeg2++;
			}
			matrix<T>& iThis = *this;
			iThis = std::move(temp);
		}
		void set_data_by_copy(size_type w, size_type h, size_type c, T* pdata) {
			this->resize(w, h, c);
			T* ptr = this->m_instance->data();
			MemoryHelper::Copy<T>(ptr, pdata, this->elements());
		}
		// Warning! If pdata is not nullptr, the lifetime will be handle by Matrix and should not be freed manually.
		void set_data_by_move(size_type w, size_type h, size_type c, T* pdata) {
			if (this->m_instance.empty()) {
				this->m_instance.create_new();
			}
			if (!this->m_instance.is_uniqe()) {
				this->m_instance.create_new();
			}
			this->m_instance->resize(w, h, c, pdata);
			this->m_size.w = w;
			this->m_size.h = h;
			this->m_indexes = myvec<size_type>::create_sequence(0, 1, c);
		}
		template<class Func, class...Args>
		void set_value_by(Func pfunc, Args&& ... args) {
			auto& inst = *this;
			for (auto& v : inst) {
				pfunc(v, std::forward<Args>(args)...);
			}
		}
		template<class V>
		matrix<V> convert_type() const {
			matrix<V> result;
			result.resize(this->cols(), this->rows(), this->chnn());
			auto itSrcBeg = this->begin();
			auto itSrcEnd = this->end();
			auto itDstBeg = result.begin();
			auto itDstEnd = result.end();
			while (itSrcBeg != itSrcEnd && itDstBeg != itDstEnd) {
				*itDstBeg = static_cast<V>(*itSrcBeg);
				++itSrcBeg; ++itDstBeg;
			}
			return result;
		}
		matrix<T> standardize() const {
			matrix<T> result = *this;
			result.to_uniqe();
			return result;
		}
		matrix<T> normalize() const {
			matrix<T> result = *this;
			result.to_uniqe();
			result._normalize();
			return result;
		}
		T max() const {
			if (this->empty()) {
				throw cyh::exception::invalid_argument_exception("The matrix is empty!");
			}
			auto itBeg = this->begin();
			auto itEnd = this->end();
			auto maxVal = *itBeg;
			while (itBeg != itEnd) {
				if (*itBeg > maxVal) {
					maxVal = *itBeg;
				}
				++itBeg;
			}
			return maxVal;
		}
		T min() const {
			if (this->empty()) {
				throw cyh::exception::invalid_argument_exception("The matrix is empty!");
			}
			auto itBeg = this->begin();
			auto itEnd = this->end();
			auto minVal = *itBeg;
			while (itBeg != itEnd) {
				if (*itBeg < minVal) {
					minVal = *itBeg;
				}
				++itBeg;
			}
			return minVal;
		}
		template<class V = T>
		V sum() const {
			return numeric::sum<V>(this->begin(), this->end());
		}

		T deviation() const {
			auto mean = this->sum<double>() / this->elements();
			auto itBeg = this->begin();
			auto itEnd = this->end();
			double sum = 0;
			while (itBeg != itEnd) {
				sum += (*itBeg - mean) * (*itBeg - mean);
				++itBeg;
			}
			return static_cast<T>(sqrt(sum / this->elements()));
		}
		T variance() const {
			auto mean = this->sum<double>() / this->elements();
			auto itBeg = this->begin();
			auto itEnd = this->end();
			double sum = 0;
			while (itBeg != itEnd) {
				sum += (*itBeg - mean) * (*itBeg - mean);
				++itBeg;
			}
			return static_cast<T>(sum / this->elements());
		}
	};

	template<class T, class TNum>
	matrix<T>& operator += (matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		for (auto& lv : lhs)
			lv += value;
		return lhs;
	}

	template<class T, class TNum>
	matrix<T>& operator -= (matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		for (auto& lv : lhs)
			lv -= value;
		return lhs;
	}

	template<class T, class TNum>
	matrix<T>& operator *= (matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		for (auto& lv : lhs)
			lv *= value;
		return lhs;
	}

	template<class T, class TNum>
	matrix<T>& operator /= (matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		for (auto& lv : lhs) 
			lv /= value;
		return lhs;
	}

	template<class T>
	matrix<T>& operator += (matrix<T>& lhs, const matrix<T>& value) {
		auto beg1 = lhs.begin();
		auto end1 = lhs.end();
		auto beg2 = value.begin();
		auto end2 = value.end();
		while (beg1 != end1 && beg2 != end2) {
			*beg1 += *beg2;
			++beg1; ++beg2;
		}
		return lhs;
	}

	template<class T>
	matrix<T>& operator -= (matrix<T>& lhs, const matrix<T>& value) {
		auto beg1 = lhs.begin();
		auto end1 = lhs.end();
		auto beg2 = value.begin();
		auto end2 = value.end();
		while (beg1 != end1 && beg2 != end2) {
			*beg1 -= *beg2;
			++beg1; ++beg2;
		}
		return lhs;
	}

	template<class T>
	matrix<T>& operator *= (matrix<T>& lhs, const matrix<T>& value) {
		auto beg1 = lhs.begin();
		auto end1 = lhs.end();
		auto beg2 = value.begin();
		auto end2 = value.end();
		while (beg1 != end1 && beg2 != end2) {
			*beg1 *= *beg2;
			++beg1; ++beg2;
		}
		return lhs;
	}

	template<class T>
	matrix<T>& operator /= (matrix<T>& lhs, const matrix<T>& value) {
		auto beg1 = lhs.begin();
		auto end1 = lhs.end();
		auto beg2 = value.begin();
		auto end2 = value.end();
		while (beg1 != end1 && beg2 != end2) {
			*beg1 /= *beg2;
			++beg1; ++beg2;
		}
		return lhs;
	}

	template<class T, class TNum>
	matrix<T> operator + (const matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result += value;
		return result;
	}

	template<class T, class TNum>
	matrix<T> operator - (const matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result -= value;
		return result;
	}

	template<class T, class TNum>
	matrix<T> operator * (const matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result *= value;
		return result;
	}

	template<class T, class TNum>
	matrix<T> operator / (const matrix<T>& lhs, TNum value) requires(cyh::type::is_pure_num_v<TNum>) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result /= value;
		return result;
	}

	template<class T>
	matrix<T> operator + (const matrix<T>& lhs, const matrix<T>& value) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result += value;
		return result;
	}

	template<class T>
	matrix<T> operator - (const matrix<T>& lhs, const matrix<T>& value) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result -= value;
		return result;
	}

	template<class T>
	matrix<T> operator * (const matrix<T>& lhs, const matrix<T>& value) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result *= value;
		return result;
	}

	template<class T>
	matrix<T> operator / (const matrix<T>& lhs, const matrix<T>& value) {
		matrix<T> result = lhs;
		result.to_uniqe();
		result /= value;
		return result;
	}

};
