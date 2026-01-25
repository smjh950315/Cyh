#pragma once
#include "c_object.hpp"
namespace cyh::interop {

	template<class T>
	c_object::c_object(T* val)
	{
		this->m_typeinfo = cyh::get_ptr(typeid(T));
		this->m_typesize = sizeof(T);
	}
	
	template<class T>
	c_array_<T>::c_array_() : c_array(this) {}
	template<class T>
	c_array_<T>::c_array_(const c_array_<T>& other) : c_array(this) {
		this->m_vec = other.m_vec;
	}
	template<class T>
	c_array_<T>::c_array_(c_array_<T>&& other) noexcept : c_array(this) {
		this->m_vec = std::move(other.m_vec);
	}
	template<class T>
	c_array_<T>& c_array_<T>::operator=(const c_array_<T>& other) {
		this->m_vec = other.m_vec;
		return *this;
	}
	template<class T>
	c_array_<T>& c_array_<T>::operator=(c_array_<T>&& other) noexcept {
		this->m_vec = std::move(other.m_vec);
		return *this;
	}
	template<class T>
	size_t c_array_<T>::element_hash() const {
		return typeid(T).hash_code();
	}
	template<class T>
	size_t c_array_<T>::element_size() const {
		return sizeof(T);
	}
	template<class T>
	void* c_array_<T>::data() const {
		return (void*)this->m_vec.data();
	}
	template<class T>
	size_t c_array_<T>::size() const {
		return this->m_vec.size();
	}
	template<class T>
	void c_array_<T>::resize(size_t sz) {
		this->m_vec.resize(sz);
	}
	template<class T>
	void c_array_<T>::reserve(size_t sz) {
		this->m_vec.reserve(sz);
	}
};
