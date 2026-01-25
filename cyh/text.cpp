#include "text.hpp"
#include <cstring>
#define __CALLBACK_BY_SIZE(fn, sz, ...)\
switch (sz)\
{\
	case 1:\
		return fn <char> ( __VA_ARGS__ );\
	case 2:\
		return fn <char16_t> (__VA_ARGS__);\
	case 4:\
		return fn <char32_t> (__VA_ARGS__);\
	default:\
		return -1;\
}

namespace cyh::text::data {
#define UI8(x) static_cast<std::uint8_t>( x )
	static constexpr std::uint8_t uppercase_map[]{
		UI8(0u),UI8(1u),UI8(2u),UI8(3u),UI8(4u),UI8(5u),UI8(6u),UI8(7u),
		UI8(8u),UI8(9u),UI8(10u),UI8(11u),UI8(12u),UI8(13u),UI8(14u),UI8(15u),
		UI8(16u),UI8(17u),UI8(18u),UI8(19u),UI8(20u),UI8(21u),UI8(22u),UI8(23u),
		UI8(24u),UI8(25u),UI8(26u),UI8(27u),UI8(28u),UI8(29u),UI8(30u),UI8(31u),
		UI8(32u),UI8(33u),UI8(34u),UI8(35u),UI8(36u),UI8(37u),UI8(38u),UI8(39u),
		UI8(40u),UI8(41u),UI8(42u),UI8(43u),UI8(44u),UI8(45u),UI8(46u),UI8(47u),
		UI8(48u),UI8(49u),UI8(50u),UI8(51u),UI8(52u),UI8(53u),UI8(54u),UI8(55u),
		UI8(56u),UI8(57u),UI8(58u),UI8(59u),UI8(60u),UI8(61u),UI8(62u),UI8(63u),
		UI8(64u),UI8(65u),UI8(66u),UI8(67u),UI8(68u),UI8(69u),UI8(70u),UI8(71u),
		UI8(72u),UI8(73u),UI8(74u),UI8(75u),UI8(76u),UI8(77u),UI8(78u),UI8(79u),
		UI8(80u),UI8(81u),UI8(82u),UI8(83u),UI8(84u),UI8(85u),UI8(86u),UI8(87u),
		UI8(88u),UI8(89u),UI8(90u),UI8(91u),UI8(92u),UI8(93u),UI8(94u),UI8(95u),
		UI8(96u),UI8(65u),UI8(66u),UI8(67u),UI8(68u),UI8(69u),UI8(70u),UI8(71u),
		UI8(72u),UI8(73u),UI8(74u),UI8(75u),UI8(76u),UI8(77u),UI8(78u),UI8(79u),
		UI8(80u),UI8(81u),UI8(82u),UI8(83u),UI8(84u),UI8(85u),UI8(86u),UI8(87u),
		UI8(88u),UI8(89u),UI8(90u),UI8(123u),UI8(124u),UI8(125u),UI8(126u),UI8(127u),
		UI8(128u),UI8(129u),UI8(130u),UI8(131u),UI8(132u),UI8(133u),UI8(134u),UI8(135u),
		UI8(136u),UI8(137u),UI8(138u),UI8(139u),UI8(140u),UI8(141u),UI8(142u),UI8(143u),
		UI8(144u),UI8(145u),UI8(146u),UI8(147u),UI8(148u),UI8(149u),UI8(150u),UI8(151u),
		UI8(152u),UI8(153u),UI8(154u),UI8(155u),UI8(156u),UI8(157u),UI8(158u),UI8(159u),
		UI8(160u),UI8(161u),UI8(162u),UI8(163u),UI8(164u),UI8(165u),UI8(166u),UI8(167u),
		UI8(168u),UI8(169u),UI8(170u),UI8(171u),UI8(172u),UI8(173u),UI8(174u),UI8(175u),
		UI8(176u),UI8(177u),UI8(178u),UI8(179u),UI8(180u),UI8(181u),UI8(182u),UI8(183u),
		UI8(184u),UI8(185u),UI8(186u),UI8(187u),UI8(188u),UI8(189u),UI8(190u),UI8(191u),
		UI8(192u),UI8(193u),UI8(194u),UI8(195u),UI8(196u),UI8(197u),UI8(198u),UI8(199u),
		UI8(200u),UI8(201u),UI8(202u),UI8(203u),UI8(204u),UI8(205u),UI8(206u),UI8(207u),
		UI8(208u),UI8(209u),UI8(210u),UI8(211u),UI8(212u),UI8(213u),UI8(214u),UI8(215u),
		UI8(216u),UI8(217u),UI8(218u),UI8(219u),UI8(220u),UI8(221u),UI8(222u),UI8(223u),
		UI8(224u),UI8(225u),UI8(226u),UI8(227u),UI8(228u),UI8(229u),UI8(230u),UI8(231u),
		UI8(232u),UI8(233u),UI8(234u),UI8(235u),UI8(236u),UI8(237u),UI8(238u),UI8(239u),
		UI8(240u),UI8(241u),UI8(242u),UI8(243u),UI8(244u),UI8(245u),UI8(246u),UI8(247u),
		UI8(248u),UI8(249u),UI8(250u),UI8(251u),UI8(252u),UI8(253u),UI8(254u),UI8(255u)
	};
	static constexpr std::uint8_t lowercase_map[]{
		UI8(0u),UI8(1u),UI8(2u),UI8(3u),UI8(4u),UI8(5u),UI8(6u),UI8(7u),
		UI8(8u),UI8(9u),UI8(10u),UI8(11u),UI8(12u),UI8(13u),UI8(14u),UI8(15u),
		UI8(16u),UI8(17u),UI8(18u),UI8(19u),UI8(20u),UI8(21u),UI8(22u),UI8(23u),
		UI8(24u),UI8(25u),UI8(26u),UI8(27u),UI8(28u),UI8(29u),UI8(30u),UI8(31u),
		UI8(32u),UI8(33u),UI8(34u),UI8(35u),UI8(36u),UI8(37u),UI8(38u),UI8(39u),
		UI8(40u),UI8(41u),UI8(42u),UI8(43u),UI8(44u),UI8(45u),UI8(46u),UI8(47u),
		UI8(48u),UI8(49u),UI8(50u),UI8(51u),UI8(52u),UI8(53u),UI8(54u),UI8(55u),
		UI8(56u),UI8(57u),UI8(58u),UI8(59u),UI8(60u),UI8(61u),UI8(62u),UI8(63u),
		UI8(64u),UI8(97u),UI8(98u),UI8(99u),UI8(100u),UI8(101u),UI8(102u),UI8(103u),
		UI8(104u),UI8(105u),UI8(106u),UI8(107u),UI8(108u),UI8(109u),UI8(110u),UI8(111u),
		UI8(112u),UI8(113u),UI8(114u),UI8(115u),UI8(116u),UI8(117u),UI8(118u),UI8(119u),
		UI8(120u),UI8(121u),UI8(122u),UI8(91u),UI8(92u),UI8(93u),UI8(94u),UI8(95u),
		UI8(96u),UI8(97u),UI8(98u),UI8(99u),UI8(100u),UI8(101u),UI8(102u),UI8(103u),
		UI8(104u),UI8(105u),UI8(106u),UI8(107u),UI8(108u),UI8(109u),UI8(110u),UI8(111u),
		UI8(112u),UI8(113u),UI8(114u),UI8(115u),UI8(116u),UI8(117u),UI8(118u),UI8(119u),
		UI8(120u),UI8(121u),UI8(122u),UI8(123u),UI8(124u),UI8(125u),UI8(126u),UI8(127u),
		UI8(128u),UI8(129u),UI8(130u),UI8(131u),UI8(132u),UI8(133u),UI8(134u),UI8(135u),
		UI8(136u),UI8(137u),UI8(138u),UI8(139u),UI8(140u),UI8(141u),UI8(142u),UI8(143u),
		UI8(144u),UI8(145u),UI8(146u),UI8(147u),UI8(148u),UI8(149u),UI8(150u),UI8(151u),
		UI8(152u),UI8(153u),UI8(154u),UI8(155u),UI8(156u),UI8(157u),UI8(158u),UI8(159u),
		UI8(160u),UI8(161u),UI8(162u),UI8(163u),UI8(164u),UI8(165u),UI8(166u),UI8(167u),
		UI8(168u),UI8(169u),UI8(170u),UI8(171u),UI8(172u),UI8(173u),UI8(174u),UI8(175u),
		UI8(176u),UI8(177u),UI8(178u),UI8(179u),UI8(180u),UI8(181u),UI8(182u),UI8(183u),
		UI8(184u),UI8(185u),UI8(186u),UI8(187u),UI8(188u),UI8(189u),UI8(190u),UI8(191u),
		UI8(192u),UI8(193u),UI8(194u),UI8(195u),UI8(196u),UI8(197u),UI8(198u),UI8(199u),
		UI8(200u),UI8(201u),UI8(202u),UI8(203u),UI8(204u),UI8(205u),UI8(206u),UI8(207u),
		UI8(208u),UI8(209u),UI8(210u),UI8(211u),UI8(212u),UI8(213u),UI8(214u),UI8(215u),
		UI8(216u),UI8(217u),UI8(218u),UI8(219u),UI8(220u),UI8(221u),UI8(222u),UI8(223u),
		UI8(224u),UI8(225u),UI8(226u),UI8(227u),UI8(228u),UI8(229u),UI8(230u),UI8(231u),
		UI8(232u),UI8(233u),UI8(234u),UI8(235u),UI8(236u),UI8(237u),UI8(238u),UI8(239u),
		UI8(240u),UI8(241u),UI8(242u),UI8(243u),UI8(244u),UI8(245u),UI8(246u),UI8(247u),
		UI8(248u),UI8(249u),UI8(250u),UI8(251u),UI8(252u),UI8(253u),UI8(254u),UI8(255u)
	};
}

namespace cyh::text::impl {
	template<class T>
	using strv_ = std::basic_string_view<T>;
	template<class T>
	using str_ = std::basic_string<T>;
	template<typename T>
	static constexpr size_t xstrlen(const T* _xstr) {
		if (_xstr == nullptr)
			return 0;
		size_t retVal = 0;
		T terminator = static_cast<T>('\0');
		T* iter = (T*)_xstr;
		while (*iter != terminator) {
			retVal++; iter++;
		}
		return retVal;
	}
	template<class T>
	constexpr T terminator() { return static_cast<T>('\0'); }
	template<class T>
	constexpr T whitespace() { return static_cast<T>(' '); }

	template<class T>
	int trim_front_(size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		T* _cstr = (T*)input;
		if (_cstr[0] == terminator<T>()) {
			*beginIndex = 0;
			*strLength = inputLength;
		}
		size_t currentIndex = 0;
		while (*_cstr++ == whitespace<T>() && currentIndex++ < inputLength);
		*beginIndex = currentIndex;
		*strLength = (inputLength - currentIndex);
		return 0;
	}
	template<class T>
	int trim_back_(size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		T* _cstr = (T*)input;
		T* _end = _cstr + inputLength;
		size_t currentIndex = inputLength;
		while (*--_end == whitespace<T>() && --currentIndex > 0);
		*beginIndex = 0;
		*strLength = currentIndex;
		return 0;
	}
	template<class T>
	int trim_(size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		trim_front_<T>(beginIndex, strLength, input, inputLength);
		if (strLength == 0) return 0;
		size_t begIdx2, strLen2;
		trim_back_<T>(&begIdx2, &strLen2, (T*)input + *beginIndex, *strLength);
		*beginIndex = (*beginIndex + begIdx2);
		*strLength = strLen2;
		return 0;
	}
	template<class T>
	int count_of_(size_t* pcount, const void* input, size_t inputLength, const void* pat, size_t patLength) {
		*pcount = 0;
		if (inputLength == 0 && patLength == 0) {
			*pcount = 1;
			return 0;
		}
		if (inputLength == 0 || patLength == 0) return 0;
		std::basic_string_view<T> _strv{ (T*)input, inputLength };
		std::basic_string_view<T> _patv{ (T*)pat, patLength };
		while (true) {
			size_t pos = _strv.find(_patv);
			if (pos == str_<T>::npos) break;
			++*pcount;
			_strv = _strv.substr(pos + patLength);
		}
		return 0;
	}
	template<class T>
	int splits_v_(void* presult, const void* input, size_t inputLength, const void* pat, size_t patLength) {
		if (inputLength == 0 && patLength == 0) return 0;
		if (inputLength == 0) return 0;
		std::basic_string_view<T> _strv{ (T*)input, inputLength };
		std::basic_string_view<T> _patv{ (T*)pat, patLength };
		std::vector<std::basic_string_view<T>>& _result = *((std::vector<std::basic_string_view<T>>*)presult);
		if (patLength == 0) {
			for(const auto& ch : _strv) {
				_result.push_back(strv_<T>{ &ch, 1 });
			}
			return 0;
		}
		while (true) {
			size_t pos = _strv.find(_patv);
			if (pos == str_<T>::npos) break;
			_result.push_back(_strv.substr(0, pos));
			_strv = _strv.substr(pos + patLength);
		}
		_result.push_back(_strv);
		return 0;
	}

	template<class T>
	int to_lowercase_(void* data, size_t length) {
		T* _data = (T*)data;
		for (size_t i = 0; i < length; ++i) {
			_data[i] = data::lowercase_map[static_cast<std::uint8_t>(_data[i])];
		}
		return 0;
	}
	template<class T>
	int to_uppercase_(void* data, size_t length) {
		T* _data = (T*)data;
		for (size_t i = 0; i < length; ++i) {
			_data[i] = data::uppercase_map[static_cast<std::uint8_t>(_data[i])];
		}
		return 0;
	}
	template<class T>
	int contain_only_(bool* result, const void* input, size_t inputLength, const void* pattern, size_t patternLength) {
		if (inputLength == 0 && patternLength == 0) {
			*result = true;
			return 0;
		}
		if (inputLength == 0 || patternLength == 0) {
			*result = false;
			return 0;
		}
		std::basic_string_view<T> _strv{ (T*)input, inputLength };
		std::basic_string_view<T> _patv{ (T*)pattern, patternLength };
		bool hasOther = false;
		while (true && _strv.length()) {
			size_t pos = _strv.find(_patv);
			hasOther = pos != 0;
			if (pos == str_<T>::npos || pos != 0) break;
			_strv = _strv.substr(pos + patternLength);
		}
		*result = !hasOther;
		return 0;
	}

	template<class T>
	int replace_(void* dst, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength) {
		str_<T>* _dst = (str_<T>*)dst;
		strv_<T> _pat = strv_<T>{ (T*)pattern, patternLength };
		size_t pos = _dst->find(_pat);
		if (pos == str_<T>::npos) return 0;
		_dst->replace(pos, patternLength, (T*)replacement, replacementLength);
		return 0;
	}
	template<class T>
	int replace_all_(void* dst, void* src, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength) {
		str_<T>* _dst = (str_<T>*)dst;
		str_<T>* _src = (str_<T>*)src;
		strv_<T> srcv = strv_<T>{ _src->data(), _src->length() };
		strv_<T> _pat = strv_<T>{ (T*)pattern, patternLength };
		strv_<T> _rep = strv_<T>{ (T*)replacement, replacementLength };
		size_t pos = srcv.find(_pat);
		while (pos != str_<T>::npos) {
			*_dst += srcv.substr(0, pos);
			*_dst += _rep;
			srcv = srcv.substr(pos + patternLength);
			pos = srcv.find(_pat);
		}
		*_dst += srcv;
		return 0;
	}
	template<class T>
	int pad_start_(void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength) {
		str_<T>* _dst = (str_<T>*)output;
		strv_<T> _src = strv_<T>{ (T*)input, inputLength };
		strv_<T> _pad = strv_<T>{ (T*)padContent, padContentLength };
		if (inputLength >= totLength) {
			_dst->append(_src.data(), inputLength);
		} else {
			size_t padLength = totLength - inputLength;
			while (padLength > padContentLength) {
				_dst->append(_pad.data(), padContentLength);
				padLength -= padContentLength;
			}
			_dst->append(_pad.data(), padLength);
			_dst->append(_src.data(), inputLength);
		}
		return 0;
	}
	template<class T>
	int pad_end_(void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength) {
		str_<T>* _dst = (str_<T>*)output;
		strv_<T> _src = strv_<T>{ (T*)input, inputLength };
		strv_<T> _pad = strv_<T>{ (T*)padContent, padContentLength };
		if (inputLength >= totLength) {
			_dst->append(_src.data(), inputLength);
		} else {
			_dst->append(_src.data(), inputLength);
			size_t padLength = totLength - inputLength;
			while (padLength > padContentLength) {
				_dst->append(_pad.data(), padContentLength);
				padLength -= padContentLength;
			}
			_dst->append(_pad.data(), padLength);
		}
		return 0;
	}

	template<class T>
	int compare_(int* result, const void* input1, size_t input1Length, const void* input2, size_t input2Length) {
		strv_<T> _str1{ (T*)input1, input1Length };
		strv_<T> _str2{ (T*)input2, input2Length };
		*result = _str1.compare(_str2);
		return 0;
	}

	int trim_front_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		__CALLBACK_BY_SIZE(trim_front_, typeSize, beginIndex, strLength, input, inputLength);
	}
	int trim_back_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		__CALLBACK_BY_SIZE(trim_back_, typeSize, beginIndex, strLength, input, inputLength);
	}
	int trim_info(size_t typeSize, size_t* beginIndex, size_t* strLength, const void* input, size_t inputLength) {
		__CALLBACK_BY_SIZE(trim_, typeSize, beginIndex, strLength, input, inputLength);
	}
	int count_of(size_t typeSize, size_t* pcount, const void* input, size_t inputLength, const void* pat, size_t patLength) {
		__CALLBACK_BY_SIZE(count_of_, typeSize, pcount, input, inputLength, pat, patLength);
	}
	int splits_v(size_t typeSize, void* presult, const void* input, size_t inputLength, const void* pat, size_t patLength) {
		__CALLBACK_BY_SIZE(splits_v_, typeSize, presult, input, inputLength, pat, patLength);
	}
	int contain_only(size_t typeSize, bool* result, const void* input, size_t inputLength, const void* pattern, size_t patternLength) {
		__CALLBACK_BY_SIZE(contain_only_, typeSize, result, input, inputLength, pattern, patternLength);
	}
	int to_lowercase(size_t typeSize, void* data, size_t length) {
		__CALLBACK_BY_SIZE(to_lowercase_, typeSize, data, length);
	}
	int to_uppercase(size_t typeSize, void* data, size_t length) {
		__CALLBACK_BY_SIZE(to_uppercase_, typeSize, data, length);
	}

	int replace(size_t typeSize, void* output, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength) {
		__CALLBACK_BY_SIZE(replace_, typeSize, output, pattern, patternLength, replacement, replacementLength);
	}
	int replace_all(size_t typeSize, void* output, void* src, const void* pattern, size_t patternLength, const void* replacement, size_t replacementLength) {
		__CALLBACK_BY_SIZE(replace_all_, typeSize, output, src, pattern, patternLength, replacement, replacementLength);
	}
	int pad_start(size_t typeSize, void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength) {
		__CALLBACK_BY_SIZE(pad_start_, typeSize, output, input, inputLength, padContent, padContentLength, totLength);
	}
	int pad_end(size_t typeSize, void* output, const void* input, size_t inputLength, const void* padContent, size_t padContentLength, size_t totLength) {
		__CALLBACK_BY_SIZE(pad_end_, typeSize, output, input, inputLength, padContent, padContentLength, totLength);
	}
	int compare(size_t typeSize, int* result, const void* input1, size_t input1Length, const void* input2, size_t input2Length) {
		__CALLBACK_BY_SIZE(compare_, typeSize, result, input1, input1Length, input2, input2Length);
	}

	static size_t count_zero_before(const char* src, size_t dot_pos) {
		if (dot_pos == 0) return 0;
		char* it_beg = (char*)src + dot_pos - 1;
		char* it_end = (char*)src - 1;
		size_t counter{};
		while (*it_beg == '0' && it_beg != it_end) {
			++counter;
			--it_beg;
		}
		return counter;
	}
	static size_t count_zero_after(const char* src, size_t srclen, size_t dot_pos) {
		if (dot_pos + 1 >= srclen) return 0;
		char* it_beg = (char*)src + dot_pos + 1;
		char* it_end = (char*)src + srclen;
		size_t counter{};
		while (*it_beg == '0' && it_beg != it_end) {
			++counter;
			++it_beg;
		}
		return counter;
	}
	int format_numstring(size_t typeSize, char* output, size_t* outputLength, const char* src, size_t srcLength, const char* fmt, bool truncate_begin, bool truncate_end) {
		strv_<char> _src{ src, srcLength };
		strv_<char> _fmt{ fmt, xstrlen(fmt) };

		size_t fmtPointPos = _fmt.find('.');
		size_t srcPointPos = _src.find('.');

		// check if the format is valid
		size_t fmtValidBegPos = _fmt.find('0');
		fmtValidBegPos = fmtValidBegPos < fmtPointPos ? fmtValidBegPos : fmtPointPos;
		if (fmtValidBegPos == strv_<char>::npos)
			return -1;

		size_t fmtLengthOfInt;
		size_t fmtLengthOfDec;
		if (fmtPointPos != strv_<char>::npos) {
			size_t zeroBefore = count_zero_before(fmt, fmtPointPos);
			size_t zeroAfter = count_zero_after(fmt, _fmt.length(), fmtPointPos);
			fmtLengthOfInt = zeroBefore;
			fmtLengthOfDec = zeroAfter;
		} else {
			fmtLengthOfInt = count_zero_after(fmt, _fmt.length(), fmtValidBegPos) + 1;
			fmtLengthOfDec = 0;
		}

		char* out_begin = output;
		char* out_current = output;
		char* fmt_current = (char*)fmt;
		char* fmt_end = fmt_current + _fmt.length();
		char* src_current = (char*)src;
		size_t srcLengthOfInt = srcPointPos == strv_<char>::npos ? _src.length() : srcPointPos;

		if (fmtValidBegPos > 0) {
			memcpy(out_current, fmt_current, fmtValidBegPos);
			out_current += fmtValidBegPos;
			fmt_current += fmtValidBegPos;
		}
		if (fmtLengthOfInt <= srcLengthOfInt) {
			if (truncate_begin) {
				if (fmtLengthOfInt == 0) {
					*out_current++ = '0';
				} else {
					size_t trunc_src_length = srcLengthOfInt - fmtLengthOfInt;
					memcpy(out_current, src_current + trunc_src_length, fmtLengthOfInt);
					out_current += fmtLengthOfInt;
				}
			} else {
				memcpy(out_current, src_current, srcLengthOfInt);
				out_current += srcLengthOfInt;
			}
		} else {
			size_t beg_fmt_length = fmtLengthOfInt - srcLengthOfInt;
			memcpy(out_current, fmt_current, beg_fmt_length);
			memcpy(out_current + beg_fmt_length, src_current, srcLengthOfInt);
			out_current += fmtLengthOfInt;
		}
		fmt_current += fmtLengthOfInt;
		src_current += srcLengthOfInt;

		size_t srcLengthOfDec = srcPointPos == strv_<char>::npos ? 0 : _src.length() - srcPointPos - 1;
		if (fmtLengthOfDec == 0 && truncate_end) {
			if (fmt_end > fmt_current && *fmt_current == '.') {
				++fmt_current;
				*out_current++ = '.';
				*out_current++ = '0';
			}				
			memcpy(out_current, fmt_current, fmt_end - fmt_current);
			out_current += (fmt_end - fmt_current);
			*outputLength = out_current - out_begin;
			return 0;
		}

		if (!(*out_current == '.'))
			*out_current++ = '.';
		if (*src_current == '.')
			++src_current;
		if (*fmt_current == '.')
			++fmt_current;

		if (fmtLengthOfDec <= srcLengthOfDec) {
			// fmt .00 src .5678			
			if (truncate_end) {
				memcpy(out_current, src_current, fmtLengthOfDec);
				out_current += fmtLengthOfDec;
			} else {
				memcpy(out_current, src_current, srcLengthOfDec);
				out_current += srcLengthOfDec;
			}
		} else {
			// fmt .0000 src .123
			memcpy(out_current, src_current, srcLengthOfDec);
			memcpy(out_current + srcLengthOfDec, fmt_current, fmtLengthOfDec - srcLengthOfDec);
			out_current += fmtLengthOfDec;
		}
		fmt_current += fmtLengthOfDec;
		memcpy(out_current, fmt_current, fmt_end - fmt_current);
		out_current += (fmt_end - fmt_current);
		*outputLength = out_current - out_begin;
		return 0;
	}
}

#include <simdutf.h>
#ifdef __WINDOWS__
#include <Windows.h>
#endif
namespace cyh {
	namespace char_def {
		using ch = char;
		using ch8 = char;
		using ch16 = char16_t;
		using ch32 = char32_t;
		using ui8 = std::uint8_t;
	};

	template<class T>
	static constexpr void copy_chars(void* dst, void* src, size_t length) {
		if (!dst || !src || !length) return;
		memcpy(dst, src, sizeof(T) * length);
	}

#define _SIMDUTF_CONVERT_FUNC_API(from,to)\
size_t Convert_Utf##from##_to_Utf##to (void* _dstBuffer, void* _src, size_t _inputLength)\
{ return simdutf::convert_utf##from##_to_utf##to##_with_errors((cyh::char_def::ch##from *)_src,_inputLength,(cyh::char_def::ch##to *)_dstBuffer).count; }
#define _SIMDUTF_CONVERT_FUNC_COPY(from,to)\
size_t Convert_Utf##from##_to_Utf##to (void* _dstBuffer, void* _src, size_t _inputLength)\
{ copy_chars<cyh::char_def::ch##from >(_dstBuffer, _src, _inputLength); return _inputLength; }
#define _SIMDUTF_CONVERT_FUNC_DECLARE(from, to) Convert_Utf##from##_to_Utf##to

	_SIMDUTF_CONVERT_FUNC_COPY(8, 8);
	_SIMDUTF_CONVERT_FUNC_API(8, 16);
	_SIMDUTF_CONVERT_FUNC_API(8, 32);
	_SIMDUTF_CONVERT_FUNC_API(16, 8);
	_SIMDUTF_CONVERT_FUNC_COPY(16, 16);
	_SIMDUTF_CONVERT_FUNC_API(16, 32);
	_SIMDUTF_CONVERT_FUNC_API(32, 8);
	_SIMDUTF_CONVERT_FUNC_API(32, 16);
	_SIMDUTF_CONVERT_FUNC_COPY(32, 32);

	namespace text::impl::utf {
		static constexpr size_t _INF = ~size_t();
		static constexpr size_t convert_ratio_map[5][5]{
			{_INF,_INF,_INF,_INF,_INF}, // 0
			{_INF,1,1,_INF,1}, // 1->0,1,2,3,4
			{_INF,3,1,_INF,1}, // 2->0,1,2,3,4
			{_INF,_INF,_INF,_INF,_INF}, // 3->0,1,2,3,4
			{_INF,4,3,_INF,1} // 4->0,1,2,3,4
		};
		// converted length (*) (buffer, buffer size, source data, source length)
		using FnConvert = size_t(*)(void*, void*, size_t);

		static constexpr size_t _impl_convert_nothing(void*, void*, size_t) { return 0; }

		static constexpr FnConvert UtfConverter[5][5] =
		{
			{
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing
			},
			{
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(8,8),
				_SIMDUTF_CONVERT_FUNC_DECLARE(8,16),
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(8,32)
			},
			{
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(16,8),
				_SIMDUTF_CONVERT_FUNC_DECLARE(16,16),
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(16,32)
			},
			{
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing,
				_impl_convert_nothing
			},
			{
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(32,8),
				_SIMDUTF_CONVERT_FUNC_DECLARE(32,16),
				_impl_convert_nothing,
				_SIMDUTF_CONVERT_FUNC_DECLARE(32,32)
			}
		};

		int calc_utf_cvt_maxlength(size_t typeSizeTo, size_t* outputLength, bool _isWinWcharO,
								   size_t typeSizeFrom, size_t inputLength, bool _isWinWcharI,
								   void* idata, void* pMetaData) {
#ifdef __WINDOWS__
			if ((_isWinWcharI || _isWinWcharO) && !(_isWinWcharI && _isWinWcharO) && idata) {
				if (_isWinWcharI) {
					// win wchar to utf8
					int size_needed = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)idata, static_cast<int>(inputLength), NULL, 0, NULL, NULL);
					if (size_needed <= 0) 
						return -1;
					// utf8 to target
					return calc_utf_cvt_maxlength(typeSizeTo, outputLength, false, 1, size_needed, false, idata, pMetaData);
				} else {				
					// target to utf8
					int size_needed = 0;
					auto _pmeta = (std::u8string*)pMetaData;
					switch (typeSizeFrom)
					{
						case 1: // utf8
						{
							// utf8 to win wchar
							size_needed = MultiByteToWideChar(CP_UTF8, 0, (char*)idata, static_cast<int>(inputLength), NULL, 0);
							break;
						}
						case 2: // utf16
						{
							auto utf8_buffer = cyh::text::to_u_string<char8_t>(std::u16string_view{ (char16_t*)idata, inputLength });
							// utf8 to win wchar
							size_needed = MultiByteToWideChar(CP_UTF8, 0, (char*)utf8_buffer.data(), static_cast<int>(utf8_buffer.length()), NULL, 0);
							if (_pmeta) {
								*_pmeta = std::move(utf8_buffer);
							}
							break;
						}
						case 4: // utf32
						{
							auto utf8_buffer = cyh::text::to_u_string<char8_t>(std::u32string_view{ (char32_t*)idata, inputLength });
							// utf8 to win wchar
							size_needed = MultiByteToWideChar(CP_UTF8, 0, (char*)utf8_buffer.data(), static_cast<int>(utf8_buffer.length()), NULL, 0);
							if (_pmeta) {
								*_pmeta = std::move(utf8_buffer);
							}
							break;
						}
						default:
							break;
					}
					if (size_needed <= 0)
						return -1;
					*outputLength = static_cast<size_t>(size_needed);
					return 0;
				}
			}
#endif
			if (convert_ratio_map[typeSizeFrom][typeSizeTo] == _INF) return -1;
			*outputLength = convert_ratio_map[typeSizeFrom][typeSizeTo] * inputLength;
			return 0;
		}
		int utf_encode(size_t typeSizeTo, void* outputBuffer, size_t* outputLength, bool _isWinWcharO, 
					   size_t typeSizeFrom, const void* inputBuffer, size_t inputLength, bool _isWinWcharI,
					   void* pMetaData) {
#ifdef __WINDOWS__
			if ((_isWinWcharI || _isWinWcharO) && !(_isWinWcharI && _isWinWcharO)) {
				if (_isWinWcharI) {
					// win wchar to utf8
					int size_needed = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)inputBuffer, static_cast<int>(inputLength), NULL, 0, NULL, NULL);
					std::u8string utf8_buffer;
					utf8_buffer.resize(size_needed);
					WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)inputBuffer, static_cast<int>(inputLength), (char*)utf8_buffer.data(), size_needed, NULL, NULL);
					// utf8 to target
					return utf_encode(typeSizeTo, outputBuffer, outputLength, false, 1, (void*)utf8_buffer.data(), utf8_buffer.length(), false, pMetaData);
				} else {					
					// target to utf8					
					std::u8string utf8_buffer;
					char* u8input{};
					int u8input_len{};
					if (typeSizeFrom == 1) {
						u8input = (char*)inputBuffer;
						u8input_len = static_cast<int>(inputLength);
					} else {
						auto _pmeta = (std::u8string*)pMetaData;
						if (!_pmeta) {
							utf8_buffer = typeSizeFrom == 2 ?
								cyh::text::to_u_string<char8_t>(std::u16string_view{ (char16_t*)inputBuffer, inputLength }) :
								cyh::text::to_u_string<char8_t>(std::u32string_view{ (char32_t*)inputBuffer, inputLength });
							u8input = (char*)utf8_buffer.data();
							u8input_len = static_cast<int>(utf8_buffer.length());
						} else {
							u8input = (char*)_pmeta->data();
							u8input_len = static_cast<int>(_pmeta->length());
						}
					}
					// utf8 to win wchar
					int size_needed = MultiByteToWideChar(CP_UTF8, 0, u8input, u8input_len, NULL, 0);
					MultiByteToWideChar(CP_UTF8, 0, u8input, u8input_len, (wchar_t*)outputBuffer, size_needed);
					*outputLength = static_cast<size_t>(size_needed);
					return 0;
				}
			}
#endif
			*outputLength = UtfConverter[typeSizeFrom][typeSizeTo](outputBuffer, (void*)inputBuffer, inputLength);
			return 0;
		}
	}
}
namespace cyh::text::impl::base64 {
	int get_base64_encoded_length(size_t* outputLength, size_t inputLength) {
		*outputLength = simdutf::base64_length_from_binary(inputLength);
		return 0;
	}
	int get_base64_decoded_length(size_t* outputLength, size_t inputLength) {
		if (inputLength % 4) {
			*outputLength = (inputLength + 3) / 4 * 3;
		} else {
			*outputLength = inputLength / 4 * 3;
		}
		return 0;
	}
	int base64_encode(void* outputBuffer, size_t* outputLength, const void* inputBuffer, size_t inputLength) {
		*outputLength = simdutf::binary_to_base64((const char*)inputBuffer, inputLength, (char*)outputBuffer);
		return 0;
	}
	int base64_decode(void* outputBuffer, size_t* outputLength, const void* inputBuffer, size_t inputLength) {
		*outputLength = simdutf::base64_to_binary((const char*)inputBuffer, inputLength, (char*)outputBuffer).count;
		return 0;
	}
};
