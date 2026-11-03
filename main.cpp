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
#include <expected>
#include <print>
#include <spanstream>
#include <thread>
#include <argparse/argparse.hpp>

static constexpr auto DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
static constexpr auto MAX_DIGITS = std::numeric_limits<sp::floating_point_t>::max_digits10;

static std::expected<std::chrono::sys_seconds, std::nullptr_t> readDate(std::string_view dateString, std::string_view dateFormat) {
    auto stream = std::ispanstream(dateString);
    auto timepoint = std::chrono::sys_seconds();
    if (std::chrono::from_stream(stream, dateFormat.data(), timepoint)) {
        return timepoint;
    }
    return std::unexpected(nullptr);
}

static void printSunPosition(std::chrono::sys_seconds date, sp::floating_point_t latitude360, sp::floating_point_t longitude360) {
	auto latitude  = sp::from360(latitude360);
	auto longitude = sp::from360(longitude360);
	auto daysAfterY2K = sp::date2DaysAfterY2K(date);
	auto sunPosition = sp::compute_sun_position(daysAfterY2K, latitude, longitude);
	auto altitudeAngle360 = sp::to360(sunPosition.altitudeAngle);
	auto azimuthAngle360 = sp::to360(sunPosition.azimuthAngle);
	std::println("{0:%Y-%m-%d %T}\t{1: {5}.{6}f}\t{2: {5}.{6}f}\t{3: {5}.{6}f}\t{4:{5}.{6}f}", date, latitude360, longitude360, altitudeAngle360, azimuthAngle360, MAX_DIGITS, MAX_DIGITS - 4);
}

static std::chrono::sys_seconds now() {
	return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

template<typename T> static std::expected<T, std::nullptr_t> floatFromString(std::string_view string) {
	auto f = T();
	auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), f);
	if (ec != std::errc()) {
	    return std::unexpected(nullptr);
	}
	return f;
}

int main(int argc, char *argv[]) {
	argparse::ArgumentParser program("sunpos");
	program.add_description("Calculate the position of the sun");
	auto latitude360String = std::string();
	auto longitude360String = std::string();
	auto dateString = std::string();
	auto header = false;
	auto track = false;
	program.add_argument("latitude").help("our latitude").store_into(latitude360String);
	program.add_argument("longitude").help("our longitude").store_into(longitude360String);
	program.add_argument("-d", "--date").help("specify the UTC date as \"YYYY-MM-DD HH:MM:SS\"").metavar("DATE").store_into(dateString);
	program.add_argument("-h", "--header").help("display table header").flag().store_into(header);
	program.add_argument("-t", "--track").help("track the current time").flag().store_into(track);

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::println(std::cerr, "{}", err.what());
		std::println(std::cerr, "{}", program.help().str());
		return 1;
	}

	auto latitude360  = floatFromString<sp::floating_point_t>(latitude360String).value_or(0);
	auto longitude360 = floatFromString<sp::floating_point_t>(longitude360String).value_or(0);
	if (header) {
		std::println("{0:19}\t{1:{5}}\t{2:{5}}\t{3:{5}}\t{4:{5}}", "UTC Date", "Latitude", "Longitude", "Altitude", "Azimuth", MAX_DIGITS);
	}
	if (track) {
		while (true) {
			auto date = now();
			printSunPosition(date, latitude360, longitude360);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	} else {
		auto date = readDate(dateString, DATE_FORMAT).value_or(now());
		printSunPosition(date, latitude360, longitude360);
	}

	return 0;
}
