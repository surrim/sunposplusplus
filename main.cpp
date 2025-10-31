#include <chrono>
#include <cmath>
#include <print>

static constexpr double fmodulo360(double x) {
    return x - std::floor(x / 360) * 360;
}

using hour_duration_t = std::chrono::duration<double, std::ratio<60 * 60>>;
using day_duration_t = std::chrono::duration<double, std::ratio<24 * 60 * 60>>;
using time_point_t = std::chrono::time_point<std::chrono::system_clock, hour_duration_t>;

static constexpr std::chrono::year_month_day getYMD(int y, int m, int d) {
    using namespace std::chrono;
    return year(y) / month(m) / day(d);
}

static constexpr time_point_t getDate(std::chrono::year_month_day ymd, double h) {
    using namespace std::chrono;
    return sys_days(ymd) + hour_duration_t(h);
}

static constexpr double rpd = std::numbers::pi / 180.0;
static constexpr auto J2000 = getDate(getYMD(2000, 1, 1), 12); // 1 Jan 2000 12:00 UTC

static constexpr double daysSinceJ2000(time_point_t date) {
    return day_duration_t(date - J2000).count();
}

struct sun_position_t { double solarz; double azi; };
sun_position_t sunpos_ultimate_azi_atan2(std::chrono::year_month_day ymd, double gmtime, double xlat, double xlon) {
    // --- Astronomical Almanac for the Year 2019, Page C5 ---
    auto n = daysSinceJ2000(getDate(ymd, gmtime));
    std::println("{}", n);
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
    auto sunlon = -15.0 * (gmtime - 12.0 + EoT * 4.0 / 60.0);
    auto PHIo = xlat * rpd;
    auto PHIs = sunlat * rpd;
    auto LAMo = xlon * rpd;
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
    auto hour = 18.0 + 33.0 / 60.0;
    auto sun_position = sunpos_ultimate_azi_atan2(getYMD(2025, 10, 31), hour, xlat, xlon);
    // Expected output:
    std::println("51.340332\t12.374750\t-26.541462\t280.398315");
    std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat, xlon, sun_position.solarz, sun_position.azi);

    //auto date = std::chrono::system_clock::now();

    return 0;
}
