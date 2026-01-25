#pragma once
#include "container_.hpp"
#include "../buffer.hpp"
#include "../numeric.hpp"
namespace cyh::container {
	template <class T, size_t _Size = 0>
	struct myvec {
		using value_type = T;
	};

	template <class T>
	struct myvec<T, 0> : cyh::buffer {
		using value_type = T;
		virtual ~myvec() {}
		constexpr T* begin() const { return this->data(); }
		constexpr T* end() const { return this->data() + this->size(); }
		constexpr T* rbegin() const { return this->end() - 1; }
		constexpr T* rend() const { return this->begin() - 1; }
		constexpr T* data() const { return this->data_t<T>(0); }
		constexpr T& operator[](size_t index) const {
			if (index >= this->size())
				throw cyh::exception::out_of_range_exception().src(__POSITION__);
			return this->data()[index];
		}
		constexpr myvec() : buffer((T*)nullptr) {}
		myvec(const myvec<T, 0>& other) : buffer((T*)nullptr) {
			buffer& bThis = *this;
			buffer& bOther = *get_ptr(other);
			bThis.operator=(bOther);
		}
		myvec(myvec<T, 0>&& other) noexcept : buffer((T*)nullptr) {
			buffer& bThis = *this;
			buffer& bOther = *get_ptr(other);
			bThis.operator=(std::move(bOther));
		}
		myvec<T>& operator=(const myvec<T, 0>& other) {
			buffer& bThis = *this;
			buffer& bOther = *get_ptr(other);
			bThis.operator=(bOther);
			return *this;
		}
		myvec<T>& operator=(myvec<T, 0>&& other) noexcept {
			buffer& bThis = *this;
			buffer& bOther = *get_ptr(other);
			bThis.operator=(std::move(bOther));
			return *this;
		}
		constexpr void push_back(const T& _val) { this->push_back_t<T>(_val); }
		constexpr void push_back(T&& _val) { this->push_back_t<T>(std::move(_val)); }
		constexpr void reserve(size_t resv) { this->expand_capacity(resv); }
		constexpr ref<T> pop_back() { return this->pop_back_t<T>(); }
		static myvec<T, 0> create_sequence(T _beg, T interval, size_t count) {
			myvec<T, 0> result;
			T current = _beg;
			for (size_t i = 0; i < count; ++i) {
				result.push_back(current);
				current += interval;
			}
			return result;
		}
	};

	namespace details {
		class PrimitiveVecHelper {
		public:
			template<class T, class U>
			static constexpr void SetValues(T& vec, U val)
				requires(cyh::type::is_primitive_type_v<std::decay_t<U>>&& cyh::type::is_container_v<std::decay_t<T>>) {
				using value_type = T::value_type;
				auto ptr = vec.data();
				for (size_t i = 0; i < vec.size(); ++i) {
					ptr[i] = static_cast<value_type>(val);
				}
			}
			template<class T, class U>
			static constexpr void Calculate(T& vec, U value, std::function<std::decay_t<U>(std::decay_t<U>, std::decay_t<U>)> func)
				requires(cyh::type::is_primitive_type_v<std::decay_t<U>>) {
				using value_type = std::decay_t<ContentType<T>>;
				auto ptr = vec.data();
				for (size_t i = 0; i < vec.size(); i++) {
					ptr[i] = static_cast<value_type>(func(static_cast<std::decay_t<U>>(ptr[i]), value));
				}
			}
			template<class T, class U>
			static constexpr void Calculate(T& vec, const U& vec2, std::function<ContentType<U>(ContentType<U>, ContentType<U>)> func)
				requires(cyh::type::is_primitive_type_v<std::decay_t<ContentType<U>>>) {
				using value_type = std::decay_t<ContentType<T>>;
				using value_type2 = std::decay_t<ContentType<U>>;
				size_t size = vec.size() < vec2.size() ? vec.size() : vec2.size();
				auto ptr = vec.data();
				auto ptr2 = vec2.data();
				for (size_t i = 0; i < size; i++) {
					ptr[i] = static_cast<value_type>(func(static_cast<value_type2>(ptr[i]), ptr2[i]));
				}
			}
			template<class T, class U>
			static constexpr bool IsContentEqual(const T& vec1, const U& vec2)
				requires(std::is_same_v<ContentType<T>, ContentType<U>>) {
				if (vec1.size() != vec2.size()) {
					return false;
				} else {
					auto ptr1 = vec1.data();
					auto ptr2 = vec2.data();
					for (size_t i = 0; i < vec1.size(); ++i) {
						if (ptr1[i] != ptr2[i]) return false;
					}
					return true;
				}
			}
			template<class T, class U>
			static constexpr void SafeSetValues(T& vec1, const U vec2)
				requires(is_primitive_container_v<T>&& is_primitive_container_v<U>) {
				using value_type = std::decay_t<ContentType<T>>;
				using value_type2 = std::decay_t<ContentType<U>>;
				size_t size = vec1.size() < vec2.size() ? vec1.size() : vec2.size();
				auto ptr1 = vec1.data();
				auto ptr2 = vec2.data();
				for (size_t i = 0; i < vec1.size(); ++i) {
					ptr1[i] = numeric::SafeCast<T>(ptr2[i]);
				}
			}
			template<class T, class U>
			static constexpr void SafeSetValues(T& vec, U val)
				requires(is_primitive_container_v<T>&& cyh::type::is_primitive_type_v<U>) {
				using value_type = std::decay_t<ContentType<T>>;
				size_t size = vec.size();
				auto ptr = vec.data();
				for (size_t i = 0; i < size; ++i) {
					ptr[i] = numeric::SafeCast<T>(val);
				}
			}
			template<class Vec>
			static Vec GetAutoFilledVec(ContentType<Vec> begin, ContentType<Vec> count) requires(is_num_container_v<Vec>) {
				ContentType<Vec> end = begin + count;
				Vec vec{};
				for (auto i = begin; i < end; ++i) {
					vec.push_back(i);
				}
				return vec;
			}
			template<class Vec>
			static constexpr bool Contains(const Vec& vec, ContentType<Vec> val) requires(is_num_container_v<Vec>) {
				for (auto i : vec) {
					if (i == val) { return true; }
				}
				return false;
			}
		};
	};

	template <class T>
	struct myvec<T, 2> {
		using value_type = T;
		union {
			T m_data[2];
			struct {
				T x, y;
			};
			struct {
				T w, h;
			};
			struct {
				T cols, rows;
			};
		};
		constexpr T* data() const {
			return (T*)m_data;
		}
		constexpr T* begin() const {
			return data;
		}
		constexpr T* end() const {
			return begin() + size();
		}
		constexpr size_t size() const {
			return 2;
		}
		constexpr myvec() : x(0), y(0) {}
		constexpr myvec(T x, T y) : x(x), y(y) {}
	};

	template <class T>
	struct myvec<T, 3> {
		using value_type = T;
		union {
			T m_data[3];
			struct {
				T x, y, z;
			};
			struct {
				T w, h, d;
			};
			struct {
				T cols, rows, chnn;
			};
			struct {
				T r, g, b;
			};
		};
		constexpr T* data() const {
			return (T*)m_data;
		}
		constexpr T* begin() const {
			return data;
		}
		constexpr T* end() const {
			return begin() + size();
		}
		constexpr size_t size() const {
			return 3;
		}
		constexpr myvec() : x(0), y(0), z(0) {}
		constexpr myvec(T x, T y, T z) : x(x), y(y), z(z) {}
		constexpr myvec(myvec<T, 2> v, T z) : x(v.x), y(v.y), z(z) {}
		constexpr myvec(T x, myvec<T, 2> v) : x(x), y(v.x), z(v.y) {}
	};

	template <class T>
	struct myvec<T, 4> {
		using value_type = T;
		union {
			T m_data[4];
			struct {
				T x, y, w, h;
			};
			struct {
				T r, g, b, a;
			};
		};
		constexpr T* data() const {
			return (T*)m_data;
		}
		constexpr T* begin() const {
			return data;
		}
		constexpr T* end() const {
			return begin() + size();
		}
		constexpr size_t size() const {
			return 4;
		}
		constexpr myvec() : x(0), y(0), w(0), h(0) {}
		constexpr myvec(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {}
		constexpr myvec(myvec<T, 2> v1, myvec<T, 2> v2) : x(v1.x), y(v1.y), w(v2.x), h(v2.y) {}
		constexpr myvec(myvec<T, 3> v, T w) : x(v.x), y(v.y), w(v.z), h(w) {}
		constexpr myvec(T x, myvec<T, 3> v) : x(x), y(v.x), w(v.y), h(v.z) {}
	};

	template<class T, size_t sz>
	constexpr myvec<T, sz>& operator+=(myvec<T, sz>& lhs, const myvec<T, sz>& rhs) {
		for (size_t i = 0; i < sz; ++i) {
			lhs.m_data[i] += rhs.m_data[i];
		}
		return lhs;
	}

	template<class T, size_t sz>
	myvec<T, sz> operator+(const myvec<T, sz>& lhs, const myvec<T, sz>& rhs) {
		myvec<T, sz> ret{ lhs };
		ret += rhs;
		return ret;
	}

	template<class T, size_t sz>
	constexpr bool operator==(const myvec<T, sz>& lhs, const myvec<T, sz>& rhs) {
		return details::PrimitiveVecHelper::IsContentEqual(lhs, rhs);
	}

	template <class T>
	using MyVec2 = myvec<T, 2>;
	template <class T>
	using MyVec3 = myvec<T, 3>;
	template <class T>
	using MyVec4 = myvec<T, 4>;
};
