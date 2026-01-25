#pragma once
#include "typedef.hpp"
#include "exceptions.hpp"
namespace cyh {
	namespace numeric {
		template<class T>
		constexpr T zero() requires(cyh::type::is_numeric_v<T>) { return T(0); }
		template<class T>
		constexpr T max() requires(cyh::type::is_numeric_v<T>) { return std::numeric_limits<T>::max(); }
		template<class T>
		constexpr T min() requires(cyh::type::is_numeric_v<T>) { return std::numeric_limits<T>::min(); }
	};
	namespace details {
		using max_unsigned_type = uint64;
		using max_singed_type = int64;
		using max_float_type = double;
		extern double get_rand_in_range(double r1, double r2);
		extern int parse_hex_str(const char* str, size_t length, int64* result);
		extern int to_hex_str(uint64 value, char* str, size_t* iolength);
		template<class T>
		constexpr double max_d() { return static_cast<double>(std::numeric_limits<T>::max()); }
		template<class T>
		constexpr double min_d() { return static_cast<double>(std::numeric_limits<T>::min()); }
		template<class T>
		constexpr bool contains_type_t() { return true; }
		template<class T, class V, class ...Xs>
		constexpr bool contains_type_t() {
			if constexpr (std::is_same_v<T, V>) {
				return true;
			} else if constexpr (sizeof...(Xs) == 0) {
				return false;
			} else {
				return contains_type_t<T, Xs...>();
			}
		}
		template<class T>
		static constexpr T get_inrange_min_max(T value, T min, T max) {
			if (value < min) {
				return min;
			} else if (value > max) {
				return max;
			} else {
				return value;
			}
		}
		struct num_compare {
			template<class T>
			static constexpr int compare(T val1, T val2) {
				if (val1 == val2) return 0;
				return val1 < val2 ? -1 : 1;
			}
			template<class T, class U>
			static constexpr int compare(T val1, U val2) requires(std::is_integral_v<T>&& std::is_integral_v<U> && !std::is_same_v<T, U>) {
				if constexpr (std::is_signed_v<T> && std::is_signed_v<U>) { // signed only
					return compare<max_singed_type>((max_singed_type)val1, (max_singed_type)val2);
				} else if constexpr (!std::is_signed_v<T> && !std::is_signed_v<U>) { // unsigned only
					return compare<max_singed_type>((max_unsigned_type)val1, (max_unsigned_type)val2);
				} else if constexpr (std::is_signed_v<T>) { // T is unsigned
					return val1 < 0 ? -1 : compare<max_singed_type>(static_cast<max_unsigned_type>(val1), static_cast<max_unsigned_type>(val2));
				} else { // U is signed
					return val2 < 0 ? 1 : compare<max_singed_type>(static_cast<max_unsigned_type>(val1), static_cast<max_unsigned_type>(val2));
				}
			}
			template<class T, class U>
			static constexpr int compare(T val1, U val2) requires(std::is_floating_point_v<T>&& std::is_floating_point_v<U> && !std::is_same_v<T, U>) {
				return compare<max_float_type>(static_cast<max_float_type>(val1), static_cast<max_float_type>(val2));
			}
			template<class T, class U>
			static constexpr int compare(T val1, U val2) requires(std::is_integral_v<T> != std::is_integral_v<U>) {
				return compare<max_float_type>(static_cast<max_float_type>(val1), static_cast<max_float_type>(val2));
			}
			template<class T, class U>
			static constexpr T max_of(T val1, U val2) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>) {
				return compare(val1, val2) == -1 ? val2 : val1;
			}
			template<class T, class U>
			static constexpr T min_of(T val1, U val2) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>) {
				return compare(val1, val2) == -1 ? val1 : val2;
			}
			template<class T, class U, class V, class... Xs>
			static constexpr T max_of(T val1, U val2, V val3, Xs... xs) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>&& cyh::type::is_numeric_v<V> && (cyh::type::is_numeric_v<Xs> && ...)) {
				return max_of(max_of(val1, val2), val3, xs...);
			}
			template<class T, class U, class V, class... Xs>
			static constexpr T min_of(T val1, U val2, V val3, Xs... xs) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>&& cyh::type::is_numeric_v<V> && (cyh::type::is_numeric_v<Xs> && ...)) {
				return min_of(min_of(val1, val2), val3, xs...);
			}
		};
	};
	namespace numeric {
		template<class T>
		T rand(T r1, T r2) requires (cyh::type::is_numeric_v<T>) {
			if (r1 == r2) return r1;
			return static_cast<T>(cyh::details::get_rand_in_range(static_cast<double>(r2), static_cast<double>(r1)));
		}
		template<class T, class V>
		constexpr bool overflow(V val) requires(std::is_integral_v<T>) {
			return static_cast<double>(val) > cyh::details::max_d<T>();
		}
		template<class T, class V>
		constexpr bool underflow(V val) requires(std::is_integral_v<T>) {
			return static_cast<double>(val) < cyh::details::min_d<T>();
		}
		template<class T, class V>
		constexpr int compare(T val1, V val2) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<V>) {
			return cyh::details::num_compare::compare(val1, val2);
		}
		template<class T, class V>
		constexpr T SafeCast(V srcVal) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<V>) {
			if (cyh::numeric::compare(cyh::numeric::max<T>(), srcVal) == -1) {
				return cyh::numeric::max<T>();
			} else if (cyh::numeric::compare(cyh::numeric::min<T>(), srcVal) == 1) {
				return cyh::numeric::min<T>();
			} else {
				return static_cast<T>(srcVal);
			}
		}
		template<class T, class U, class V, class ...Xs>
		constexpr T max(T val1, U val2, V val3, Xs... xs) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>&& cyh::type::is_numeric_v<V> && (cyh::type::is_numeric_v<Xs> && ...)) {
			return cyh::details::num_compare::max_of(val1, val2, val3, xs...);
		}
		template<class T, class U, class V, class ...Xs>
		constexpr T min(T val1, U val2, V val3, Xs... xs) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U>&& cyh::type::is_numeric_v<V> && (cyh::type::is_numeric_v<Xs> && ...)) {
			return cyh::details::num_compare::max_of(val1, val2, val3, xs...);
		}
		template<class T, class U, class ...Xs>
		constexpr T sum(T val1, U val2, Xs... xs) requires(cyh::type::is_numeric_v<T>&& cyh::type::is_numeric_v<U> && (cyh::type::is_numeric_v<Xs> && ...)) {
			if (cyh::details::contains_type_t<float>() || cyh::details::contains_type_t<double>()) {
				return static_cast<T>(static_cast<double>(val1) + static_cast<double>(val2) + (static_cast<double>(xs) + ...));
			} else {
				return static_cast<T>(static_cast<int64>(val1) + static_cast<int64>(val2) + (static_cast<int64>(xs) + ...));
			}
		}
		template<class T>
		constexpr T get_inrange(T value, T r1, T r2) {
			if (r1 == r2) return r1;
			if (r1 < r2) {
				return cyh::details::get_inrange_min_max(value, r1, r2);
			} else {
				return cyh::details::get_inrange_min_max(value, r2, r1);
			}
		}
		template<class T, class It>
		T sum(It begin, It end) requires(cyh::type::is_numeric_v<T>) {
			T sum = T(0);
			for (auto it = begin; it != end; ++it) {
				sum += *it;
			}
			return sum;
		}
		int64 from_hex_string(const std::string_view& _hexStr);
		std::string to_hex_string(int64 value);
	};
};
