#include <cmath>
#include <print>

template<typename F> static constexpr F fmodulo360(F x) {
    return x - std::floor(x / 360) * 360;
}

static constexpr double rpd = std::acos(-1.0) / 180.0;

struct sun_position_t { double solarz; double azi; };
sun_position_t sunpos_ultimate_azi_atan2(int inyear, int inmon, int inday, double gmtime, double xlat, double xlon) {
    auto nday = std::array{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (
        ((inyear % 100) != 0 && (inyear %   4) == 0) ||
        ((inyear % 100) == 0 && (inyear % 400) == 0)
    ) {
        nday[1] = 29;
    } else {
        nday[1] = 28;
    }

    auto julday = std::array<int, 13>();
    for (auto i = 1; i <= 12; i++) {
        julday[i] = julday[i - 1] + nday[i - 1];
    }
    // Note: julday[12] is equal to either 365 or 366.

    auto dyear = inyear - 2000;
    auto dayofyr = julday[inmon - 1] + inday;
    auto xleap = 0;
    if (dyear <= 0) {
        xleap = 0.25 * dyear; // Note: xleap has the SAME SIGN as dyear.
    } else {
        if ((dyear % 4) == 0) {
            xleap = 0.25 * dyear; // For leap-years.
        } else {
            xleap = 0.25 * dyear + 1; // +1 is for year 2000.
        }
    }

    // --- Astronomical Almanac for the Year 2019, Page C5 ---
    auto n = -1.5 + dyear * 365 + xleap + dayofyr + gmtime / 24.0;
    auto L = fmodulo360(280.460 + 0.9856474 * n);
    auto g = fmodulo360(357.528 + 0.9856003 * n);
    auto lambda = fmodulo360(L + 1.915 * std::sin(g * rpd) + 0.020 * std::sin(2.0 * g * rpd));
    auto epsilon = 23.439 - 0.0000004 * n;
    auto alpha = fmodulo360(std::atan2(std::cos(epsilon * rpd) * std::sin(lambda * rpd),
                        std::cos(lambda * rpd)) / rpd);
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
    auto sun_position = sunpos_ultimate_azi_atan2(2025, 10, 31, hour, xlat, xlon);
    // Expected output:
    std::println("51.340332\t12.374750\t-26.541462\t280.398315");
    std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat, xlon, sun_position.solarz, sun_position.azi);
    return 0;
}
