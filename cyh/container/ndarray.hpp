#pragma once
#include <cyh/typedef.hpp>
#include <cyh/reference.hpp>
#include <vector>
#include "my_vec.hpp"
namespace cyh::container
{
	class ndarray;
	class ndarray
	{
	public:
		template<class T>
		using vec = std::vector<T>;
		virtual size_t typesize() const = 0;
		virtual size_t dims() const = 0;
		virtual vec<size_t> shape() const = 0;
		virtual ~ndarray() = default;
	};
	namespace details
	{
		template<class T>
		using vec = std::vector<T>;
		extern size_t cross(const vec<size_t>& _vec);
		extern bool is_equal(const vec<size_t>& l, const vec<size_t>& r);
		extern size_t cross(const myvec<size_t>& _vec);
		extern bool is_equal(const myvec<size_t>& l, const myvec<size_t>& r);
		struct array_data
		{
			vec<char> data;
			vec<size_t> shape;
			size_t typesize{1};
			void resize(const vec<size_t>& _shape)
			{
				if (is_equal(_shape, shape))
					return;
				auto requiredBytes = cross(_shape) * typesize;
				if (requiredBytes > data.size())
					data.resize(requiredBytes);
				shape = _shape;
			}
		};
		struct nditerator
		{
			using value_type = char;
			using iterator = nditerator;
			using reference = value_type&;
			using pointer = value_type*;
			ref<array_data> data;
			vec<size_t> shape_r;
			vec<MyVec2<size_t>> range;
			vec<size_t> pos;
			size_t dim{};
			nditerator(const nditerator&);
			nditerator(nditerator&&) noexcept;
			nditerator& operator=(const nditerator&);
			nditerator& operator=(nditerator&&) noexcept;
			nditerator(const ref<array_data>& d, const vec<MyVec2<size_t>>& r);
			~nditerator();
			const value_type& operator*() const;
			value_type& operator*();
			const pointer operator->() const;
			pointer operator->();
			iterator& operator++();
			iterator operator++(int);
			bool operator == (const iterator& rhs) const;
			bool operator != (const iterator& rhs) const;
		};
	};
	class xdarray : public ndarray
	{
		ref<details::array_data> m_data;
		vec<MyVec2<size_t>> m_range{};
	public:
		using iterator = details::nditerator;
		size_t typesize() const override;
		size_t dims() const override;
		vec<size_t> shape() const override;
		void resize(const vec<size_t>& _shape);
		bool crop(xdarray& out, const vec<size_t>& beg, const vec<size_t>& end);
		iterator begin();
		iterator end();
		const iterator begin() const;
		const iterator end() const;
		~xdarray() {}
	};
};
