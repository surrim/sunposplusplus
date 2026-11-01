/* Copyright 2026 surrim
 *
 * This file is part of sunpos++.
 *
 * sunpos++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * sunpos++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sunpos++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sunpos.h"
#include <cmath>

constexpr sc::floating_point_t operator ""_fp(long double d) {
	return sc::floating_point_t(d);
}

static constexpr auto J2000 = std::chrono::sys_days(std::chrono::year(2000) / 1 / 1); // 1 Jan 2000 00:00 UTC

static sc::floating_point_t days_since_j2000(std::chrono::sys_seconds date) {
	using namespace std::chrono;
	return duration_cast<duration<sc::floating_point_t, std::ratio<24 * 60 * 60>>>(date - J2000).count();
}

static constexpr sc::floating_point_t fmodulo_rad(sc::floating_point_t x) {
	static constexpr auto INV_PI = std::numbers::inv_pi_v<sc::floating_point_t>;
	static constexpr auto INV_TWO_PI = 0.5_fp * INV_PI;
	return x - std::floor(INV_TWO_PI * x) * sc::TWO_PI;
}

std::chrono::sys_seconds sc::get_date(int y, int m, int d, int hh, int mm, int ss) {
	using namespace std::chrono;
	auto ymd = year(y) / month(m) / day(d);
	return sys_days(ymd) + hours(hh) + minutes(mm) + seconds(ss);
}

sc::sun_position_t sc::compute_sun_position(std::chrono::sys_seconds date, floating_point_t latitude, floating_point_t longitude) {
	auto daysSinceJ2000 = days_since_j2000(date);
	return compute_sun_position(daysSinceJ2000, latitude, longitude);
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
// and https://www.sciencedirect.com/science/article/pii/S0960148121004031
sc::sun_position_t sc::compute_sun_position(floating_point_t daysSinceJ2000, floating_point_t latitude, floating_point_t longitude) {
	using std::sin, std::cos, std::asin, std::atan2, std::round;

	// --- Astronomical Almanac for the Year 2019, Page C5 ---
	auto n = daysSinceJ2000 - 0.5_fp;
	auto L = fmodulo_rad(280.460_deg + 0.985'647'4_deg * n);
	auto g = fmodulo_rad(357.528_deg + 0.985'600'3_deg * n);
	auto lambda = fmodulo_rad(L + 1.915_deg * sin(g) + 0.020_deg * sin(2 * g));
	auto epsilon = 23.439_deg - 0.000'000'4_deg * n; // 23.43916666666666666666
	auto alpha = atan2(cos(epsilon) * sin(lambda), cos(lambda));
	auto sunLatitude = asin(sin(epsilon) * sin(lambda));
	auto EoT = fmodulo_rad(L - alpha);
	//auto R = 1.000'14 - 0.016'71 * cos(g) - 0.000'14 * cos(2 * g);

	// --- Solar geometry ---
	auto fractionOfDay = daysSinceJ2000 - round(daysSinceJ2000);
	auto sunLongitude = PI * (1 - 2 * fractionOfDay) - EoT;
	auto Sx = cos(sunLatitude) * sin(sunLongitude - longitude);
	auto Sy = cos(latitude) * sin(sunLatitude) - sin(latitude) * cos(sunLatitude) * cos(sunLongitude - longitude);
	auto Sz = sin(latitude) * sin(sunLatitude) + cos(latitude) * cos(sunLatitude) * cos(sunLongitude - longitude);

	return {
		.zenithAngle = asin(Sz),
		.azimuthAngle = fmodulo_rad(atan2(Sx, Sy))
	};
}
