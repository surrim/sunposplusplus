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

// 1 Jan 2000 12:00 UTC
static constexpr auto J2000x = std::chrono::sys_days(std::chrono::year(2000) / 1 / 1) + std::chrono::hours(12);

static sc::floating_point_t days_since_j2000(std::chrono::sys_seconds date) {
    using namespace std::chrono;

	return duration_cast<duration<sc::floating_point_t, std::ratio<24 * 60 * 60>>>(date - J2000x).count();
}

static constexpr sc::floating_point_t day_fraction(std::chrono::sys_seconds t) {
    using namespace std::chrono;

    auto timepoint = t;
    auto startOfDay = sys_days(floor<days>(timepoint));
    auto secs = duration_cast<seconds>(timepoint - startOfDay);
    return secs.count() / sc::floating_point_t(24 * 60 * 60);
}

static constexpr sc::floating_point_t fmodulo_rad(sc::floating_point_t x) {
	static constexpr auto INV_TWO_PI = 0.5_fp * std::numbers::inv_pi_v<sc::floating_point_t>;
	static constexpr auto TWO_PI = 2 * std::numbers::pi_v<sc::floating_point_t>;
	return x - std::floor(INV_TWO_PI * x) * TWO_PI;
}

std::chrono::sys_seconds sc::get_date(int y, int m, int d, int hh, int mm, int ss) {
	using namespace std::chrono;

    auto ymd = year(y) / month(m) / day(d);
    return sys_days(ymd) + hours(hh) + minutes(mm) + seconds(ss);
}

sc::sun_position_t sc::compute_sun_position(std::chrono::sys_seconds date, floating_point_t xlat, floating_point_t xlon) {
	auto n = days_since_j2000(date);
	auto df = day_fraction(date);
	return compute_sun_position(n, df, xlat, xlon);
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
// and https://www.sciencedirect.com/science/article/pii/S0960148121004031
sc::sun_position_t sc::compute_sun_position(floating_point_t n, floating_point_t df, floating_point_t xlat, floating_point_t xlon) {
	// --- Astronomical Almanac for the Year 2019, Page C5 ---
	auto L = fmodulo_rad(280.460_deg + 0.985'647'4_deg * n);
	auto g = fmodulo_rad(357.528_deg + 0.985'600'3_deg * n);
	auto lambda = fmodulo_rad(L + 1.915_deg * std::sin(g) + 0.020_deg * std::sin(2 * g));
	auto epsilon = 23.439_deg - 0.000'000'4_deg * n; // 23.43916666666666666666
	auto alpha = std::atan2(std::cos(epsilon) * std::sin(lambda), std::cos(lambda));
	auto sunlat = std::asin(std::sin(epsilon) * std::sin(lambda));
	auto EoT = fmodulo_rad(L - alpha);
	//auto R = 1.000'14 - 0.016'71 * std::cos(g) - 0.000'14 * std::cos(2 * g);

	// --- Solar geometry ---
	auto sunlon = std::numbers::pi_v<floating_point_t> * (-2 * df + 1) - EoT;
	auto Sx = std::cos(sunlat) * std::sin(sunlon - xlon);
	auto Sy = std::cos(xlat) * std::sin(sunlat) - std::sin(xlat) * std::cos(sunlat) * std::cos(sunlon - xlon);
	auto Sz = std::sin(xlat) * std::sin(sunlat) + std::cos(xlat) * std::cos(sunlat) * std::cos(sunlon - xlon);

	return {
		.zenithAngle = std::asin(Sz),
		.azimuthAngle = fmodulo_rad(std::atan2(Sx, Sy))
	};
}
