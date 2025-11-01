#pragma once

#include <chrono>
#include <stdfloat>

#ifndef SP_FLOATING_POINT_TYPE
#define SP_FLOATING_POINT_TYPE std::float32_t
#endif

using floating_point_t = SP_FLOATING_POINT_TYPE;
static constexpr floating_point_t R2D = 180 / std::numbers::pi_v<floating_point_t>;
static constexpr floating_point_t D2R = std::numbers::pi_v<floating_point_t> / 180;

constexpr floating_point_t operator ""_deg(long double d) {
	return floating_point_t(d) * D2R;
}

static constexpr auto getDate(int y, unsigned m, unsigned d, std::chrono::nanoseconds ns = {}) {
	return std::chrono::sys_days(std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d)) + ns;
}

struct sun_position_t {
	floating_point_t solarz;
	floating_point_t azi;
};
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::time_point<std::chrono::system_clock> date, floating_point_t xlat, floating_point_t xlon);
