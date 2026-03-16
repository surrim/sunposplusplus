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
#include <chrono>
#include <expected>
#include <print>
#include <spanstream>
#include <thread>
#include <argparse/argparse.hpp>

static constexpr auto DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

static std::expected<std::chrono::sys_seconds, nullptr_t> read_date(std::string_view date_string, std::string_view date_format) {
    auto stream = std::ispanstream(date_string);
    auto timepoint = std::chrono::sys_seconds();
    if (std::chrono::from_stream(stream, std::string(date_format).c_str(), timepoint)) {
        return timepoint;
    }
    return std::unexpected(nullptr);
}

void print_sun_position(std::chrono::sys_seconds date, double latitude_degrees, double longitude_degrees) {
	const auto latitude  = sc::floating_point_t(latitude_degrees  * sc::D2R);
	const auto longitude = sc::floating_point_t(longitude_degrees * sc::D2R);
	const auto sun_position = sc::compute_sun_position(date, latitude, longitude);
	std::println("{:%Y-%m-%d %T}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.6f}", date, latitude_degrees, longitude_degrees, sun_position.zenithAngle * sc::R2D, sun_position.azimuthAngle * sc::R2D);
}

int main(int argc, char *argv[]) {
	argparse::ArgumentParser program("sunpos");
	program.add_description("Calculate the position of the sun");
	auto latitude_degrees  = 0.0;
	auto longitude_degrees = 0.0;
	auto date_string = std::string();
	auto track = false;
	program.add_argument("latitude").help("our latitude").store_into(latitude_degrees);
	program.add_argument("longitude").help("our longitude").store_into(longitude_degrees);
	program.add_argument("-d", "--date").help("specify the UTC date as \"YYYY-MM-DD HH:MM:SS\"").metavar("DATE").store_into(date_string);
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
			const auto date = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
			print_sun_position(date, latitude_degrees, longitude_degrees);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	} else {
		const auto date = read_date(date_string, DATE_FORMAT).value_or(std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
		print_sun_position(date, latitude_degrees, longitude_degrees);
	}

	return 0;
}
