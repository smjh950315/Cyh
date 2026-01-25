#pragma once
#include <cyh/typedef.hpp>
namespace cyh::time {
	enum class TIME_UNIT : size_t
	{
		TIME_UNIT_NANOSECOND,
		TIME_UNIT_MICROSECOND,
		TIME_UNIT_MILLISECOND,
		TIME_UNIT_SECOND,
		TIME_UNIT_MINUTE,
		TIME_UNIT_HOUR,
		TIME_UNIT_DAY
	};
	enum TIME_TYPE {
		TIME_TYPE_UNDEF = 0,
		TIME_TYPE_GMT = 1,
		TIME_TYPE_LOCAL = 2,
	};
	struct time {
		const int second;
		const int minute;
		const int hour;
	};
	struct date {
		const int day;
		const int month;
		const int year;
	};
};
