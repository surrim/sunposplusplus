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

static std::expected<std::chrono::sys_seconds, nullptr_t> readDate(std::string_view dateString, std::string_view dateFormat) {
    auto stream = std::ispanstream(dateString);
    auto timepoint = std::chrono::sys_seconds();
    if (std::chrono::from_stream(stream, dateFormat.data(), timepoint)) {
        return timepoint;
    }
    return std::unexpected(nullptr);
}

static void printSunPosition(std::chrono::sys_seconds date, auto latitude360, auto longitude360) {
	auto latitude  = sp::from360(latitude360);
	auto longitude = sp::from360(longitude360);
	auto daysAfterY2K = sp::date2DaysAfterY2K(date);
	auto sunPosition = sp::compute_sun_position(daysAfterY2K, latitude, longitude);
	auto zenithAngle360 = sp::to360(sunPosition.zenithAngle);
	auto azimuthAngle360 = sp::to360(sunPosition.azimuthAngle);
	std::println("{:%Y-%m-%d %T}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", date, latitude360, longitude360, zenithAngle360, azimuthAngle360);
}

int main(int argc, char *argv[]) {
	argparse::ArgumentParser program("sunpos");
	program.add_description("Calculate the position of the sun");
	auto latitude360  = 0.0;
	auto longitude360 = 0.0;
	auto dateString = std::string();
	auto track = false;
	program.add_argument("latitude").help("our latitude").store_into(latitude360);
	program.add_argument("longitude").help("our longitude").store_into(longitude360);
	program.add_argument("-d", "--date").help("specify the UTC date as \"YYYY-MM-DD HH:MM:SS\"").metavar("DATE").store_into(dateString);
	program.add_argument("-t", "--track").help("track the current time").flag().store_into(track);

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::println(std::cerr, "{}", err.what());
		std::println(std::cerr, "{}", program.help().str());
		return 1;
	}

	if (track) {
		while (true) {
			auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
			printSunPosition(date, latitude360, longitude360);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	} else {
		auto date = readDate(dateString, DATE_FORMAT).value_or(std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
		printSunPosition(date, latitude360, longitude360);
	}

	return 0;
}
