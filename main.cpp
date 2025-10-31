#include <cmath>
#include <print>

static constexpr float fmodulo(float x, float y) {
    return x - floor(x / y) * y;
}

void sunpos_ultimate_azi_atan2(int inyear, int inmon, int inday, float gmtime,
                               float xlat, float xlon, float *solarz, float *azi) {
    int nday[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int julday[13];
    int xleap, i, dyear, dayofyr;
    float n, L, g, lambda, epsilon, alpha, delta, /*R, */EoT;
    float sunlat, sunlon, PHIo, PHIs, LAMo, LAMS, Sx, Sy, Sz;
    const float rpd = acos(-1.0) / 180.0f;

    if (((inyear % 100) != 0 && (inyear % 4) == 0) ||
        ((inyear % 100) == 0 && (inyear % 400) == 0)) {
        nday[1] = 29;
    } else {
        nday[1] = 28;
    }

    julday[0] = 0;
    for (i = 1; i <= 12; i++) {
        julday[i] = julday[i - 1] + nday[i - 1];
    }
    // Note: julday[12] is equal to either 365 or 366.

    dyear = inyear - 2000;
    dayofyr = julday[inmon - 1] + inday;
    if (dyear <= 0) {
        xleap = (int)((float)dyear / 4.0f); // Note: xleap has the SAME SIGN as dyear.
    } else {
        if ((dyear % 4) == 0) {
            xleap = (int)((float)dyear / 4.0f); // For leap-years.
        } else {
            xleap = (int)((float)dyear / 4.0f) + 1; // +1 is for year 2000.
        }
    }

    // --- Astronomical Almanac for the Year 2019, Page C5 ---
    n = -1.5f + dyear * 365.0f + xleap * 1.0f + dayofyr + gmtime / 24.0f;
    L = fmodulo(280.460f + 0.9856474f * n, 360.0f);
    g = fmodulo(357.528f + 0.9856003f * n, 360.0f);
    lambda = fmodulo(L + 1.915f * sinf(g * rpd) + 0.020f * sinf(2.0f * g * rpd), 360.0f);
    epsilon = 23.439f - 0.0000004f * n;
    alpha = fmodulo(atan2f(cosf(epsilon * rpd) * sinf(lambda * rpd),
                        cosf(lambda * rpd)) / rpd, 360.0f);
    delta = asinf(sinf(epsilon * rpd) * sinf(lambda * rpd)) / rpd;
    //R = 1.00014f - 0.01671f * cosf(g * rpd) - 0.00014f * cosf(2.0f * g * rpd);
    EoT = fmodf((L - alpha) + 180.0f, 360.0f) - 180.0f; // In deg.

    // --- Solar geometry ---
    sunlat = delta; // In deg.
    sunlon = -15.0f * (gmtime - 12.0f + EoT * 4.0f / 60.0f);
    PHIo = xlat * rpd;
    PHIs = sunlat * rpd;
    LAMo = xlon * rpd;
    LAMS = sunlon * rpd;
    Sx = cosf(PHIs) * sinf(LAMS - LAMo);
    Sy = cosf(PHIo) * sinf(PHIs) - sinf(PHIo) * cosf(PHIs) * cosf(LAMS - LAMo);
    Sz = sinf(PHIo) * sinf(PHIs) + cosf(PHIo) * cosf(PHIs) * cosf(LAMS - LAMo);

    // solarz = acos(Sz) / rpd; // In deg.
    *solarz = asinf(Sz) / rpd; // In deg.
    // azi = atan2(-Sx, -Sy) / rpd; // In deg. South-Clockwise Convention.
    *azi = atan2f(Sx, Sy) / rpd; // In deg. South-Clockwise Convention.
}

int main() {
    float xlat = 51.340333f, xlon = 12.37475f, solarz, azi;
    float hour = 18.0f + 33.0f / 60.0f;
    sunpos_ultimate_azi_atan2(2025, 10, 31, hour, xlat, xlon, &solarz, &azi);
    // Expected output:
    std::println("51.340332\t12.374750\t-26.541462\t280.398315");
    std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat, xlon, solarz, fmodf(azi + 360.0f, 360.0f));
    return 0;
}
