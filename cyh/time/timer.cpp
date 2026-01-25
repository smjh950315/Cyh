#include "timer.hpp"
#include "time_details.h"
#ifdef __WINDOWS__
//#include <Windows.h>
#else
#include <time.h>
#endif
namespace cyh::time
{
	timer::timer()
	{
	}

	void timer::start()
	{
		this->m_start_epoch_nano = details::c_time_callbacks::get__epoch_nano();
	}

	void timer::stop()
	{
		this->m_stop_epoch_nano = details::c_time_callbacks::get__epoch_nano();
	}

	void timer::reset()
	{
		this->m_start_epoch_nano = 0;
		this->m_stop_epoch_nano = 0;
	}

	timespan timer::elapsed() const
	{
		return timespan{ m_stop_epoch_nano - m_start_epoch_nano };
	}
};

