#pragma once
#include "timespan.h"
namespace cyh::time 
{
	class timer
	{
		int64 m_start_epoch_nano{};
		int64 m_stop_epoch_nano{};
	public:
		timer();
		void start();
		void stop();
		void reset();
		timespan elapsed() const;
	};
};

