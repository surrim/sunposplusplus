#pragma once

#include <ctime>
#include <numbers>
#include <stdfloat>

#ifndef SP_FLOATING_POINT_TYPE
#define SP_FLOATING_POINT_TYPE std::float32_t
#endif

namespace sc {
	using floating_point_t = SP_FLOATING_POINT_TYPE;
	static constexpr floating_point_t R2D = 180 / std::numbers::pi_v<floating_point_t>;
	static constexpr floating_point_t D2R = std::numbers::pi_v<floating_point_t> / 180;

	constexpr floating_point_t operator ""_deg(long double d) {
		return floating_point_t(d) * D2R;
	}

	[[nodiscard]] std::time_t get_date(int y, int m, int d, int hh = 0, int mm = 0, int ss = 0);

	struct sun_position_t {
		floating_point_t solarz;
		floating_point_t azi;
	};
	[[nodiscard]] sun_position_t compute_sun_position(std::time_t date, floating_point_t xlat, floating_point_t xlon);
}
