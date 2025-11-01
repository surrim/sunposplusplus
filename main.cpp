#include <print>
#include <thread>
#include "suncalc.h"

int main() {
	using namespace sc;

	auto xlat = 51.340333_deg, xlon = 12.37475_deg;
	auto date = get_date(2025, 10, 31, std::chrono::hours(18) + std::chrono::minutes(33));
	//auto date = std::chrono::system_clock::now();
	auto sun_position = compute_sun_position(date, xlat, xlon);
	// Expected output:
	std::println("51.340332\t12.374750\t-26.541462\t280.398315");
	std::println("{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", xlat * R2D, xlon * R2D, sun_position.solarz * R2D, sun_position.azi * R2D);
	while (true) {
		auto date = std::chrono::system_clock::now();
		auto sun_position = compute_sun_position(date, xlat, xlon);
		std::println("[{:%T}] {:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", date, xlat * R2D, xlon * R2D, sun_position.solarz * R2D, sun_position.azi * R2D);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
