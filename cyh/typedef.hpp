#pragma once
#include <string>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <cmath>
#include <cstdint>
#include <climits>
#include <cstring>
#ifdef _MSVC_LANG
#pragma warning (push)
#pragma warning (disable: 4068)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // __GNUC__

#if UINTPTR_MAX != UINT_MAX
#define PLATFORM_64BIT
#else
#define PLATFORM_32BIT
#endif
#if defined(_WIN32) || defined(_WIN64)
#define __WINDOWS__
#elif defined(__linux__) || defined(__unix__)
#define __UNIX__
#endif
// this function shold be checked before call
#define __UNCHECKED__ 
// set up a functional pointer from extern func
#define SET_CALLBACK_API(_def, _impl) decltype(_def) _def = _impl
#define __CALLBACK__(_def, _impl) decltype(_def) _def = _impl
namespace cyh {

#if defined (__FILE__) && defined (__LINE__)
	namespace details {
		template<class T = void>
		std::string dbg_GetCodePositionString(const char* file, int line) {
			std::string retVal = "File: ";
			retVal += file;
			retVal += ", ";
			retVal += "Line: ";
			retVal += std::to_string(line);
			return retVal;
		}
	};
#define __POSITION__ cyh::details::dbg_GetCodePositionString( __FILE__ , __LINE__ )
#else
#define __POSITION__ std::string{}
#endif
	using pvoid = void*;
	using type_info = std::type_info;

	using uchar = std::make_unsigned_t<char>;
	using ushort = std::make_unsigned_t<short>;
	using uint = std::make_unsigned_t<int>;
	using ullong = std::make_unsigned_t<long long>;

	using nuint = size_t;
	using nint = std::make_signed_t<size_t>;
	using signed_t = std::make_signed_t<size_t>;
	using ulong = unsigned long;

	using uintptr = std::uintptr_t;
	using intptr = std::intptr_t;
	using byte = std::uint8_t;
	using int8 = std::int8_t;
	using uint8 = std::uint8_t;
	using int16 = std::int16_t;
	using uint16 = std::uint16_t;
	using int32 = std::int32_t;
	using uint32 = std::uint32_t;
	using int64 = std::int64_t;
	using uint64 = std::uint64_t;
	using f32 = float;
	using f64 = double;

	using ch8 = char8_t;
	using ch16 = char16_t;
	using ch32 = char32_t;

	class Environment {
	private:
		static constexpr std::uint32_t uint32_ = 0x01234567;
		static constexpr std::uint8_t cs_lowAddressValue = (const uint8_t&)uint32_;
		static constexpr std::uint8_t cs_lowByte = 0x67;
		static constexpr std::uint8_t cs_heiByte = 0x01;
	public:
		static constexpr bool is_big_endian = cs_heiByte == cs_lowAddressValue;
		static constexpr bool is_little_endian = cs_lowByte == cs_lowAddressValue;
		static constexpr bool is_64bit = UINTPTR_MAX != UINT_MAX;
		static constexpr bool is_32bit = UINTPTR_MAX == UINT_MAX;
	};

	namespace type {
		template <typename T, typename... Ts>
		inline constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

		template <typename T>
		inline constexpr bool is_char_v = is_any_of_v<std::decay_t<T>, char, uchar, ch8, ch16, ch32, wchar_t>;
	};

	namespace details {

		template<typename T>
		struct member_class_type;

		template<typename ClassType, class MemberType>
		struct member_class_type<MemberType ClassType::*> {
			using type = ClassType;
			using self = MemberType;
			using self_ref = self&;
			using self_ptr = self*;
		};

		template<typename T>
		struct elem_type_ {
			using type = std::decay_t<decltype(*std::declval<T>().begin())>;
		};
		template<typename T>
			requires(std::is_pointer_v<std::decay_t<T>>)
		struct elem_type_<T> {
			using type = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;
		};
		template<typename T>
			requires(!std::is_pointer_v<std::decay_t<T>>)
		struct elem_type_<T> {
			using type = std::decay_t<decltype(*std::declval<T>().begin())>;
		};

		template<typename T, typename ... Args>
		struct callableby_ {
			template<typename U, typename ... TArgs>
			static auto test(U u, TArgs ...args) -> decltype(u(std::forward<TArgs>(args)...), std::true_type{});
			template<typename ...>
			static auto test() -> std::false_type {};
			static constexpr bool value = !std::is_same_v<decltype(test(std::declval<T>(), std::declval<Args>()...)), std::false_type>;
		};

		template<typename T, typename ... Args> requires(callableby_<T, Args...>::value)
			struct functor_type_ {
			using return_type = decltype(std::declval<T>()(std::forward<Args>(std::declval<Args>())...));
		};

		template<class T>
		struct array_length_
		{
			static constexpr size_t total_size = sizeof(T);
			static constexpr size_t first_size = sizeof(std::declval<T>()[0]);
			static constexpr size_t element_count = total_size / first_size;
		};
	};
	namespace trait {
		template<class T>
		using elem_type = details::elem_type_<T>::type;
		template<class MemberPtrType>
		using class_of_member = typename cyh::details::member_class_type<std::decay_t<decltype(std::declval<MemberPtrType>())>>::type;
		template<class MemberPtrType>
		using class_member_type = typename cyh::details::member_class_type<std::decay_t<decltype(std::declval<MemberPtrType>())>>::self;
		template<class TFunc, class...Args>
		using func_return_type = typename cyh::details::functor_type_<TFunc, Args...>::return_type;
	};
	namespace details {
		struct null_reference {
			template<class _Any>
			bool operator == (_Any&& _any) requires(std::is_pointer_v<std::decay_t<_Any>>) {
				return nullptr == (void*)(_any);
			}
			template<class _Any>
			operator _Any() const requires(std::is_pointer_v<std::decay_t<_Any>>) {
				return (_Any)nullptr;
			}
		};

		template <class T, class U>
		struct is_addable {
			template <class X, class Y>
			static auto test(X x, Y y) -> decltype(x + y);
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>(), std::declval<U>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T, class U>
		struct is_add_equatable {
			template <class X, class Y>
			static auto test(X x, Y y) -> decltype(x += y);
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>(), std::declval<U>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T, class CharType>
		struct is_printable {
			template <class X>
			static auto test(std::basic_ostream<CharType>& os, X x) -> decltype(os << x);
			static std::false_type test(...);
			using type = decltype(test(std::declval<std::basic_ostream<CharType>&>(), std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T, class CharType>
		struct is_string_convertable {
			template <class X>
			static auto test(X x) -> decltype((std::basic_string<CharType>)x);
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T>
		struct is_to_string_convertable {
			template <class X>
			static auto test(X x) -> decltype(x.to_string());
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T>
		struct is_std_to_string_convertable {
			template <class X>
			static auto test(X x) -> decltype(std::to_string(x));
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};
		template<class T>
		struct is_address_available {
			template <class X>
			static auto test(X x) -> decltype(&x);
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template <class T, template <class...> class Template>
		struct is_specialization : std::false_type {};

		template <template <class...> class Template, class... Args>
		struct is_specialization<Template<Args...>, Template> : std::true_type {};

		template<class T>
		struct is_container {
			template <class X>
			static auto test(X x) -> decltype(std::begin(x), std::end(x), x.size(), std::true_type());
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T>
		struct is_std_buffer {
			template<typename U>
			static auto test(U u) -> decltype(u.size(), u.data(), u.begin(), u.end(), u.resize(0), std::true_type());
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T>
		struct is_const_std_buffer {
			template<typename U>
			static auto test(U u) -> decltype(u.size(), u.data(), u.begin(), u.end(), std::true_type());
			static std::false_type test(...);
			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_same_v<type, std::false_type>;
		};

		template<class T>
		struct is_string {
			static constexpr bool value = is_specialization<std::decay_t<T>, std::basic_string>::value || is_specialization<std::decay_t<T>, std::basic_string_view>::value;
		};

		template<class T>
		struct is_string_data {
			static constexpr bool value = is_string<T>::value || type::is_char_v<trait::elem_type<T>>;
		};

#ifdef _MSVC_LANG
#pragma warning (pop)
#endif

		template<class T, class...Ts>
		struct is_same_multiple : std::false_type {};

		template<class T>
		struct is_same_multiple<T> : std::true_type {};

		template<class T, class U>
		struct is_same_multiple<T, U> : std::is_same<T, U> {};

		template<class T, class U, class...V>
		struct is_same_multiple<T, U, V...> {
			static constexpr bool value = std::is_same_v<T, U>&& is_same_multiple<U, V...>::value;
		};

		template<class Base, class...Ts>
		struct is_base_multiple : std::false_type {};

		template<class Base>
		struct is_base_multiple<Base> : std::true_type {};

		template<class Base, class U>
		struct is_base_multiple<Base, U> : std::is_base_of<Base, U> {};

		template<class Base, class U, class...V>
		struct is_base_multiple<Base, U, V...> {
			static constexpr bool value = std::is_base_of_v<Base, U>&& is_base_multiple<Base, V...>::value;
		};

		template<class T, class...Ts>
		struct is_assignable_multiple : std::false_type {};

		template<class T>
		struct is_assignable_multiple<T> : std::true_type {};

		template<class T, class U>
		struct is_assignable_multiple<T, U> : std::is_base_of<T, U> {};

		template<class T, class U, class...V>
		struct is_assignable_multiple<T, U, V...> {
			static constexpr bool value = std::is_assignable_v<T, U>&& is_assignable_multiple<T, V...>::value;
		};

		template<class To, class From>
		struct is_assignable_tolvalref {
			template<class T, class F>
			static auto test(T& t, F f) -> decltype(t = f, std::true_type());
			template<class T, class F>
			static auto test(...) -> std::false_type {};
			static constexpr bool value = std::is_same_v<decltype(test<To, From>(std::declval<std::add_lvalue_reference_t<To>>(), std::declval<From>())), std::true_type>;
		};

		template<class T>
		struct is_default_validator {
			template<class X>
			static auto test(X x) -> decltype(std::decay_t<X>::is_default_value((void*)0), std::true_type{});
			static auto test(...) -> decltype(std::false_type{});
			static constexpr bool value = decltype(test(std::declval<T>()))::value;
		};

		template <typename T>
		struct is_iterator {
			template <typename U>
			static auto test(U&&) -> decltype(
				++std::declval<U>(),
				std::declval<U>()++,
				*std::declval<U>(),
				std::declval<U>() != std::declval<U>(),
				std::true_type{});

			static std::false_type test(...);

			using type = decltype(test(std::declval<T>()));
			static constexpr bool value = !std::is_void_v<type>;
		};
	};

	template<class T>
	concept STR = cyh::details::is_string<T>::value;

	template<class T>
	concept STRD = cyh::details::is_string_data<T>::value;

	static constexpr cyh::details::null_reference null{};

	namespace type {
		template <class T, template <class...> class Template>
		inline constexpr bool is_specialization_v = cyh::details::is_specialization<T, Template>::value;

		template <typename T>
		inline constexpr bool is_primitive_type_v = !std::is_class_v<std::decay_t<T>>;

		template<class T>
		inline constexpr bool is_container_v = cyh::details::is_container<T>::value;

		template<class T>
		inline constexpr bool is_numeric_v = std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>;

		template<class T>
		inline constexpr bool is_pure_num_v = is_any_of_v<std::decay_t<T>, short, ushort, int, uint, long, ulong, long long, ullong, float, double>;

		template<class T>
		inline constexpr bool is_std_buffer_v = cyh::details::is_std_buffer<T>::value;

		template<class T>
		inline constexpr bool is_const_std_buffer_v = cyh::details::is_const_std_buffer<T>::value;

		template<class T, class CharType = char>
		inline constexpr bool is_printable_v = cyh::details::is_printable<T, CharType>::value;

		template<class T, class U>
		inline constexpr bool is_addable_v = cyh::details::is_addable<T, U>::value;

		template<class T, class U>
		inline constexpr bool is_add_equatable_v = cyh::details::is_add_equatable<T, U>::value;

		// (string<CharType>)value
		template<class T, class CharType = char>
		inline constexpr bool is_string_convertable_v = cyh::details::is_string_convertable<T, CharType>::value;

		// value.to_string()
		template<class T>
		inline constexpr bool is_to_string_convertable_v = cyh::details::is_to_string_convertable<T>::value;

		// std::to_string(value)
		template<class T>
		inline constexpr bool is_std_to_string_convertable_v = cyh::details::is_std_to_string_convertable<T>::value;

		// string v = value;
		template<class T, class CharType = char>
		inline constexpr bool is_string_assignable_v = std::is_assignable_v<std::basic_string<CharType>, T>;

		template<class T, class ... Ts>
		inline constexpr bool is_same_multipule_v = cyh::details::is_same_multiple<T, Ts...>::value;

		template<class TBase, class ... Ts>
		inline constexpr bool is_base_multipule_v = cyh::details::is_base_multiple<TBase, Ts...>::value;

		template<class T, class ... Ts>
		inline constexpr bool is_assignable_multiple_v = cyh::details::is_assignable_multiple<T, Ts...>::value;

		template<class To, class From>
		inline constexpr bool is_assignable_tolvalref_v = cyh::details::is_assignable_tolvalref<To, From>::value;

		template<class _Buffer, class T>
		inline constexpr bool is_buffer_of_v = cyh::type::is_std_buffer_v<_Buffer> && std::is_same_v<std::decay_t<T>, std::decay_t<decltype(*std::declval<_Buffer>().begin())>>;

		template<class T>
		inline constexpr bool is_char_buffer_v = cyh::type::is_std_buffer_v<T> && cyh::type::is_char_v<std::decay_t<decltype(*std::declval<T>().begin())>>;

		template<class T>
		inline constexpr bool is_const_char_buffer_v = cyh::type::is_const_std_buffer_v<T> && cyh::type::is_char_v<std::decay_t<decltype(*std::declval<T>().begin())>>;

		template<class T>
		inline constexpr bool is_default_validator_v = cyh::details::is_default_validator<T>::value;

		template<class T>
		inline constexpr bool is_stdstring_v = is_specialization_v<T, std::basic_string>;

		template<class T>
		inline constexpr bool is_stdstring_view_v = is_specialization_v<T, std::basic_string_view>;

		template<class T>
		inline constexpr bool is_stdstring_data_v = cyh::details::is_string_data<T>::value;

		template<class T, class V>
		inline constexpr bool is_diff_const_char_buffer_v = cyh::type::is_const_char_buffer_v<T>
			&& !std::is_same_v<cyh::trait::elem_type<T>, std::decay_t<V>>;

		template<class T>
		inline constexpr bool is_char_pointer_v = std::is_pointer_v<std::decay_t<T>> && cyh::type::is_char_v<std::remove_pointer_t<std::decay_t<T>>>;

		template <typename T>
		inline constexpr bool is_iterator_v = cyh::details::is_iterator<T>::value;

		template<class T, typename ... Args>
		inline constexpr bool is_callableby_v = cyh::details::callableby_<T, Args...>::value;
	
		template<class T>
		inline constexpr size_t array_length_v = cyh::details::array_length_<T>::element_count;
	};

	template<class T>
	constexpr T* get_ptr(const T& const_ref) { return const_cast<T*>(&const_ref); }

	// get length of c-style string
	template<typename T>
	static constexpr size_t xstrlen(const T* _xstr, size_t _maxlen = ~size_t{}) requires(cyh::type::is_char_v<T>) {
		if (_xstr == nullptr)
			return 0;
		size_t retVal = 0;
		T terminator = static_cast<T>('\0');
		T* iter = (T*)_xstr;
		size_t len = 0;
		while ((*iter != terminator) && len++ < _maxlen) 
		{
			retVal++; iter++;
		}
		return retVal;
	}

	template<class T, size_t _Max = cyh::type::array_length_v<T>>
	void write_char_array(T& _arr, const char* _src) requires (std::is_bounded_array_v<T> && cyh::type::array_length_v<T> != 0)
	{
		strncpy((char*)_arr, _src, cyh::type::array_length_v<T> - 1);
		((char*)_arr)[_Max - 1] = '\0';
	}
};
