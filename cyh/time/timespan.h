#pragma once
#include "time_.hpp"
namespace cyh {
	struct timespan {
		int64 m_epoch_nano{};
		static timespan from_seconds(double _seconds);
		static timespan from_minutes(double _minutes);
		static timespan from_hours(double _hours);
		static timespan from_days(double _days);
		double microseconds() const;
		double milliseconds() const;
		double seconds() const;
		double minutes() const;
		double hours() const;
		double days() const;
		void add_seconds(double);
		void add_minutes(double);
		void add_hours(double);
		void add_days(double);
		std::string to_string(cyh::time::TIME_UNIT _unit, const char* fmt = "0.00") const;
	};
	timespan& operator+=(timespan& lhs, const timespan& rhs);
	timespan& operator-=(timespan& lhs, const timespan& rhs);
	timespan operator+(const timespan& lhs, const timespan& rhs);
	timespan operator-(const timespan& lhs, const timespan& rhs);
};
