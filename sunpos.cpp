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

static constexpr auto DAYS_PER_YEAR = 365;
static constexpr auto SECOND = uint32_t(1);
static constexpr auto SECONDS_PER_MINUTE = 60 * SECOND;
static constexpr auto SECONDS_PER_HOUR   = 60 * SECONDS_PER_MINUTE;
static constexpr auto SECONDS_PER_DAY    = 24 * SECONDS_PER_HOUR;
static constexpr auto DAYS_PER_SECOND = sp::floating_point_t(1) / SECONDS_PER_DAY;
static constexpr auto Y2K = std::chrono::sys_days(std::chrono::year(2000) / 1 / 1); // 1 Jan 2000 00:00 UTC
static constexpr auto UNIX_TIME_OFFSET_IN_DAYS = 10957;

static constexpr auto MONTH_DAYS_ACCUMULATED = (uint16_t[12]) { 0 + DAYS_PER_YEAR, 31 + DAYS_PER_YEAR, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static constexpr sp::floating_point_t fmodulo_rad(sp::floating_point_t x) {
	static constexpr auto INV_PI = std::numbers::inv_pi_v<sp::floating_point_t>;
	static constexpr auto INV_TWO_PI = sp::floating_point_t(0.5) * INV_PI;
	return x - std::floor(INV_TWO_PI * x) * sp::TWO_PI;
}

static int16_t floorDivide(int16_t a, uint16_t b) {
	return a / b - (a % b < 0);
}

int32_t sp::date2DaysAfterY2K(int16_t year, uint8_t month, uint8_t day) {
	auto relativeYear = year - 2000 - (month <= 2);
	return DAYS_PER_YEAR * relativeYear
		+ floorDivide(relativeYear, 4)
		- floorDivide(relativeYear, 100)
		+ floorDivide(relativeYear, 400)
		+ MONTH_DAYS_ACCUMULATED[month - 1]
		+ day;
}

sp::floating_point_t sp::date2DaysAfterY2K(int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
	return date2DaysAfterY2K(year, month, day)
		+ (hour * SECONDS_PER_HOUR + minute * SECONDS_PER_MINUTE + second * SECOND) * DAYS_PER_SECOND;
}

sp::floating_point_t sp::date2DaysAfterY2K(uint32_t unixTime) {
	return unixTime * (1 / floating_point_t(SECONDS_PER_DAY)) + UNIX_TIME_OFFSET_IN_DAYS;
}

sp::floating_point_t sp::date2DaysAfterY2K(std::chrono::sys_seconds date) {
	using namespace std::chrono;
	return duration_cast<duration<floating_point_t, std::ratio<SECONDS_PER_DAY>>>(date - Y2K).count();
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
// and https://www.sciencedirect.com/science/article/pii/S0960148121004031
sp::sun_position_t sp::compute_sun_position(floating_point_t daysAfterY2K, floating_point_t latitude, floating_point_t longitude) {
	using std::sin, std::cos, std::asin, std::atan2, std::round;

	// --- Astronomical Almanac for the Year 2019, Page C5 ---
	auto n = daysAfterY2K - floating_point_t(0.5);
	auto L = fmodulo_rad(from360(280.460f128) + from360(0.985'647'4f128) * n);
	auto g = fmodulo_rad(from360(357.528f128) + from360(0.985'600'3f128) * n);
	auto lambda = fmodulo_rad(L + from360(1.915f128) * sin(g) + from360(0.020f128) * sin(2 * g));
	auto epsilon = from360(23.439f128) - from360(0.000'000'4f128) * n; // 23.43916666666666666666
	auto alpha = atan2(cos(epsilon) * sin(lambda), cos(lambda));
	auto sunLatitude = asin(sin(epsilon) * sin(lambda));
	auto EoT = fmodulo_rad(L - alpha);
	//auto R = 1.000'14 - 0.016'71 * cos(g) - 0.000'14 * cos(2 * g);

	// --- Solar geometry ---
	auto fractionOfDay = daysAfterY2K - round(daysAfterY2K);
	auto sunLongitude = PI * (1 - 2 * fractionOfDay) - EoT;
	auto Sx = cos(sunLatitude) * sin(sunLongitude - longitude);
	auto Sy = cos(latitude) * sin(sunLatitude) - sin(latitude) * cos(sunLatitude) * cos(sunLongitude - longitude);
	auto Sz = sin(latitude) * sin(sunLatitude) + cos(latitude) * cos(sunLatitude) * cos(sunLongitude - longitude);

	return {
		.zenithAngle = asin(Sz),
		.azimuthAngle = fmodulo_rad(atan2(Sx, Sy))
	};
}
