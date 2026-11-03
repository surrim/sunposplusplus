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

#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <stdfloat>

#ifndef SP_FLOATING_POINT_TYPE
#define SP_FLOATING_POINT_TYPE std::float32_t
#endif

namespace sp {
	using floating_point_t = SP_FLOATING_POINT_TYPE;
	static constexpr auto PI = std::numbers::pi_v<floating_point_t>;
	static constexpr auto TWO_PI = 2 * PI;

	template<typename T = floating_point_t>
	[[nodiscard]] static constexpr inline T to360(floating_point_t r) {
		return T(360 / TWO_PI * r);
	}

	[[nodiscard]] static constexpr inline floating_point_t from360(auto d) {
		return floating_point_t(TWO_PI / 360 * d);
	}

	template<typename T = uint8_t>
	[[nodiscard]] static constexpr inline T to256(floating_point_t r) {
		return T(std::round(256 / TWO_PI * r));
	}

	[[nodiscard]] static constexpr inline floating_point_t from256(auto b) {
		return floating_point_t(TWO_PI / 256 * b);
	}

	[[nodiscard]] int32_t date2DaysAfterY2K(int16_t year, uint8_t month, uint8_t day);
	[[nodiscard]] floating_point_t date2DaysAfterY2K(int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute = 0, uint8_t second = 0);
	[[nodiscard]] floating_point_t date2DaysAfterY2K(uint32_t unixTime);
	[[nodiscard]] floating_point_t date2DaysAfterY2K(std::chrono::sys_seconds date);

	struct sun_position_t {
		floating_point_t zenithAngle;
		floating_point_t azimuthAngle;
	};

	[[nodiscard]] sun_position_t compute_sun_position(floating_point_t daysAfterY2K, floating_point_t latitude, floating_point_t longitude);
}
