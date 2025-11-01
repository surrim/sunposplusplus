#include <chrono>
#include <cmath>
#include <print>
#include <thread>

using day_duration_t = std::chrono::duration<double, std::ratio<24 * 60 * 60>>;

static constexpr auto getDate(int y, unsigned m, unsigned d, std::chrono::nanoseconds ns = {}) {
	return std::chrono::sys_days(std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d)) + ns;
}

static constexpr auto J2000 = getDate(2000, 1, 1, std::chrono::hours(12)); // 1 Jan 2000 12:00 UTC

static constexpr double daysSinceJ2000(std::chrono::time_point<std::chrono::system_clock> date) {
	return day_duration_t(date - J2000).count();
}

static double dayFraction(std::chrono::time_point<std::chrono::system_clock> n) {
	auto time = std::chrono::system_clock::to_time_t(n);
	auto tm = std::tm();
	gmtime_r(&time, &tm);
	auto ymd = getDate(1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday);
	return day_duration_t(n - ymd).count();
}

static constexpr auto R2D = 180 / std::numbers::pi;
static constexpr auto D2R = std::numbers::pi / 180;

static constexpr double fmodulo2Pi(double x) {
	return x - std::floor(0.5 * std::numbers::inv_pi * x) * 2 * std::numbers::pi;
}

constexpr double operator ""_deg(long double d) {
	return d * D2R;
}

// See https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position
struct sun_position_t { double solarz; double azi; };
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::time_point<std::chrono::system_clock> date, double xlat, double xlon) {
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
	auto sunlon = -2 * std::numbers::pi * dayFraction(date) + std::numbers::pi - EoT;
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

int main() {
	auto xlat = 51.340333_deg, xlon = 12.37475_deg;
	auto date = getDate(2025, 10, 31, std::chrono::hours(18) + std::chrono::minutes(33));
	//auto date = std::chrono::system_clock::now();
	auto sun_position = sunpos_ultimate_azi_atan2(date, xlat, xlon);
	// Expected output:
	std::println("51.340332\t12.374750\t-26.541462\t280.398315");
	std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat * R2D, xlon * R2D, sun_position.solarz * R2D, sun_position.azi * R2D);
	while (true) {
		auto date = std::chrono::system_clock::now();
		auto sun_position = sunpos_ultimate_azi_atan2(date, xlat, xlon);
		std::println("[{:%T}] {:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", date, xlat * R2D, xlon * R2D, sun_position.solarz * R2D, sun_position.azi * R2D);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
