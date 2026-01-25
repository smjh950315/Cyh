#include "timespan.h"
#include "time_details.h"
#include <cyh/text.hpp>
using namespace cyh::time::details;
using cyh::time::TIME_UNIT;
namespace cyh {
	static inline timespan create_from_nanoseconds(double _nanoseconds) {
		return timespan{ .m_epoch_nano = static_cast<int64>(_nanoseconds) };
	}
	timespan timespan::from_seconds(double _seconds) {
		return create_from_nanoseconds(_seconds * nano_rate_from::seconds);
	}
	timespan timespan::from_minutes(double _minutes) {
		return create_from_nanoseconds(_minutes * nano_rate_from::minutes);
	}
	timespan timespan::from_hours(double _hours) {
		return create_from_nanoseconds(_hours * nano_rate_from::hours);
	}
	timespan timespan::from_days(double _days) {
		return create_from_nanoseconds(_days * nano_rate_from::days);
	}
	double timespan::microseconds() const {
		return	this->m_epoch_nano / nano_rate_from::micros;
	}
	double timespan::milliseconds() const {
		return this->m_epoch_nano / nano_rate_from::millis;
	}
	double timespan::seconds() const {
		return this->m_epoch_nano / nano_rate_from::seconds;
	}
	double timespan::minutes() const {
		return this->m_epoch_nano / nano_rate_from::minutes;
	}
	double timespan::hours() const {
		return this->m_epoch_nano / nano_rate_from::hours;
	}
	double timespan::days() const {
		return this->m_epoch_nano / nano_rate_from::days;
	}
	void timespan::add_seconds(double val) {
		this->m_epoch_nano += static_cast<int64>(val * nano_rate_from::seconds);
	}
	void timespan::add_minutes(double val) {
		this->m_epoch_nano += static_cast<int64>(val * nano_rate_from::minutes);
	}
	void timespan::add_hours(double val) {
		this->m_epoch_nano += static_cast<int64>(val * nano_rate_from::hours);
	}
	void timespan::add_days(double val) {
		this->m_epoch_nano += static_cast<int64>(val * nano_rate_from::days);
	}

	std::string timespan::to_string(cyh::time::TIME_UNIT _unit, const char* fmt) const {
		static constexpr auto max_rate_count = sizeof(nano_rate_from::nano_rates) / sizeof(nano_rate_from::nanos);
		double epoch_nano = static_cast<double>(this->m_epoch_nano);
		const char* unit_str = nullptr;
		size_t _unitInt = static_cast<size_t>(_unit);
		if (_unitInt >= max_rate_count)
			_unitInt = static_cast<size_t>(TIME_UNIT::TIME_UNIT_NANOSECOND);
		return cyh::to_string(epoch_nano / nano_rate_from::nano_rates[_unitInt], fmt) + " " + unit_cstr::units[_unitInt];
	}

	timespan& operator+=(timespan& lhs, const timespan& rhs) {
		lhs.m_epoch_nano += rhs.m_epoch_nano;
		return lhs;
	}
	timespan& operator-=(timespan& lhs, const timespan& rhs) {
		lhs.m_epoch_nano -= rhs.m_epoch_nano;
		return lhs;
	}
	timespan operator+(const timespan& lhs, const timespan& rhs) {
		timespan result = lhs;
		result += rhs;
		return result;
	}
	timespan operator-(const timespan& lhs, const timespan& rhs) {
		timespan result = lhs;
		result -= rhs;
		return result;
	}
};
