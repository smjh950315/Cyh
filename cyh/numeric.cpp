#include "numeric.hpp"
#include <cyh/text.hpp>
#include <random>
#define INVUI8 static_cast<unsigned char>(255)
#define UI8(x) static_cast<unsigned char>(x)
#define U8(x) static_cast<unsigned char>(x)
namespace cyh {

	static constexpr char hex_chars[] = "0123456789ABCDEFabcdef";
	static constexpr uchar hex_map[256]
	{
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//15
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//31
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//47
		UI8(0),UI8(1),UI8(2),UI8(3),UI8(4),UI8(5),UI8(6),UI8(7),UI8(8),UI8(9),INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//63
		INVUI8,U8(10),U8(11),U8(12),U8(13),U8(14),U8(15),INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//79
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//95
		INVUI8,U8(10),U8(11),U8(12),U8(13),U8(14),U8(15),INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,//111
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
		INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,INVUI8,
	};

	namespace details {
		static std::random_device gs_rand_device = {};
		double get_rand_in_range(double _range1, double _range2) {
			if (_range1 == _range2) return _range1;
			double min, max;
			min = _range1 < _range2 ? _range1 : _range2;
			max = _range1 > _range2 ? _range1 : _range2;
			std::mt19937 generator(gs_rand_device());
			std::uniform_real_distribution<double> uniNum(min, max);
			double rand = uniNum(generator);
			return rand;
		}
		int parse_hex_str(const char* str, size_t length, int64* result) {
			if (!str || length == 0) return -1;
			int64 value = 0;
			for (size_t i = 0; i < length; ++i) {
				uchar ch = hex_map[static_cast<uchar>(str[i])];
				if (ch == INVUI8) return -1;
				value = (value << 4) | ch;
			}
			*result = value;
			return 0;
		}
		int to_hex_str(uint64 value, char* str, size_t* iolength) {
			if (!str || !iolength) return -1;
			char* beg = str;
			char* end = str + *iolength;
			while (value > 0 && beg != end) {
				*beg++ = hex_chars[value & 0xF];
				value >>= 4;
			}
			std::reverse(str, beg);
			*iolength = beg - str;
			return 0;
		}
	}

	namespace numeric {
		int64 from_hex_string(const std::string_view& _hexStr) {
			int64 result = 0;
			details::parse_hex_str(_hexStr.data(), _hexStr.length(), &result);
			return result;
		}
		std::string to_hex_string(int64 value) {
			char buffer[32]{};
			size_t outLen = sizeof(buffer);
			if (details::to_hex_str((uint64)value, buffer, &outLen) != 0) {
				return {};
			}
			return std::string{ buffer, outLen };
		}
	};
};