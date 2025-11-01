#include <chrono>
#include <cmath>
#include <print>

static constexpr double fmodulo360(double x) {
    return x - std::floor(x / 360) * 360;
}

using day_duration_t = std::chrono::duration<double, std::ratio<24 * 60 * 60>>;

static constexpr auto getDate(int y, unsigned m, unsigned d, std::chrono::nanoseconds ns = {}) {
    return std::chrono::sys_days(std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d)) + ns;
}

static constexpr double rpd = std::numbers::pi / 180;
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

struct sun_position_t { double solarz; double azi; };
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::time_point<std::chrono::system_clock> date, double xlat, double xlon) {
    // --- Astronomical Almanac for the Year 2019, Page C5 ---
    auto n = daysSinceJ2000(date);
    auto L = fmodulo360(280.460 + 0.9856474 * n);
    auto g = fmodulo360(357.528 + 0.9856003 * n);
    auto lambda = fmodulo360(L + 1.915 * std::sin(g * rpd) + 0.020 * std::sin(2.0 * g * rpd));
    auto epsilon = 23.439 - 0.0000004 * n;
    auto alpha = fmodulo360(std::atan2(std::cos(epsilon * rpd) * std::sin(lambda * rpd), std::cos(lambda * rpd)) / rpd);
    auto delta = asinf(std::sin(epsilon * rpd) * std::sin(lambda * rpd)) / rpd;
    //auto R = 1.00014 - 0.01671 * std::cos(g * rpd) - 0.00014 * std::cos(2.0 * g * rpd);
    auto EoT = fmodulo360((L - alpha) + 180.0) - 180.0; // In deg.

    // --- Solar geometry ---
    auto sunlat = delta; // In deg.
    auto sunlon = -360 * dayFraction(date) + 180 - EoT; // In deg.
    auto PHIo =   xlat * rpd;
    auto PHIs = sunlat * rpd;
    auto LAMo =   xlon * rpd;
    auto LAMS = sunlon * rpd;
    auto Sx = std::cos(PHIs) * std::sin(LAMS - LAMo);
    auto Sy = std::cos(PHIo) * std::sin(PHIs) - std::sin(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);
    auto Sz = std::sin(PHIo) * std::sin(PHIs) + std::cos(PHIo) * std::cos(PHIs) * std::cos(LAMS - LAMo);

    return {
        .solarz = asinf(Sz) / rpd, // In deg.
        .azi = fmodulo360(std::atan2(Sx, Sy) / rpd) // In deg.
    };
}

int main() {
    auto xlat = 51.340333, xlon = 12.37475;
    auto date = getDate(2025, 10, 31, std::chrono::hours(18) + std::chrono::minutes(33));
    //auto date = std::chrono::system_clock::now();
    auto sun_position = sunpos_ultimate_azi_atan2(date, xlat, xlon);
    // Expected output:
    std::println("51.340332\t12.374750\t-26.541462\t280.398315");
    std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat, xlon, sun_position.solarz, sun_position.azi);

    return 0;
}
