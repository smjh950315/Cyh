#pragma once
#include <cyh/typedef.hpp>
#include <cyh/reference.hpp>
#include <vector>
#include "my_vec.hpp"
namespace cyh::container
{
	class ndarray;

	namespace details
	{
		template<class T>
		using vec = std::vector<T>;
		extern size_t cross(const vec<size_t>& _vec);
		extern bool is_equal(const vec<size_t>& l, const vec<size_t>& r);
		extern size_t cross(const myvec<size_t>& _vec);
		extern bool is_equal(const myvec<size_t>& l, const myvec<size_t>& r);
		
		template<class T>
		vec<T> get_begin_of(const vec<T>& in, size_t count) {
			auto max = count < in.size() ? count : in.size();
			vec<T> ret;
			for (size_t i = 0; i < max; ++i) 
				ret.push_back(in[i]);
			return ret;
		}

		struct array_data
		{
			myvec<unsigned char> data;
			vec<size_t> shape;
			array_data() = default;
			~array_data() = default;
			array_data(const array_data&) = default;
			array_data(array_data&&) noexcept = default;
			array_data& operator=(const array_data&) = default;
			array_data& operator=(array_data&&) noexcept = default;
			void resize(const vec<size_t>& _shape, size_t _typesize)
			{
				if (is_equal(_shape, shape))
					return;
				auto requiredBytes = cross(_shape) * _typesize;
				if (requiredBytes > data.size())
					data.resize(requiredBytes);
				shape = _shape;
			}
		};

		class nditerator
		{
		protected:
			static void copy_to_left(nditerator* l, const nditerator* r);
			static void move_to_left(nditerator* l, nditerator* r);
			static void move_next(nditerator* it);
		public:
			using value_type = unsigned char;
			using iterator = nditerator;
			using reference = value_type&;
			using pointer = value_type*;
			ref<array_data> data;
			vec<size_t> shape_r;
			vec<MyVec2<size_t>> range;
			vec<size_t> pos;
			size_t dim{};
			size_t typesize{};
			nditerator() {}
			nditerator(const nditerator&);
			nditerator(nditerator&&) noexcept;
			nditerator& operator=(const nditerator&);
			nditerator& operator=(nditerator&&) noexcept;
			nditerator(const ref<array_data>& d, const vec<MyVec2<size_t>>& r, size_t ts);
			virtual ~nditerator();
			const value_type& operator*() const;
			value_type& operator*();
			const pointer operator->() const;
			pointer operator->();
			iterator& operator++();
			iterator operator++(int);
			bool operator == (const iterator& rhs) const;
			bool operator != (const iterator& rhs) const;
		};

		template<class T>
		class nditerator_ final : private nditerator
		{
		public:
			using value_type = T;
			using iterator = nditerator_<T>;
			using reference = value_type&;
			using pointer = value_type*;
			nditerator_(const iterator& _other) {
				copy_to_left(this, &_other);
			}
			nditerator_(iterator&& _other) noexcept {
				move_to_left(this, &_other);
			}
			iterator& operator=(const iterator& _other) {
				copy_to_left(this, &_other);
				return *this;
			}
			iterator& operator=(iterator&& _other) noexcept {
				move_to_left(this, &_other);
				return *this;
			}
			nditerator_(const nditerator& it) { copy_to_left(this, &it); }
			nditerator_(nditerator&& it) { move_to_left(this, &it); }
			nditerator_(const ref<array_data>& d, const vec<MyVec2<size_t>>& r, size_t ts) : nditerator(d, r, ts) {}
			~nditerator_() {}
			const value_type& operator*() const {
				return *(this->operator->());
			}
			value_type& operator*() {
				return *(this->operator->());
			}
			const pointer operator->() const {
				return (pointer)(nditerator::operator->());
			}
			pointer operator->() {
				return (pointer)(nditerator::operator->());
			}
			iterator& operator++() {
				move_next(this);
				return *this;
			}
			iterator operator++(int) {
				iterator ret = *this;
				move_next(this);
				return ret;
			}
			bool operator == (const iterator& rhs) const {
				return is_equal(pos, rhs.pos);
			}
			bool operator != (const iterator& rhs) const {
				return !is_equal(pos, rhs.pos);;
			}
		};
	};
	class ndarray
	{
	public:
		template<class T>
		using vec = std::vector<T>;
	private:
		ref<details::array_data> m_data;
		vec<MyVec2<size_t>> m_range{};
		friend void copy_to_left(ndarray* l, const ndarray* r);
		friend void move_to_left(ndarray* l, ndarray* r);
	protected:
		size_t m_typesize{ 1 };
		ndarray();
	public:
		using iterator = details::nditerator;
		using func_op_data = void(*)(void*, void*);
		using func_print_data = void(*)(void*);		
		ndarray(const ndarray&);
		ndarray(ndarray&&) noexcept;
		ndarray& operator=(const ndarray&);
		ndarray& operator=(ndarray&&) noexcept;
		size_t typesize() const;
		size_t dims() const;
		vec<size_t> shape() const;
		virtual void resize(const vec<size_t>& _shape);
		bool crop(ndarray& out, const vec<size_t>& beg, const vec<size_t>& end);
		ndarray crop(const vec<size_t>& beg, const vec<size_t>& end);
		iterator begin();
		iterator end();
		const iterator begin() const;
		const iterator end() const;
		void copy_to(const ndarray& _other, func_op_data _convert = nullptr);
		void operate_with_other(const ndarray& _other, const std::function<void(void*, void*)>& _op);
		void print_format(func_print_data _print);
		virtual ~ndarray() {}
	};
	template<class T>
	class ndarray_ : public ndarray
	{
		static void func_copy_to_left_(void* l, void* r) {
			T* vl = (T*)l, * vr = (T*)r;
			*vl = *vr;
		}
	public:
		using iterator = details::nditerator_<T>;
		ndarray_() : ndarray() { this->m_typesize = sizeof(T); }
		virtual ~ndarray_() = default;
		iterator begin() { return ndarray::begin(); }
		iterator end() { return ndarray::end(); }
		const iterator begin() const { return ndarray::begin(); }
		const iterator end() const { return ndarray::end(); }
		void operate_with_other(const ndarray_<T>& _other, const std::function<void(T*, T*)>& _op) {
			ndarray::operate_with_other(_other,
										[_op](void* l, void* r)
										{
											_op((T*)l, (T*)r);
										});
		}
		void swap_val(const ndarray_<T>& _other) {
			operate_with_other(_other,
							   [](T* l, T* r)
							   {
								   T tmp = *l; *l = *r; *r = tmp;
							   });
		}
		void copy_to(const ndarray_<T>& _other) {
			operate_with_other(_other, 
							   [](T*l,T*r)
							   {
								   *l = *r;
							   });
		}
	};
	template<class T>
	class array3d_ final : public ndarray_<T>
	{
	public:
		template<class T>
		using vec = std::vector<T>;
		array3d_(const ndarray& _other) {
			if (_other.shape().size() != 3) 
				throw std::exception("dim should be 3");
			ndarray& This = *this;
			This = _other;
		}
		array3d_() {}
		~array3d_() {}
		void resize(const vec<size_t>& _shape) override {
			if (_shape.size() != 3)
				throw std::exception("dim should be 3");
			ndarray::resize(_shape);
		}
		array3d_ operator[](size_t index) {
			auto sh = this->shape();
			auto newSh = details::get_begin_of(sh, 2);
			newSh.push_back(index);
			return this->crop({ 0, 0, index }, newSh);
		}
	};
};
