#include "components/ordered_midpoint.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <utility>

static_assert(ordered_midpoint(-8LL, 3LL) == -3);
static_assert(ordered_midpoint<std::int8_t>(-128, 127) == -1);
static_assert(ordered_midpoint<std::uint8_t>(0, 255) == 127);
static_assert(ordered_midpoint(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()) == -1);
static_assert(ordered_midpoint(std::uint64_t(0), std::numeric_limits<std::uint64_t>::max()) == std::numeric_limits<std::uint64_t>::max() / 2);

template<class T>
void small() {
	for (int l = std::numeric_limits<T>::min(); l <= std::numeric_limits<T>::max(); ++l)
		for (int r = l; r <= std::numeric_limits<T>::max(); ++r)
			assert(ordered_midpoint(T(l), T(r)) == std::midpoint(T(l), T(r)));
}

template<class T>
void wide() {
	constexpr T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	std::array<T, 7> values{low, T(low + 1), T(low / 2), 0, T(high / 2), T(high - 1), high};
	for (T l : values) for (T r : values) if (l <= r) assert(ordered_midpoint(l, r) == std::midpoint(l, r));
	std::mt19937_64 rng(978482);
	for (int i = 0; i < 100000; ++i) {
		T l = T(rng()), r = T(rng());
		if (r < l) std::swap(l, r);
		assert(ordered_midpoint(l, r) == std::midpoint(l, r));
	}
}

int main() {
	small<std::int8_t>();
	small<std::uint8_t>();
	wide<std::int16_t>();
	wide<std::uint16_t>();
	wide<std::int32_t>();
	wide<std::uint32_t>();
	wide<std::int64_t>();
	wide<std::uint64_t>();
}
