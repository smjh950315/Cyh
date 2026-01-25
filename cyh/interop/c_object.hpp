#pragma once
#include "interop_.hpp"
namespace cyh::interop
{
	template<class T>
	class c_array_;

	class c_object {
		size_t m_typesize{};
		std::type_info* m_typeinfo{};
	public:
		template<class T>
		c_object(T* val);
		size_t type_size() const;
		size_t hash_code() const;
		const char* type_name() const;
		virtual ~c_object() = default;
	};

	class c_array : public c_object
	{
		template<class T>
		friend class c_array_;
		template<class T>
		c_array(c_array_<T>* drv) : c_object(drv) {}
	public:
		virtual size_t element_hash() const = 0;
		virtual size_t element_size() const = 0;
		virtual void* data() const = 0;
		virtual size_t size() const = 0;
		virtual void resize(size_t sz) = 0;
		virtual void reserve(size_t sz) = 0;
		virtual ~c_array() = default;
	};

	template<class T>
	class c_array_ : public c_array
	{
		std::vector<T> m_vec;
	public:
		c_array_();
		c_array_(const c_array_<T>& other);
		c_array_(c_array_<T>&& other) noexcept;
		c_array_<T>& operator=(const c_array_<T>& other);
		c_array_<T>& operator=(c_array_<T>&& other) noexcept;
		size_t element_hash() const override;
		size_t element_size() const override;
		virtual void* data() const override;
		virtual size_t size() const override;
		virtual void resize(size_t sz) override;
		virtual void reserve(size_t sz) override;
		virtual ~c_array_() = default;
	};
};
