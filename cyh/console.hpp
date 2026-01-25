#pragma once
#include "typedef.hpp"
#include "exceptions.hpp"
#include <iomanip>
#include <unordered_map>
#include <cyh/reflection.hpp>
#include <cyh/text.hpp>
namespace cyh {
	std::ostream& operator << (std::ostream& _cout, const ch8*);
	std::ostream& operator << (std::ostream& _cout, const ch16*);
	std::ostream& operator << (std::ostream& _cout, const ch32*);

	std::ostream& operator << (std::ostream& _cout, const std::u8string&);
	std::ostream& operator << (std::ostream& _cout, const std::u16string&);
	std::ostream& operator << (std::ostream& _cout, const std::u32string&);
};
namespace cyh::console {

	struct console_print_format {
		std::string seperator;
		uint row_items = ~uint{};
		uint item_width{};
	};

	namespace details {
		class print_utils {
#ifdef _MSVC_LANG
#pragma warning (push)
#pragma warning (disable: 4068)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // __GNUC__
			template<class T>
			struct is_self_printable {
				template<class P>
				static auto test(P x) -> decltype(std::declval<P>().print(), std::true_type{});
				static std::false_type test(...);
				static constexpr bool value = decltype(test(std::declval<T>()))::value;
			};

			template<class T>
			struct is_iterable {
				template<class P>
				static auto test(P x) -> decltype(std::declval<P>().begin(), std::declval<P>().end(), std::true_type{});
				static std::false_type test(...);
				static constexpr bool value = decltype(test(std::declval<T>()))::value;
			};

#ifdef _MSVC_LANG
#pragma warning (pop)
#endif

			template<class T>
			struct try_print_base {
				static constexpr void print(T&& _value) { std::cout << cyh::to_string(std::forward<T>(_value)); }
				static constexpr void print(T&& _value, uint _setw) { std::cout << std::setw(_setw) << cyh::to_string(std::forward<T>(_value)); }
			};
			template<class T, bool _bool_constant = is_self_printable<T>::value >
			struct self_print {
				static constexpr void print(T&& _value) requires(_bool_constant) { _value.print(); }
				static constexpr void print(T&& _value, uint _setw) requires(_bool_constant) { std::cout << std::setw(_setw) << ""; _value.print(); }
			};
			template<class T, bool _bool_constant = is_iterable<T>::value>
			struct print_iterable {
				static constexpr void print(T&& _value) requires(_bool_constant) {
					print_container(std::forward<T>(_value));
				}
				static constexpr void print(T&& _value, uint _setw) requires(_bool_constant) {
					print_container(std::forward<T>(_value), console_print_format{ .item_width = _setw });
				}
			};
			template<class T, class ostream_char_type, std::basic_ostream<ostream_char_type>& ostream_instance, bool _bool_constant = cyh::type::is_printable_v<T, ostream_char_type>>
			struct ostream_interfece_ : try_print_base<T> {
				static constexpr void print(T&& _value) requires(_bool_constant) { ostream_instance << _value; }
				static constexpr void print(T&& _value, uint _setw) requires(_bool_constant) { ostream_instance << std::setw(_setw) << _value; }
			};
			template<class T>
			static constexpr void print_single(T&& _value) {
				if constexpr (is_self_printable<T>::value) {
					self_print<T>::print(std::forward<T>(_value));
				} else if constexpr (cyh::type::is_printable_v<T, char>) {
					ostream_interfece_<T, char, std::cout>::print(std::forward<T>(_value));
				} else if constexpr (cyh::type::is_printable_v<T, wchar_t>) {
					ostream_interfece_<T, wchar_t, std::wcout>::print(std::forward<T>(_value));
				} else if constexpr (is_iterable<T>::value) {
					print_iterable<T>::print(std::forward<T>(_value));
				} else {
					try_print_base<T>::print(std::forward<T>(_value));
				}
			}
			template<class T>
			static constexpr void print_single(T&& _value, uint _setw) {
				if constexpr (is_self_printable<T>::value) {
					self_print<T>::print(std::forward<T>(_value), _setw);
				} else if constexpr (cyh::type::is_printable_v<T, char>) {
					ostream_interfece_<T, char, std::cout>::print(std::forward<T>(_value), _setw);
				} else if constexpr (cyh::type::is_printable_v<T, wchar_t>) {
					ostream_interfece_<T, wchar_t, std::wcout>::print(std::forward<T>(_value), _setw);
				} else if constexpr (is_iterable<T>::value) {
					print_iterable<T>::print(std::forward<T>(_value), _setw);
				} else {
					try_print_base<T>::print(std::forward<T>(_value), _setw);
				}
			}

			static constexpr void print() {}
			static constexpr void print_width(uint width) {}

			template<class Prefix>
			static constexpr void print_with_prefix_(const Prefix& prefix) {}
			template<class Prefix, class T, class ... U>
			static constexpr void print_with_prefix_(const Prefix& prefix, T&& _first, U&&..._others) {
				print_single(prefix);
				print_single(std::forward<T>(_first));
				print_with_prefix_(prefix, std::forward<U>(_others)...);
			}
			template<class Sep, class T, class ... U>
			static constexpr void print_with_seperator(const Sep& seperator, T&& _first, U&&..._others) {
				print_single(std::forward<T>(_first));
				print_with_prefix_(seperator, std::forward<U>(_others)...);
			}

			static void print_format_(uint width, uint max_per_row, uint current_index, const char* sep) {}
			template<class T, class ... U>
			static void print_format_(uint width, uint max_per_row, uint current_index, const char* sep, T&& _first, U&&..._others) {
				if (width == 0) {
					print_single(std::forward<T>(_first));
				} else {
					print_single(std::forward<T>(_first), width);
				}
				if (current_index + 1 == max_per_row) {
					std::cout << std::endl;
					print_format_(width, max_per_row, 0, sep, std::forward<U>(_others)...);
				} else {
					if (sep) { std::cout << sep; }
					print_format_(width, max_per_row, current_index + 1, sep, std::forward<U>(_others)...);
				}
			}
			static void print_format_iter_(uint width, uint max_per_row, uint current_index, const char* sep) {}
			template<class Iterator>
			static void print_format_iter_(uint width, uint max_per_row, uint current_index, const char* sep, Iterator&& _begin, Iterator&& end) {
				if (_begin == end) { return; }
				if (width == 0) {
					print_single(*_begin);
				} else {
					print_single(*_begin, width);
				}
				if (current_index + 1 == max_per_row) {
					std::cout << std::endl;
					print_format_iter_(width, max_per_row, 0, sep, ++_begin, end);
				} else {
					if (sep) { std::cout << sep; }
					print_format_iter_(width, max_per_row, current_index + 1, sep, ++_begin, end);
				}
			}
		public:
			template<class T, class ... U>
			static constexpr void print(T&& _first, U&&..._others) {
				print_single(std::forward<T>(_first));
				print(std::forward<U>(_others)...);
			}

			template<class T, class ... U>
			static constexpr void print_width(uint width, T&& _first, U&&..._others) {
				print_single(std::forward<T>(_first), width);
				print_width(width, std::forward<U>(_others)...);
			}

			template<class T, class ... U>
			static constexpr void print_line(T&& _first, U&&..._others) {
				print(std::forward<T>(_first), std::forward<U>(_others)...);
				std::cout << std::endl;
			}
			template<class T, class ... U>
			static void print_format(const console_print_format& format, T&& _first, U&&..._others) {
				if (format.row_items == 0) { throw cyh::exception::invalid_argument_exception("Item count in a row shouldn't be zero!"); }
				print_format_(format.item_width, format.row_items, 0, format.seperator.c_str(), std::forward<T>(_first), std::forward<U>(_others)...);
			}

			template<class TContainer>
			static void print_container(TContainer&& _container, const console_print_format& format = {}) {
				if (format.row_items == 0) { throw cyh::exception::invalid_argument_exception("Item count in a row shouldn't be zero!"); }
				print_format_iter_(format.item_width, format.row_items, 0, format.seperator.c_str(), _container.begin(), _container.end());
			}
		};
	};

	template<class T, class ... U>
	constexpr void print(T&& _first, U&&..._others) {
		cyh::console::details::print_utils::print(std::forward<T>(_first), std::forward<U>(_others)...);
	}
	template<class T, class ... U>
	constexpr void println(T&& _first, U&&..._others) {
		cyh::console::details::print_utils::print_line(std::forward<T>(_first), std::forward<U>(_others)...);
	}
	template<class T, class ... U>
	constexpr void print_format(const console_print_format& format, T&& _first, U&&..._others) {
		cyh::console::details::print_utils::print_format(format, std::forward<T>(_first), std::forward<U>(_others)...);
	}

	void execute(const char* _command, std::string* recv = nullptr);
	std::string console_path();
	std::string readline();
	size_t read_args(std::unordered_map<std::string, std::vector<std::string>>& arg_result, const char* _prefix, int argc, const char** argv);
	std::unordered_map<std::string, std::vector<std::string>> read_args(const char* _prefix, int argc, const char** argv);
	template<class T>
	void read_args(T& arg_model, const char* _prefix, int argc, const char** argv) {
		auto arg_map = read_args(_prefix, argc, argv);
		std::unordered_map<std::string, std::any> arg_map_any;
		for (auto& [key, value] : arg_map) {
			arg_map_any[key] = value;
		}
		cyh::reflection::try_set_members(arg_model, arg_map_any, true);
	}
	template<class T>
	T read_args(const char* _prefix, int argc, const char** argv) {
		T result{};
		read_args(result, _prefix, argc, argv);
		return result;
	}
};

