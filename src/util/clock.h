#pragma once

#include <chrono>

#include "global.h"
#include "misc.h"

using Duration = std::chrono::duration<f64, std::milli>;

struct Clock
{
  typedef Duration				duration;
  typedef duration::rep			rep;
  typedef duration::period		period;
  typedef std::chrono::time_point<std::chrono::steady_clock, duration> 	time_point;

  static inline time_point now() noexcept
  {
	  return std::chrono::time_point_cast<Duration>(time_point::clock::now());
  }
};

using TimePoint = Clock::time_point;


struct Timer
{
	TimePoint begin;

	inline void start() { begin = Clock::now(); }
	inline Duration& stop() { prev.push(Clock::now()-begin); return prev.data.back(); }
	LowPass<Duration, 10> prev;
};
