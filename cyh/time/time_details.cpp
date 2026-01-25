#include "time_details.h"
#include <chrono>
#include <ctime>
#ifdef __WINDOWS__
#include <Windows.h>
#else
#include <time.h>
#endif
namespace cyh::time::details {
	inline std::tm* get_local_tm(const std::time_t* src, std::tm* dst) {
#if defined(_WIN32)                    // Visual C++, clang-cl, etc.
		return (::_localtime64_s(dst, src) == 0) ? dst : nullptr;
#elif defined(__STDC_LIB_EXT1__)       // C11 Annex K, some libcs
		return (::localtime_s(dst, src) == 0) ? dst : nullptr;
#elif defined(_POSIX_THREAD_SAFE_FUNCTIONS)    // Linux, macOS, *BSD ¡K
		return ::localtime_r(src, dst);            // returns dst on success
#else                                     // fallback = not thread-safe
		if (auto* p = ::localtime(src)) { *dst = *p; return dst; }
		return nullptr;
#endif
	}
	inline std::tm* get_gmt_tm(const std::time_t* src, std::tm* dst) {
#if defined(_WIN32)
		return (::_gmtime64_s(dst, src) == 0) ? dst : nullptr;
#elif defined(__STDC_LIB_EXT1__)
		return (::gmtime_s(dst, src) == 0) ? dst : nullptr;
#elif defined(_POSIX_THREAD_SAFE_FUNCTIONS)
		return ::gmtime_r(src, dst);
#else
		if (auto* p = ::gmtime(src)) { *dst = *p; return dst; }
		return nullptr;
#endif
	}

	inline std::time_t get_local_timestamp(std::tm* _ptm) {
		return std::mktime((tm*)_ptm);
	}
	inline std::time_t get_gmt_timestamp(std::tm* _ptm) {
#if defined(_WIN32)
		return _mkgmtime(_ptm);
#else
		return timegm(_ptm);
#endif
	}

	inline bool try_get_tm(const int64 _time_t, std::tm* _ptm, std::tm* (*callback)(const std::time_t*, std::tm*)) {
		if (callback == nullptr) { return false; }
		std::time_t time = static_cast<std::time_t>(_time_t);
		if (callback(&time, _ptm) == nullptr) { return false; }
		return true;
	}
	inline bool try_get_timestamp(std::tm* _ptm, std::time_t* _pts, std::time_t(*callback)(std::tm*)) {
		if (callback == nullptr) { return false; }
		std::time_t time = callback(_ptm);
		if (time == -1) { return false; }
		*_pts = time;
		return true;
	}

	static bool time_t__to__tm_impl(const int64 _time_t, std::tm* _ptm, int time_type) {
		switch (time_type)
		{
		case cyh::time::TIME_TYPE_GMT:
			return try_get_tm(_time_t, _ptm, get_gmt_tm);
		default:
			return try_get_tm(_time_t, _ptm, get_local_tm);
		}
	}
	static bool tm__to__time_t_impl(const std::tm* _ptm, int64* _time_t, int time_type) {
		switch (time_type)
		{
		case cyh::time::TIME_TYPE_GMT:
			return try_get_timestamp((std::tm*)_ptm, _time_t, get_gmt_timestamp);
		default:
			return try_get_timestamp((std::tm*)_ptm, _time_t, get_local_timestamp);
		}
	}

	static bool _impl_get__time_t_now(int64* _time_t, int64* _fraction) {
		auto _now = std::chrono::system_clock::now();
		auto _now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(_now.time_since_epoch()).count();
		auto _now_s = std::chrono::system_clock::to_time_t(_now);
		*_time_t = _now_s;
		if (_fraction)
			*_fraction = _now_ns % 1000000000ll;
		return true;
	}
	static int64 _impl_get__epoch_nano() {
		auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto epoch_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);
		return epoch_nano.count();
	}

	SET_CALLBACK_API(c_time_callbacks::time_t__to__tm, time_t__to__tm_impl);
	SET_CALLBACK_API(c_time_callbacks::tm__to__time_t, tm__to__time_t_impl);

	SET_CALLBACK_API(c_time_callbacks::get__time_t_now, _impl_get__time_t_now);
	SET_CALLBACK_API(c_time_callbacks::get__epoch_nano, _impl_get__epoch_nano);
};
