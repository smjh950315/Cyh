#pragma once
#include <cyh/time/time_.hpp>
#include <ctime>
namespace cyh::time::details {
	struct unit_cstr {
		static constexpr const char* nano = "ns";
		static constexpr const char* micro = "us";
		static constexpr const char* milli = "ms";
		static constexpr const char* second = "s";
		static constexpr const char* minute = "m";
		static constexpr const char* hour = "h";
		static constexpr const char* day = "d";
		static constexpr const char* units[] = {
			nano,
			micro,
			milli,
			second,
			minute,
			hour,
			day
		};
	};
	struct nano_rate_from {
		static constexpr double nanos = 1.0;
		static constexpr double micros = 1e3;
		static constexpr double millis = 1e6;
		static constexpr double seconds = 1e9;
		static constexpr double minutes = 1e9 * 60.0;
		static constexpr double hours = 1e9 * 3600.0;
		static constexpr double days = 1e9 * 86400.0;
		static constexpr double nano_rates[] = {
			nanos,
			micros,
			millis,
			seconds,
			minutes,
			hours,
			days
		};
	};
	struct c_time_callbacks {
		static bool (*time_t__to__tm)(const int64 _time_t, std::tm* my_tm, int time_type);
		static bool (*tm__to__time_t)(const std::tm* my_tm, int64* _time_t, int time_type);
		static bool (*get__time_t_now)(int64* _time_t, int64* _fraction);
		static int64(*get__epoch_nano)();
	};
};