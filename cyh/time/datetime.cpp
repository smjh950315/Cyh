#include "datetime.hpp"
#include "time_details.h"
#include <cyh/memory_helper.hpp>
#include <cyh/text.hpp>
#include <chrono>
#include <cstring>
using tm = std::tm;
#define __FORCE_SET_CONST_VALUE_(val, _val) \
{\
	if (val != _val) { \
		using value_type = std::decay_t<decltype(val)>; \
		value_type* pval = (value_type*)(&val); \
		*pval = _val; \
	}\
}
namespace cyh {
	static constexpr const char* fmt1 = "0";
	static constexpr const char* fmt2 = "00";
	static constexpr const char* fmt3 = "000";
	static constexpr const char* fmt4 = "0000";
	static constexpr const char* fmts[5]{
		nullptr,
		fmt1,
		fmt2,
		fmt3,
		fmt4
	};
	static bool replace_on_existing(std::string& str, const std::string_view& pattern, const int& value, const char* additional = nullptr) {
		auto index = str.find(pattern);
		if (pattern.length() > 4 || pattern.length() == 0) { return false; }
		if (index != std::string::npos) {
			std::string to_inserted = cyh::to_string(value, fmts[pattern.length()]);
			if (additional)
				to_inserted += additional;
			str.replace(index, pattern.length(), to_inserted);
			return true;
		}
		return false;
	}

#define TRY_REPLACE_( c ) static bool try_replace_##c (std::string& result, const int dt, const char* additional = nullptr) { \
if ( result.find( #c#c#c#c ) != std::string::npos ) { return replace_on_existing(result, #c#c#c#c, dt, additional); } \
else if ( result.find( #c#c#c ) != std::string::npos ) { return replace_on_existing(result, #c#c#c, dt, additional); } \
else if ( result.find( #c#c ) != std::string::npos ) { return replace_on_existing(result, #c#c, dt, additional); } \
else { return replace_on_existing(result, #c, dt, additional); } \
}

	TRY_REPLACE_(y);
	TRY_REPLACE_(M);
	TRY_REPLACE_(d);
	TRY_REPLACE_(H);
	TRY_REPLACE_(h);
	TRY_REPLACE_(m);
	TRY_REPLACE_(s);

	static std::string parse_format(const datetime& dt, const std::string_view& format_str) {
		size_t length = format_str.length();
		std::string result = std::string(format_str);
		try_replace_y(result, dt.date.year);
		try_replace_M(result, dt.date.month);
		try_replace_d(result, dt.date.day);
		try_replace_H(result, dt.time.hour);
		bool is_12h_fmt = try_replace_h(result, dt.time.hour > 12 ? (dt.time.hour - 12) : dt.time.hour);
		try_replace_m(result, dt.time.minute);
		if (is_12h_fmt) {
			if (dt.time.hour > 12) {
				try_replace_s(result, dt.time.second, " pm");
			} else {
				try_replace_s(result, dt.time.second, " am");
			}
		} else {
			try_replace_s(result, dt.time.second);
		}
		return result;
	}

	__UNCHECKED__ static void read_from_tm(cyh::time::date* pdate, const std::tm* _ptm) {
		__FORCE_SET_CONST_VALUE_(pdate->year, _ptm->tm_year + 1900);
		__FORCE_SET_CONST_VALUE_(pdate->month, _ptm->tm_mon + 1);
		__FORCE_SET_CONST_VALUE_(pdate->day, _ptm->tm_mday);
	}
	__UNCHECKED__ static void read_from_tm(cyh::time::time* ptime, const std::tm* _ptm) {
		__FORCE_SET_CONST_VALUE_(ptime->hour, _ptm->tm_hour);
		__FORCE_SET_CONST_VALUE_(ptime->minute, _ptm->tm_min);
		__FORCE_SET_CONST_VALUE_(ptime->second, _ptm->tm_sec);
	}
};
namespace cyh::time::details {
	static constexpr long long ll_1e3 = 1000;
	static constexpr long long ll_1e6 = 1000000;
	static constexpr long long ll_1e9 = 1000000000;
};
using namespace cyh::time::details;
namespace cyh {
	static void write_datetime(const int64 _timestamp, datetime* pdt) {
		tm _tm{};
		c_time_callbacks::time_t__to__tm(_timestamp, &_tm, pdt->timetype);
		read_from_tm(&pdt->date, &_tm);
		read_from_tm(&pdt->time, &_tm);
		__FORCE_SET_CONST_VALUE_(pdt->timestamp, _timestamp);
		__FORCE_SET_CONST_VALUE_(pdt->epoch_nano, _timestamp * ll_1e9);
		__FORCE_SET_CONST_VALUE_(pdt->weekday, _tm.tm_wday + 1);
		__FORCE_SET_CONST_VALUE_(pdt->yearday, _tm.tm_yday + 1);
	}
	static void write_datetime(const std::tm* _ptm, datetime* pdt) {
		int64 timestamp{};
		c_time_callbacks::tm__to__time_t(_ptm, &timestamp, pdt->timetype);
		write_datetime(timestamp, pdt);
	}
	static void reset_datetime(datetime* pdt) {
		memset(pdt, 0, sizeof(datetime));
	}
	static void copy_datetime(datetime* lhs, const datetime* rhs) {
		if (lhs == rhs) return;
		memcpy(lhs, rhs, sizeof(datetime));
	}
	static inline ctimestamp get_ctimestamp_from_epoch_nano(int64 _nano) {
		auto epoch = _nano / ll_1e9;
		auto fraction = _nano % ll_1e9;
		return ctimestamp(epoch, fraction);
	}

	datetime datetime::now() {
		return ctimestamp::now().get_datetime(cyh::time::TIME_TYPE_LOCAL);
	}
	datetime datetime::from_struct_tm(void* _tmStruct, cyh::time::TIME_TYPE _type)
	{
		if (!_tmStruct)
			return {};
		datetime retVal{};
		*((int*)&retVal.timetype) = _type;
		write_datetime((tm*)_tmStruct, &retVal);
		return retVal;
	}
	bool datetime::parse(datetime& _out, const std::string_view& _text, const std::string_view& _format)
	{
		int year{}, month{}, day{}, hour{}, minute{}, second{};
		auto tbeg = _text.begin();
		auto tend = _text.end();
		auto fbeg = _format.begin();
		auto fend = _format.end();
		auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
		auto isPatternChar = [](char c)
			{
				return c == 'y' || c == 'M' || c == 'd' || c == 'H' || c == 'h' || c == 'm' || c == 's';
			};
		while (tbeg != tend && fbeg != fend) {
			auto tc = *tbeg;
			auto fc = *fbeg;
			if (tc == fc) {
				++tbeg;
				++fbeg;
			} else if (isPatternChar(fc)) {
				size_t pattern_len = 0;
				char pattern_char = fc;
				while (fbeg != fend && *fbeg == pattern_char) {
					++pattern_len;
					++fbeg;
				}
				std::string num_str;
				while (tbeg != tend && isDigit(*tbeg) && num_str.length() < pattern_len) {
					num_str += *tbeg;
					++tbeg;
				}
				if (num_str.length() != pattern_len) {
					return false;
				}
				int value = std::atoi(num_str.c_str());
				switch (pattern_char) {
				case 'y':
					year = value;
					break;
				case 'M':
					month = value;
					break;
				case 'd':
					day = value;
					break;
				case 'H':
				case 'h':
					hour = value;
					break;
				case 'm':
					minute = value;
					break;
				case 's':
					second = value;
					break;
				default:
					return false;
				}
			} else {
				return false;
			}
		}
		_out = datetime(year, month, day, hour, minute, second);
		return true;
	}
	datetime::datetime() {}
	datetime::datetime(const int64 _timestamp)
	{
		*this = ctimestamp(_timestamp).get_datetime(time::TIME_TYPE_GMT);
	}
	datetime::datetime(int year, int month, int day, int _timetype) : timetype(_timetype) {
		tm _tm{};
		_tm.tm_year = year - 1900;
		_tm.tm_mon = month - 1;
		_tm.tm_mday = day;
		write_datetime(&_tm, this);
	}
	datetime::datetime(int year, int month, int day, int hour, int minute, int second, int _timetype) : timetype(_timetype) {
		tm _tm{};
		_tm.tm_year = year - 1900;
		_tm.tm_mon = month - 1;
		_tm.tm_mday = day;
		_tm.tm_hour = hour;
		_tm.tm_min = minute;
		_tm.tm_sec = second;
		write_datetime(&_tm, this);
	}
	datetime::datetime(const cyh::time::date& _date, const cyh::time::time& _time) {
		tm _tm{};
		_tm.tm_year = _date.year - 1900;
		_tm.tm_mon = _date.month - 1;
		_tm.tm_mday = _date.day;
		_tm.tm_hour = _time.hour;
		_tm.tm_min = _time.minute;
		_tm.tm_sec = _time.second;
		write_datetime(&_tm, this);
	}
	std::string datetime::to_string(const std::string_view& _format) const {
		return parse_format(*this, _format);
	}
	datetime::datetime(const datetime& dt) {
		copy_datetime(this, &dt);
	}
	datetime::datetime(datetime&& dt) noexcept {
		copy_datetime(this, &dt);
		reset_datetime(&dt);
	}
	datetime& datetime::operator=(const datetime& dt) {
		copy_datetime(this, &dt);
		return *this;
	}
	datetime& datetime::operator=(datetime&& dt) noexcept {
		copy_datetime(this, &dt);
		reset_datetime(&dt);
		return *this;
	}
	datetime datetime::get_timetype_of(cyh::time::TIME_TYPE _type) const {
		if (this->timetype == _type)
			return *this;
		return get_ctimestamp_from_epoch_nano(this->epoch_nano).get_datetime(_type);
	}
	bool datetime::write_tm_struct(void* _tmStruct) const
	{
		if (!_tmStruct || this->date.year < 1900 || this->date.month < 1)
			return false;
		tm* _ptm = (tm*)_tmStruct;
		auto& _tm = *_ptm;
		_tm.tm_year = this->date.year - 1900;
		_tm.tm_mon = this->date.month - 1;
		_tm.tm_mday = this->date.day;
		_tm.tm_hour = this->time.hour;
		_tm.tm_min = this->time.minute;
		_tm.tm_sec = this->time.second;
		return true;
	}
	bool operator==(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() == rhs.get_epoch_nano();
	}
	bool operator!=(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() != rhs.get_epoch_nano();
	}
	bool operator<(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() < rhs.get_epoch_nano();
	}
	bool operator<=(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() <= rhs.get_epoch_nano();
	}
	bool operator>(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() > rhs.get_epoch_nano();
	}
	bool operator>=(const ctimestamp& lhs, const ctimestamp& rhs) {
		return lhs.get_epoch_nano() >= rhs.get_epoch_nano();
	}
	timespan operator-(const ctimestamp& lhs, const ctimestamp& rhs) {
		return timespan{ .m_epoch_nano = lhs.get_epoch_nano() - rhs.get_epoch_nano() };
	}
	timespan operator-(const datetime& lhs, const datetime& rhs) {
		return timespan{ .m_epoch_nano = lhs.epoch_nano - rhs.epoch_nano };
	}

	datetime operator+(const datetime& lhs, const timespan& rhs) {
		return get_ctimestamp_from_epoch_nano(lhs.epoch_nano + rhs.m_epoch_nano).get_datetime((cyh::time::TIME_TYPE)lhs.timetype);
	}
	datetime operator-(const datetime& lhs, const timespan& rhs) {
		return get_ctimestamp_from_epoch_nano(lhs.epoch_nano - rhs.m_epoch_nano).get_datetime((cyh::time::TIME_TYPE)lhs.timetype);
	}

#define __RETURN_CONVERTED_TS_(timestamp) \
tm _tm{}; \
c_time_callbacks::time_t__to__tm(timestamp, &_tm, cyh::time::TIME_TYPE::TIME_TYPE_GMT); \
return _tm

	ctimestamp ctimestamp::now() {
		ctimestamp _timestamp{};
		c_time_callbacks::get__time_t_now(&_timestamp.timestamp, &_timestamp.fraction);
		return _timestamp;
	}
	ctimestamp::ctimestamp(int64_t _timestamp) : timestamp(_timestamp) {}
	ctimestamp::ctimestamp(int64_t _timestamp, int64 _fraction) : timestamp(_timestamp), fraction(_fraction) {}
	ctimestamp::ctimestamp(int year, int month, int day, cyh::time::TIME_TYPE _type) {
		tm _tm{};
		_tm.tm_year = year - 1900;
		_tm.tm_mon = month - 1;
		_tm.tm_mday = day;
		c_time_callbacks::tm__to__time_t(&_tm, &this->timestamp, _type);
	}
	ctimestamp::ctimestamp(int year, int month, int day, int hour, int minute, int second, cyh::time::TIME_TYPE _type) {
		tm _tm{};
		_tm.tm_year = year - 1900;
		_tm.tm_mon = month - 1;
		_tm.tm_mday = day;
		_tm.tm_hour = hour;
		_tm.tm_min = minute;
		_tm.tm_sec = second;
		c_time_callbacks::tm__to__time_t(&_tm, &this->timestamp, _type);
	}
	ctimestamp::ctimestamp(const ctimestamp& dt) {
		this->timestamp = dt.timestamp;
		this->fraction = dt.fraction;
	}
	ctimestamp::ctimestamp(ctimestamp&& dt) noexcept {
		this->timestamp = dt.timestamp;
		this->fraction = dt.fraction;
		dt.timestamp = 0;
		dt.fraction = 0;
	}
	ctimestamp& ctimestamp::operator=(const ctimestamp& dt) {
		if (this == &dt) return *this;
		this->timestamp = dt.timestamp;
		this->fraction = dt.fraction;
		return *this;
	}
	ctimestamp& ctimestamp::operator=(ctimestamp&& dt) noexcept {
		if (this == &dt) return *this;
		this->timestamp = dt.timestamp;
		this->fraction = dt.fraction;
		dt.timestamp = 0;
		dt.fraction = 0;
		return *this;
	}

	int ctimestamp::get_year() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_year + 1900;
	}
	int ctimestamp::get_month() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_mon + 1;
	}
	int ctimestamp::get_day() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_mday;
	}
	int ctimestamp::get_hour() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_hour;
	}
	int ctimestamp::get_minute() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_min;
	}
	int ctimestamp::get_second() const {
		__RETURN_CONVERTED_TS_(this->timestamp).tm_sec;
	}
	int ctimestamp::get_millisecond() const {
		return static_cast<int>(this->fraction / ll_1e6);
	}
	int64 ctimestamp::get_epoch_nano() const {
		return this->timestamp * ll_1e9 + this->fraction;
	}
	cyh::time::date ctimestamp::get_date(cyh::time::TIME_TYPE timeType) const {
		tm _tm{};
		c_time_callbacks::time_t__to__tm(this->timestamp, &_tm, timeType);
		return cyh::time::date{ .day = _tm.tm_mday, .month = _tm.tm_mon + 1, .year = _tm.tm_year + 1900 };
	}
	cyh::time::time ctimestamp::get_time(cyh::time::TIME_TYPE timeType) const {
		tm _tm{};
		c_time_callbacks::time_t__to__tm(this->timestamp, &_tm, timeType);
		return cyh::time::time{ .second = _tm.tm_sec, .minute = _tm.tm_min, .hour = _tm.tm_hour };
	}
	datetime ctimestamp::get_datetime(cyh::time::TIME_TYPE timeType) const {
		tm _tm{};
		c_time_callbacks::time_t__to__tm(this->timestamp, &_tm, timeType);
		cyh::time::date d{ .day = _tm.tm_mday, .month = _tm.tm_mon + 1, .year = _tm.tm_year + 1900 };
		cyh::time::time t{ .second = _tm.tm_sec, .minute = _tm.tm_min, .hour = _tm.tm_hour };
		datetime dt{};
		memcpy(&dt.date, &d, sizeof(cyh::time::date));
		memcpy(&dt.time, &t, sizeof(cyh::time::time));
		__FORCE_SET_CONST_VALUE_(dt.timestamp, this->timestamp);
		__FORCE_SET_CONST_VALUE_(dt.millisecond, this->get_millisecond());
		__FORCE_SET_CONST_VALUE_(dt.epoch_nano, this->get_epoch_nano());
		__FORCE_SET_CONST_VALUE_(dt.timetype, timeType);
		__FORCE_SET_CONST_VALUE_(dt.weekday, _tm.tm_wday + 1);
		__FORCE_SET_CONST_VALUE_(dt.yearday, _tm.tm_yday + 1);
		return dt;
	}
};
