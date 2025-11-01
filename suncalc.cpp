#include "suncalc.h"
#include <cmath>

using day_duration_t = std::chrono::duration<floating_point_t, std::ratio<24 * 60 * 60>>;

constexpr floating_point_t operator ""_fp(long double d) {
	return floating_point_t(d);
}

static constexpr auto J2000 = getDate(2000, 1, 1, std::chrono::hours(12)); // 1 Jan 2000 12:00 UTC

static constexpr floating_point_t daysSinceJ2000(std::chrono::time_point<std::chrono::system_clock> date) {
	return day_duration_t(date - J2000).count();
}

static floating_point_t dayFraction(std::chrono::time_point<std::chrono::system_clock> n) {
	auto time = std::chrono::system_clock::to_time_t(n);
	auto tm = std::tm();
	gmtime_r(&time, &tm);
	auto ymd = getDate(1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday);
	return day_duration_t(n - ymd).count();
}

static constexpr floating_point_t fmodulo2Pi(floating_point_t x) {
	return x - std::floor(0.5_fp * std::numbers::inv_pi_v<floating_point_t> * x) * 2 * std::numbers::pi_v<floating_point_t>;
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::time_point<std::chrono::system_clock> date, floating_point_t xlat, floating_point_t xlon) {
	// --- Astronomical Almanac for the Year 2019, Page C5 ---
	auto n = daysSinceJ2000(date);
	auto L = fmodulo2Pi(280.460_deg + 0.985'647'4_deg * n);
	auto g = fmodulo2Pi(357.528_deg + 0.985'600'3_deg * n);
	auto lambda = fmodulo2Pi(L + 1.915_deg * std::sin(g) + 0.020_deg * std::sin(2 * g));
	auto epsilon = 23.439_deg - 0.000'000'4_deg * n; // 23.43916666666666666666
	auto alpha = std::atan2(std::cos(epsilon) * std::sin(lambda), std::cos(lambda));
	auto delta = std::asin(std::sin(epsilon) * std::sin(lambda));
	//auto R = 1.000'14 - 0.016'71 * std::cos(g) - 0.000'14 * std::cos(2 * g);
	auto EoT = fmodulo2Pi(L - alpha);

	// --- Solar geometry ---
	auto sunlat = delta;
	auto sunlon = -2 * std::numbers::pi_v<floating_point_t> * dayFraction(date) + std::numbers::pi_v<floating_point_t> - EoT;
	auto PHIo =   xlat;
	auto PHIs = sunlat;
	auto LAMo =   xlon;
	auto LAMS = sunlon;
	auto Sx = std::cos(PHIs) * std::sin(LAMS - LAMo);
	auto Sy = std::cos(PHIo) * std::sin(PHIs) - std::sin(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);
	auto Sz = std::sin(PHIo) * std::sin(PHIs) + std::cos(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);

	return {
		.solarz = std::asin(Sz),
		.azi = fmodulo2Pi(std::atan2(Sx, Sy))
	};
}
