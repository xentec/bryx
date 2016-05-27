#pragma once

#include <chrono>

#include "global.h"

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = TimePoint::duration;
