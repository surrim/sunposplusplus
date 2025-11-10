/* Copyright 2025 surrim
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

static auto J2000 = sc::get_date(2000, 1, 1, 12); // 1 Jan 2000 12:00 UTC

static sc::floating_point_t days_since_j2000(std::time_t date) {
	return (date - J2000) * (1 / sc::floating_point_t(24 * 60 * 60));
}

static sc::floating_point_t day_fraction(std::time_t time) {
	auto tm = std::tm();
	gmtime_r(&time, &tm);
	auto ymd = sc::get_date(tm.tm_year + 1900 , tm.tm_mon + 1, tm.tm_mday);
	return (time - ymd) * (1 / sc::floating_point_t(24 * 60 * 60));
}

static constexpr sc::floating_point_t fmodulo_rad(sc::floating_point_t x) {
	return x - std::floor(0.5_fp * std::numbers::inv_pi_v<sc::floating_point_t> * x) * 2 * std::numbers::pi_v<sc::floating_point_t>;
}

std::time_t sc::get_date(int y, int m, int d, int hh, int mm, int ss) {
	auto tm = std::tm{
		.tm_sec  = ss,
		.tm_min  = mm,
		.tm_hour = hh,
		.tm_mday = d,
		.tm_mon  = m - 1,
		.tm_year = y - 1900
	};
#ifndef _WIN32
	return timegm(&tm);
#else
	return _mkgmtime(&tm);
#endif
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
// and https://www.sciencedirect.com/science/article/pii/S0960148121004031
sc::sun_position_t sc::compute_sun_position(std::time_t date, sc::floating_point_t xlat, sc::floating_point_t xlon) {
	// --- Astronomical Almanac for the Year 2019, Page C5 ---
	auto n = days_since_j2000(date);
	auto L = fmodulo_rad(280.460_deg + 0.985'647'4_deg * n);
	auto g = fmodulo_rad(357.528_deg + 0.985'600'3_deg * n);
	auto lambda = fmodulo_rad(L + 1.915_deg * std::sin(g) + 0.020_deg * std::sin(2 * g));
	auto epsilon = 23.439_deg - 0.000'000'4_deg * n; // 23.43916666666666666666
	auto alpha = std::atan2(std::cos(epsilon) * std::sin(lambda), std::cos(lambda));
	auto delta = std::asin(std::sin(epsilon) * std::sin(lambda));
	//auto R = 1.000'14 - 0.016'71 * std::cos(g) - 0.000'14 * std::cos(2 * g);
	auto EoT = fmodulo_rad(L - alpha);

	// --- Solar geometry ---
	auto sunlat = delta;
	auto sunlon = -2 * std::numbers::pi_v<floating_point_t> * day_fraction(date) + std::numbers::pi_v<floating_point_t> - EoT;
	auto PHIo =   xlat;
	auto PHIs = sunlat;
	auto LAMo =   xlon;
	auto LAMS = sunlon;
	auto Sx = std::cos(PHIs) * std::sin(LAMS - LAMo);
	auto Sy = std::cos(PHIo) * std::sin(PHIs) - std::sin(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);
	auto Sz = std::sin(PHIo) * std::sin(PHIs) + std::cos(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);

	return {
		.solarz = std::asin(Sz),
		.azi = fmodulo_rad(std::atan2(Sx, Sy))
	};
}
