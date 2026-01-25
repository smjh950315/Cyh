#pragma once
#include <cyh/typedef.hpp>
#include <iostream>
#include <vector>
namespace cyh::text::impl {
	// sizeof(T), size_t*, size_t*, size_t*, T*, size_t
	extern int trim_front_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength);
	// sizeof(T), size_t*, size_t*, size_t*, T*, size_t
	extern int trim_back_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength);
	// sizeof(T), size_t*, size_t*, T*, size_t
	extern int trim_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength);
	// sizeof(T), size_t*, T*, size_t*, T*, size_t
	extern int count_of(size_t typeSize, size_t* pcount, const void* input, size_t inputLength, const void* pat, size_t patLength);
	// sizeof(T), vector<string_view<T>>*, T*, size_t, T*, size_t
	extern int splits_v(size_t typeSize, void* presult, const void* input, size_t inputLength, const void* pat, size_t patLength);
	// sizeof(T), T*, size_t
	extern int to_lowercase(size_t typeSize, void* data, size_t length);
	// sizeof(T), T*, size_t
	extern int to_uppercase(size_t typeSize, void* data, size_t length);
	// sizeof(T), bool*, T*, size_t, T*, size_t
	extern int contain_only(size_t typeSize, bool* result, const void* input, size_t inputLength, const void* pattern, size_t patternLength);
	// sizeof(T), string<T>, T*, size_t, T*, size_t
	extern int replace(size_t typeSize, void* output, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength);
	// sizeof(T), string<T>, string<T>, T*, size_t, T*, size_t
	extern int replace_all(size_t typeSize, void* output, void* src, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength);
	// sizeof(T), string<T>, T*, size_t, T*, size_t, size_t
	extern int pad_start(size_t typeSize, void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength);
	// sizeof(T), string<T>, T*, size_t, T*, size_t, size_t
	extern int pad_end(size_t typeSize, void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength);
	// sizeof(T), int*, T*, size_t, T*, size_t
	extern int compare(size_t typeSize, int* presult, const void* input1, size_t inputLength1, const void* input2, size_t inputLength2);
	// outputLength max = 1024
	extern int format_numstring(size_t typeSize, char* output, size_t* outputLength, const char* src, size_t srcLength, const char* fmt, bool truncate_begin, bool truncate_end);
};
namespace cyh::text::impl::utf {
	extern int calc_utf_cvt_maxlength(size_t typeSizeTo, size_t* outputLength, bool _isWinWcharO,
									  size_t typeSizeFrom, size_t inputLength, bool _isWinWcharI,
									  void* idata, void* pMetaData);
	extern int utf_encode(size_t typeSizeTo, void* outputBuffer, size_t* outputLength, bool _isWinWcharO,
						  size_t typeSizeFrom, const void* inputBuffer, size_t inputLength, bool _isWinWcharI, 
						  void* pMetaData);
};
namespace cyh::text::impl::base64 {
	extern int get_base64_encoded_length(size_t* outputLength, size_t inputLength);
	extern int get_base64_decoded_length(size_t* outputLength, size_t inputLength);
	extern int base64_encode(void* outputBuffer, size_t* outputLength,
							 const void* inputBuffer, size_t inputLength);
	extern int base64_decode(void* outputBuffer, size_t* outputLength,
							 const void* inputBuffer, size_t inputLength);
};
namespace cyh::text::details {
	template<class TChar, STRD T>
	inline std::basic_string<TChar> to_u_string_from_xstr(const T& src) {
		using element_type = cyh::trait::elem_type<T>;
		std::basic_string_view<element_type> sv{ src };
		size_t srcLen = sv.length();
		if (srcLen == 0)
			return {};
		size_t maxLen;
		std::u8string metaData;
		int validCode = cyh::text::impl::utf::calc_utf_cvt_maxlength(sizeof(TChar), &maxLen, std::is_same_v<wchar_t, TChar>, 
																	 sizeof(element_type), srcLen, std::is_same_v<wchar_t, element_type>,
																	 (void*)sv.data(), &metaData);
		if (validCode == -1) 
			return {};
		std::basic_string<TChar> result;
		result.resize(maxLen);
		size_t outLen;
		validCode = cyh::text::impl::utf::utf_encode(sizeof(TChar), result.data(), &outLen, std::is_same_v<wchar_t, TChar>,
													 sizeof(element_type), sv.data(), srcLen, std::is_same_v<wchar_t, element_type>,
													 &metaData);
		if (validCode == -1) 
			return {};
		result.resize(outLen);
		return result;
	}
	template<class char_type = char>
	struct parser_impl_ {
		using m_string = std::basic_string<char_type>;
		template<class T>
		struct to_string_base {
			static bool append(m_string& str, T&& value) {
				str.append("[unknow format(");
				str.append(typeid(std::decay_t<T>).name());
				str.append(")]");
				return false;
			}
		};
		template<class T, bool _bool_constant = cyh::type::is_add_equatable_v<m_string, T>>
		struct str_addequatable : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str += std::forward<T>(value); }
		};
		template<class T, bool _bool_constant = cyh::type::is_std_to_string_convertable_v<T>>
		struct str_std_to_stringable : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str += std::to_string(std::forward<T>(value)); }
		};
		template<class T, bool _bool_constant = cyh::type::is_to_string_convertable_v<T>>
		struct str_self_to_stringable : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str += value.to_string(); }
		};
		template<class T, bool _bool_constant = cyh::type::is_addable_v<m_string, T>>
		struct str_addable : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str = str + std::forward<T>(value); }
		};
		template<class T, bool _bool_constant = std::is_assignable_v<m_string, T>>
		struct str_assignable : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str += std::forward<T>(value); }
		};
		template<class T, bool _bool_constant = cyh::type::is_stdstring_data_v<T>>
		struct str_diffchartype : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) { str += cyh::text::details::to_u_string_from_xstr<char_type>(value); }
		};
		template<class T, bool _bool_constant = cyh::type::is_char_v<T>>
		struct str_single_char : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) {
				using _char_type = std::decay_t<T>;
				_char_type tmp[2] = {};
				tmp[0] = value;				
				std::basic_string_view<_char_type> sv{ ((const _char_type*)tmp), size_t(1) };
				to_string_t(str, sv);
			}
		};
		template<class T, bool _bool_constant = cyh::type::is_char_pointer_v<T>>
		struct str_charptr : to_string_base<T> {
			static constexpr void append(m_string& str, T&& value) requires(_bool_constant) {
				using _char_type = std::remove_cvref_t<std::remove_pointer_t<std::decay_t<T>>>;
				std::basic_string_view<_char_type> sv{ ((const _char_type*)value), xstrlen(value) };
				to_string_t(str, sv);
			}
		};

		template<class T>
		static constexpr void to_string_t(m_string& str, T&& value) {
			if constexpr (cyh::type::is_char_v<T>) { // char
				str_single_char<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_char_pointer_v<T>) { // const ?* chptr
				str_charptr<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_to_string_convertable_v<T>) { // value.to_string()
				str_self_to_stringable<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_std_to_string_convertable_v<T>) { // std::to_string(value)
				str_std_to_stringable<T>::append(str, std::forward<T>(value));
			} else if constexpr (std::is_assignable_v<m_string, T>) { // m_string str = value
				str_assignable<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_add_equatable_v<m_string, T>) { // m_string value
				str_addequatable<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_addable_v<m_string, T>) { // m_string str += value
				str_addable<T>::append(str, std::forward<T>(value));
			} else if constexpr (cyh::type::is_stdstring_data_v<T>) { // std::basic_string<?> std::basic_string_view<?>
				str_diffchartype<T>::append(str, std::forward<T>(value));
			} else { // unknown
				to_string_base<T>::append(str, std::forward<T>(value));
			}
		}
		template<class T>
		static constexpr void to_string_t(m_string& str, T&& value) requires(std::is_same_v<bool, std::decay_t<T>>) {
			if (value) {
				return to_string_t(str, "true");
			} else {
				return to_string_t(str, "false");
			}
		}
	};
	template<class T, class _Sep>
	void concat_by_impl_(std::basic_string<T>& dst, _Sep&& _sep) {}
	template<class T, class _Sep, class V, class... _Args>
	void concat_by_impl_(std::basic_string<T>& dst, _Sep&& _sep, V&& _src, _Args&& ... _args) {
		parser_impl_<T>::to_string_t(dst, std::forward<_Sep>(_sep));
		parser_impl_<T>::to_string_t(dst, std::forward<V>(_src));
		concat_by_impl_<T>(dst, std::forward<_Sep>(_sep), std::forward<_Args>(_args)...);
	}
};
namespace cyh::text {
	template<class char_type = char>
	class parser {
	public:
		using m_string = std::basic_string<char_type>;
		template<class T>
		static m_string to_string(T&& value) requires(!std::is_same_v<m_string, std::decay_t<T>>) {
			m_string str{};
			cyh::text::details::parser_impl_<char_type>::to_string_t(str, std::forward<T>(value));
			return str;
		}
		template<class T>
		static m_string to_string(T&& value) requires(std::is_same_v<m_string, std::decay_t<T>>) { return value; }
		template<class T>
		static m_string to_string(const T* cstr) requires (std::is_same_v<char_type, std::decay_t<T>>) { return cstr; }
	};
	template<class TChar, class T>
	inline std::basic_string<TChar> to_u_string(const T& src) requires(!cyh::type::is_pure_num_v<T>) {
		return parser<TChar>::to_string(src);
	}
	template<class TChar, class T>
	inline std::basic_string<TChar> to_u_string(const T& src) requires(cyh::type::is_pure_num_v<T>) {
		auto _str = std::to_string(src);
		return parser<TChar>::to_string(_str);
	}
};
namespace cyh::text::details {
	template<class T, class FnCalcLen, class FnCode>
	inline int callback_base64_(T* poutputBuffer, FnCalcLen calcLen, FnCode code, const void* src, size_t srcLength) {
		if (poutputBuffer == nullptr || src == nullptr) return -1;
		if (srcLength == 0) return -1;
		size_t outLen;
		if (calcLen(&outLen, srcLength) == -1) return -1;
		T& result = *((T*)poutputBuffer);
		size_t origSize = result.size();
		result.resize(origSize + outLen + 1);
		if (code(result.data() + origSize, &outLen, src, srcLength) == -1) return -1;
		if (outLen == std::string::npos) {
			result.resize(origSize);
			return -1;
		}
		result.resize(outLen + origSize);
		return 0;
	}
};
namespace cyh::text {
	template<STR T>
	inline T trim_front(const T& src) {
		if (src.empty()) return {};
		size_t begIdx, totLen;
		cyh::text::impl::trim_front_info(sizeof(cyh::trait::elem_type<T>), &begIdx, &totLen, src.data(), src.length());
		return src.substr(begIdx, totLen);
	}
	template<STR T>
	inline T trim_back(const T& src) {
		if (src.empty()) return {};
		size_t begIdx, totLen;
		cyh::text::impl::trim_back_info(sizeof(cyh::trait::elem_type<T>), &begIdx, &totLen, src.data(), src.length());
		return src.substr(begIdx, totLen);
	}
	template<STR T>
	inline T trim(const T& src) {
		size_t begIdx, totLen;
		cyh::text::impl::trim_info(sizeof(cyh::trait::elem_type<T>), &begIdx, &totLen, src.data(), src.length());
		return src.substr(begIdx, totLen);
	}
	template<STRD T, STRD V>
	inline size_t count_of(const T& src, const V& pattern) {
		size_t count;
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "count_of only support char type string_view");
		auto s = std::basic_string_view<elem_type>{ src };
		auto p = std::basic_string_view<elem_type>{ pattern };
		return -1 != cyh::text::impl::count_of(sizeof(cyh::trait::elem_type<T>), &count, s.data(), s.length(), p.data(), p.length())
			? count : std::string::npos;
	}
	template<STR T, STRD U, STRD V>
	inline void splits(std::vector<T>* dst, const U& src, const V& pattern) requires(cyh::type::is_specialization_v<T, std::basic_string_view>) {
		if (!dst) return;
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "splits only support char type string_view");
		auto s = std::basic_string_view<elem_type>{ src };
		auto p = std::basic_string_view<elem_type>{ pattern };
		cyh::text::impl::splits_v(sizeof(cyh::trait::elem_type<T>), dst, s.data(), s.length(), p.data(), p.length());
	}
	template<STR T, STRD U, STRD V>
	inline void splits(std::vector<T>* dst, const U& src, const V& pattern) requires(cyh::type::is_specialization_v<T, std::basic_string>) {
		if (!dst) return;
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "splits only support char type string_view");
		std::vector<std::basic_string_view<elem_type>> _dst;
		splits(&_dst, src, pattern);
		if (_dst.size() == 0) return;
		dst->reserve(_dst.size());
		for (auto& item : _dst) {
			dst->emplace_back(std::basic_string<elem_type>{item.data(), item.length()});
		}
	}
	template<class T, class _Sep, class V, class... _Args>
	std::basic_string<T> concat_by(_Sep&& _sep, V&& _first, _Args&& ... _args) {
		std::basic_string<T> result;
		cyh::text::details::parser_impl_<T>::to_string_t(result, std::forward<V>(_first));
		cyh::text::details::concat_by_impl_<T>(result, std::forward<_Sep>(_sep), std::forward<_Args>(_args)...);
		return result;
	}
	template<class T, class It, class _Sep>
	std::basic_string<T> concat_iterable_by(_Sep&& _sep, It begin, It end) requires(cyh::type::is_iterator_v<It>) {
		std::basic_string<T> result;
		if (begin == end) return result;
		using parser_type = cyh::text::details::parser_impl_<T>;
		parser_type::to_string_t(result, *begin);
		for (++begin; begin != end; ++begin) {
			parser_type::to_string_t(result, std::forward<_Sep>(_sep));
			parser_type::to_string_t(result, *begin);
		}
		return result;
	}
	template<STR T>
	inline void to_upper_case(T* data) {
		if (data == nullptr) return;
		cyh::text::impl::to_uppercase(sizeof(cyh::trait::elem_type<T>), data->data(), data->length());
	}
	template<STR T>
	inline void to_lower_case(T* data) {
		if (data == nullptr) return;
		cyh::text::impl::to_lowercase(sizeof(cyh::trait::elem_type<T>), data->data(), data->length());
	}
	template<STRD T, STRD U>
	inline bool contains(const T& src, const U& pattern) {
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "contains only support char type string_view");
		auto s = std::basic_string_view<elem_type>{ src };
		auto p = std::basic_string_view<elem_type>{ pattern };
		return s.find(p) != std::string::npos;
	}
	template<STRD T, STRD U>
	inline bool contains_only(const T& src, const U& pattern) {
		bool result;
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "contains_only only support char type string_view");
		auto s = std::basic_string_view<elem_type>{ src };
		auto p = std::basic_string_view<elem_type>{ pattern };
		cyh::text::impl::contain_only(sizeof(elem_type), &result, s.data(), s.length(), p.data(), p.length());
		return result;
	}

	template<class T, STRD U, STRD V>
	inline int replace(std::basic_string<T>* target, const U& pattern, const V& replacement) {
		auto p = std::basic_string_view<T>{ pattern };
		auto r = std::basic_string_view<T>{ replacement };
		if (target == nullptr || pattern.empty()) return -1;
		return cyh::text::impl::replace(sizeof(T), target, p.data(), p.length(), r.data(), r.length());
	}
	template<class T, STRD U, STRD V>
	inline int replace_all(std::basic_string<T>* dst, std::basic_string<T>* src, const U& pattern, const V& replacement) {
		auto p = std::basic_string_view<T>{ pattern };
		auto r = std::basic_string_view<T>{ replacement };
		if (dst == nullptr || src == nullptr || pattern.empty()) return -1;
		return cyh::text::impl::replace_all(sizeof(T), dst, src, p.data(), p.length(), r.data(), r.length());
	}
	template<class T, STRD U, STRD V>
	inline int replace_all(std::basic_string<T>* dst, const U& pattern, const V& replacement) {
		std::basic_string<T> temp;
		auto p = std::basic_string_view<T>{ pattern };
		auto r = std::basic_string_view<T>{ replacement };
		if (dst == nullptr || pattern.empty()) return -1;
		if (0 == cyh::text::impl::replace_all(sizeof(T), &temp, dst, p.data(), p.length(), r.data(), r.length())) {
			*dst = std::move(temp); return 0;
		} else {
			return -1;
		}
	}

	template<class T, STRD U, STRD V>
	inline int pad_start(std::basic_string<T>* dst, const U& src, const V& padContent, size_t totLength) {
		auto s = std::basic_string_view<T>{ src };
		auto p = std::basic_string_view<T>{ padContent };
		if (dst == nullptr || padContent.empty()) return -1;
		return cyh::text::impl::pad_start(sizeof(T), dst, s.data(), s.length(), p.data(), p.length(), totLength);
	}
	template<class T, STRD U, STRD V>
	inline int pad_end(std::basic_string<T>* dst, const U& src, const V& padContent, size_t totLength) {
		auto s = std::basic_string_view<T>{ src };
		auto p = std::basic_string_view<T>{ padContent };
		if (dst == nullptr || padContent.empty()) return -1;
		return cyh::text::impl::pad_end(sizeof(T), dst, s.data(), s.length(), p.data(), p.length(), totLength);
	}

	template<STRD T, STRD U>
	inline int compare(const T& lhs, const U& rhs) {
		int result;
		using elem_type = cyh::trait::elem_type<T>;
		static_assert(cyh::type::is_char_v<elem_type>, "count_of only support char type string_view");
		auto l = std::basic_string_view<elem_type>{ lhs };
		auto r = std::basic_string_view<elem_type>{ rhs };
		cyh::text::impl::compare(sizeof(elem_type), &result, l.data(), l.length(), r.data(), r.length());
		return result;
	}
}
namespace cyh {
	template<class T, class char_type = char>
	std::basic_string<char_type> to_string_(T&& val) {
		return cyh::text::parser<char_type>::to_string(std::forward<T>(val));
	}

	template<class T>
	inline std::string to_string(const T& src, const char* fmt, bool truncate_beg = false, bool truncate_end = true) requires(cyh::type::is_pure_num_v<T>) {
		std::string numstr = std::to_string(src);
		if (fmt == nullptr) return numstr;
		char buffer[1024]{}; size_t outLen;
		return 0 == cyh::text::impl::format_numstring(sizeof(T), buffer, &outLen, numstr.data(), numstr.length(), fmt, truncate_beg, truncate_end) ?
			 std::string{ buffer, outLen } : numstr;
	}
	template<class T>
	inline std::string to_string(const T& src) {
		return cyh::text::to_u_string<char>(src);
	}
	template<class T>
	inline std::u8string to_u8string(const T& src) {
		return cyh::text::to_u_string<char8_t>(src);
	}
	template<class T>
	inline std::u16string to_u16string(const T& src) {
		return cyh::text::to_u_string<char16_t>(src);
	}
	template<class T>
	inline std::u32string to_u32string(const T& src) {
		return cyh::text::to_u_string<char32_t>(src);
	}
	template<class T>
	int to_base64(T* poutput, const byte* src, size_t srcLength) {
		return cyh::text::details::callback_base64_(poutput, cyh::text::impl::base64::get_base64_encoded_length, cyh::text::impl::base64::base64_encode, src, srcLength);
	}
	template<class T>
	int from_base64(T* poutput, const char* src, size_t srcLength) {
		return cyh::text::details::callback_base64_(poutput, cyh::text::impl::base64::get_base64_decoded_length, cyh::text::impl::base64::base64_decode, src, srcLength);
	}

	template<class DstChar, class SrcCharBuff>
	std::basic_string<DstChar>& operator += (std::basic_string<DstChar>& lhs, const SrcCharBuff& rhs)
		requires(std::is_class_v<std::decay_t<SrcCharBuff>>&& cyh::type::is_char_v<DstChar>&& cyh::type::is_const_char_buffer_v<SrcCharBuff>&& cyh::type::is_diff_const_char_buffer_v<SrcCharBuff, DstChar>) {
		lhs += cyh::text::to_u_string<DstChar>(std::basic_string_view<cyh::trait::elem_type<SrcCharBuff>>{rhs.data(), rhs.length()});
		return lhs;
	}

	template<class DstChar, class SrcChar>
	std::basic_string<DstChar>& operator += (std::basic_string<DstChar>& lhs, const SrcChar* rhs)
		requires(cyh::type::is_char_v<DstChar>&& cyh::type::is_char_v<SrcChar> && !std::is_same_v<std::decay_t<DstChar>, std::decay_t<SrcChar>>) {
		lhs += cyh::text::to_u_string<DstChar>(std::basic_string_view<SrcChar>{rhs, cyh::xstrlen(rhs)});
		return lhs;
	}
}
