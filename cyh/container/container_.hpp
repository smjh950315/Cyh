#pragma once
#include <cyh/typedef.hpp>
#include <cyh/exceptions.hpp>
#include <functional>
#include <algorithm>
namespace std {
	template<template<class, class... > class TContainerDst, template<class, class... > class TContainerSrc, class T, class _Pred, class... Ts, class... Vs>
	void find_and_pushback(TContainerDst<T, Ts...>& _out, const TContainerSrc<T, Vs...>& _src, _Pred&& _pred) {
		auto _it_now = _src.begin();
		auto _it_end = _src.end();
		while (true) {
			_it_now = std::find_if(_it_now, _it_end, _pred);
			if (_it_now != _it_end) {
				_out.push_back(*_it_now);
				++_it_now;
			} else {
				break;
			}
		}
	}
}
namespace cyh::container {
	template<class T>
	using ContentType = T::value_type;

	template<class T>
	inline constexpr bool is_primitive_container_v = cyh::type::is_container_v<std::decay_t<T>> && cyh::type::is_primitive_type_v<std::decay_t<ContentType<T>>>;

	template<class T>
	inline constexpr bool is_num_container_v = cyh::type::is_container_v<std::decay_t<T>> && cyh::type::is_numeric_v<std::decay_t<ContentType<T>>>;

	namespace details
	{
		enum NUM_TYPE
		{
			TYPE_SZ_1 = 0b0001,
			TYPE_SZ_2 = 0b0010,
			TYPE_SZ_4 = 0b0100,
			TYPE_SZ_8 = 0b1000,
			NUM_INT8 = TYPE_SZ_1 | 1 << 4,
			NUM_INT16 = TYPE_SZ_2 | 1 << 4,
			NUM_INT32 = TYPE_SZ_4 | 1 << 4,
			NUM_INT64 = TYPE_SZ_8 | 1 << 4,
			NUM_FLOAT = TYPE_SZ_4 | 1 << 5,
			NUM_DOUBLE = TYPE_SZ_8 | 1 << 5
		};
		static constexpr bool is_float(NUM_TYPE num_type)
		{
			return (num_type & (1 << 5)) != 0;
		}
		static constexpr bool is_integer(NUM_TYPE num_type)
		{
			return !is_float(num_type);
		}
		static constexpr size_t type_size(NUM_TYPE num_type)
		{
			NUM_TYPE filtered = (NUM_TYPE)(num_type & 0b1111);
			switch (filtered)
			{
				case cyh::container::details::TYPE_SZ_1:
					return 1;
				case cyh::container::details::TYPE_SZ_2:
					return 2;
				case cyh::container::details::TYPE_SZ_4:
					return 4;
				case cyh::container::details::TYPE_SZ_8:
					return 8;
				default:
					return 0;
			}
		}
        template<class T>
        constexpr NUM_TYPE GetContainerTypeEnum() { throw cyh::exception::invalid_argument_exception(); }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<uint8>() { return NUM_INT8; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<uint16>() { return NUM_INT16; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<uint32>() { return NUM_INT32; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<uint64>() { return NUM_INT64; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<int8>() { return NUM_INT8; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<int16>() { return NUM_INT16; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<int32>() { return NUM_INT32; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<int64>() { return NUM_INT64; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<float>() { return NUM_FLOAT; }
        template<> constexpr NUM_TYPE GetContainerTypeEnum<double>() { return NUM_DOUBLE; }
	};
};
