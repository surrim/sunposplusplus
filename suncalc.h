#pragma once

#include <chrono>

static constexpr auto R2D = 180 / std::numbers::pi;
static constexpr auto D2R = std::numbers::pi / 180;

constexpr double operator ""_deg(long double d) {
	return d * D2R;
}

static constexpr auto getDate(int y, unsigned m, unsigned d, std::chrono::nanoseconds ns = {}) {
	return std::chrono::sys_days(std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d)) + ns;
}

struct sun_position_t {
	double solarz;
	double azi;
};
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::time_point<std::chrono::system_clock> date, double xlat, double xlon);
