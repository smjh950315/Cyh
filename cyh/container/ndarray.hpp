#pragma once
#include <cyh/typedef.hpp>
#include <cyh/reference.hpp>
#include <vector>
#include "my_vec.hpp"
namespace cyh::container
{
	class ndarray;

	template<class T>
	class ndarray_;

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
			array_data() = default;
			~array_data() = default;
			array_data(const array_data&) = default;
			array_data(array_data&&) noexcept = default;
			array_data& operator=(const array_data&) = default;
			array_data& operator=(array_data&&) noexcept = default;
			void resize(const vec<size_t>& _shape, size_t _typesize);
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
			nditerator(const ref<array_data>& d, const vec<size_t>& dshape, const vec<MyVec2<size_t>>& r, size_t ts);
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
		const std::type_info* m_typeinfo;
		ref<details::array_data> m_data;
		vec<MyVec2<size_t>> m_range{};
		vec<size_t> m_datashape;
	protected:
		size_t m_typesize;		
		ndarray(size_t _typesize, const std::type_info* _typeinfo);
		template<class T>
		ndarray(T* ptr) : ndarray(sizeof(T), &typeid(T)) {}
	public:
		static void copy_to_left(ndarray* l, const ndarray* r);
		static void move_to_left(ndarray* l, ndarray* r);
		using iterator = details::nditerator;
		using func_op_from_data = void(*)(void*, const void*);
		using func_op_to_data = void(*)(const void*, void*);	
		using func_print_data = void(*)(void*);		
		iterator begin();
		iterator end();
		const iterator begin() const;
		const iterator end() const;
		ndarray();
		ndarray(const ndarray&);
		ndarray(ndarray&&) noexcept;
		ndarray& operator=(const ndarray&);
		ndarray& operator=(ndarray&&) noexcept;
		bool is_initialized() const;
		bool is_full_range() const;
		const std::type_info* typeinfo() const;
		size_t typesize() const;
		size_t dims() const;
		size_t elements() const;

		virtual vec<size_t> shape() const;
		virtual void resize(const vec<size_t>& _shape);
		virtual bool reshape(const vec<size_t>& _shape);

		bool crop(ndarray& out, const vec<size_t>& beg, const vec<size_t>& end) const;
		ndarray crop(const vec<size_t>& beg, const vec<size_t>& end) const;
		bool clone(ndarray& out) const;
		ndarray clone() const;
		ndarray slice(size_t index) const;
		void operate_with_other(ndarray& _other, const std::function<void(void*, void*)>& _op);
		void operate_with_other(const ndarray& _other, const std::function<void(void*, const void*)>& _op);
		void operate_with_other(ndarray& _other, const std::function<void(const void*, void*)>& _op) const;
		void operate(const std::function<void(void*)>& _op);
		void copy_to(ndarray& _other, func_op_to_data _convert = nullptr) const;
		void print_format(func_print_data _print) const;
		virtual ~ndarray() {}
	};
	
	template<class T>
	class ndarray_ : public ndarray
	{
		template<class U, class V>
		static void p_copy_from(U& l, const V& r) { l = static_cast<U>(r); }
		template<class U, class V>
		static void p_copy_to(const U& l, V& r) { r = static_cast<V>(l); }
		template<class U, class V>
		static void p_swap(U& l, V& r) { U t = static_cast<U>(r); r = static_cast<V>(l); l = t; }
	public:
		using iterator = details::nditerator_<T>;
		ndarray_() : ndarray((T*)0) {}
		virtual ~ndarray_() = default;
		iterator begin() { return ndarray::begin(); }
		iterator end() { return ndarray::end(); }
		const iterator begin() const { return ndarray::begin(); }
		const iterator end() const { return ndarray::end(); }
		template<class V>
		void operate_with_other(ndarray_<V>& _other, const std::function<void(T&, V&)>& _op) {
			ndarray::operate_with_other(_other,
										[_op](void* l, void* r)
										{
											_op(*((T*)l), *((V*)r));
										});
		}
		template<class V>
		void operate_with_other(const ndarray_<V>& _other, const std::function<void(T&, const V&)>& _op) {
			ndarray::operate_with_other(_other,
										[_op](void* l, const void* r)
										{
											_op(*((T*)l), *((V*)r));
										});
		}		
		template<class V>
		void operate_with_other(ndarray_<V>& _other, const std::function<void(const T&, V&)>& _op) const {
			ndarray::operate_with_other(_other,
										[_op](const void* l, void* r)
										{
											_op(*((T*)l), *((V*)r));
										});
		}
		void operate(const std::function<void(T&)>& _op) {
			ndarray::operate([_op](void* l)
							 {
								 _op(*((T*)l));
							 });
		}
		template<class V>
		void swap_val(const ndarray_<V>& _other) {
			this->operate_with_other<V>(_other, p_swap<T, V>);
		}
		template<class V>
		void copy_from(const ndarray_<V>& _other) {
			this->operate_with_other<V>(_other, p_copy_from<T, V>);
		}
		template<class V>
		void copy_to(ndarray_<V>& _other) const {
			this->operate_with_other<V>(_other, p_copy_to<T, V>);
		}
		ndarray_<T> crop(const vec<size_t>& beg, const vec<size_t>& end) const {
			ndarray_<T> tmp{};
			ndarray::crop(tmp, beg, end);
			return tmp;
		}
		ndarray_<T> clone() const {
			ndarray_<T> tmp{};
			ndarray::clone(tmp);
			return tmp;
		}

		template<class V>
		ndarray_<T>& operator=(const V& val) requires(cyh::type::is_pure_num_v<T> && cyh::type::is_pure_num_v<V>)
		{
			T _val = static_cast<T>(val);
			operate([_val](T& l) { l = _val; }); return *this;
		}
	};

	template<class T, class V>
	ndarray_<T>& operator+=(ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		T _r = static_cast<T>(r);
		l.operate([_r](T& val) { val += _r; }); return l;
	}
	template<class T, class V>
	ndarray_<T>& operator-=(ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		T _r = static_cast<T>(r);
		l.operate([_r](T& val) { val -= _r; }); return l;
	}
	template<class T, class V>
	ndarray_<T>& operator*=(ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		T _r = static_cast<T>(r);
		l.operate([_r](T& val) { val *= _r; }); return l;
	}

	template<class T, class V>
	ndarray_<T> operator+(const ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		ndarray_<T> ret = l.clone(); ret += r; return ret;
	}
	template<class T, class V>
	ndarray_<T> operator-(const ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		ndarray_<T> ret = l.clone(); ret -= r; return ret;
	}
	template<class T, class V>
	ndarray_<T> operator*(const ndarray_<T>& l, V r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		ndarray_<T> ret = l.clone(); ret *= r; return ret;
	}

	template<class T, class V>
	ndarray_<T>& operator+=(ndarray_<T>& l, const ndarray_<V>& r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		l.operate_with_other(r,
							 [](T& vl, const V& vr) {
								 vl += static_cast<T>(vr);
							 });
		return l;
	}
	template<class T, class V>
	ndarray_<T>& operator-=(ndarray_<T>& l, const ndarray_<V>& r) requires(cyh::type::is_pure_num_v<T>&& cyh::type::is_pure_num_v<V>)
	{
		l.operate_with_other(r,
							 [](T& vl, const V& vr) {
								 vl -= static_cast<T>(vr);
							 });
		return l;
	}


	template<class T>
	class array3d_ final : public ndarray_<T>
	{
	public:
		template<class V>
		using vec = std::vector<V>;
		array3d_ operator[](size_t index) {
			return this->slice(index);
		}
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
	};
};
