#include "math/util/integer_division.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

using I = std::int64_t;
using W = boost::multiprecision::int256_t;

static_assert(floor_div(-7, 3) == -3 && ceil_div(-7, 3) == -2);
static_assert(floor_div(7, -3) == -3 && ceil_div(7, -3) == -2);
static_assert(floor_div(-7, -3) == 2 && ceil_div(-7, -3) == 3);
static_assert(ceil_div(10u, 3u) == 4);
static_assert(std::is_same_v<decltype(floor_div(I(7), 3)), I>);
static_assert(floor_div(__int128_t(-7), __int128_t(3)) == -3);

template <class T>
void check(T a, T b) {
	if (!b) return;
	if constexpr (std::numeric_limits<T>::is_signed) {
		if (a == std::numeric_limits<T>::min() && b == -1) {
			bool caught = false;
			try { (void)floor_div(a, b); } catch (const std::overflow_error &) { caught = true; }
			assert(caught);
			caught = false;
			try { (void)ceil_div(a, b); } catch (const std::overflow_error &) { caught = true; }
			assert(caught);
			return;
		}
	}
	W x = a, y = b;
	if (y < 0) x = -x, y = -y;
	W lo = floor_div(a, b), hi = ceil_div(a, b);
	assert(lo * y <= x && x < (lo + 1) * y);
	assert((hi - 1) * y < x && x <= hi * y);
}

int main() {
	for (int a = -256; a <= 256; ++a) for (int b = -256; b <= 256; ++b) check(a, b);
	for (int a = -128; a <= 127; ++a) for (int b = -128; b <= 127; ++b) check(static_cast<signed char>(a), static_cast<signed char>(b));
	const I low = std::numeric_limits<I>::min(), high = std::numeric_limits<I>::max();
	std::array<I, 9> values{low, low + 1, -3, -1, 0, 1, 3, high - 1, high};
	for (I a : values) for (I b : values) check(a, b);
	std::array<__int128_t, 7> large{std::numeric_limits<__int128_t>::min(), __int128_t(low) * high, -1, 0, 1, __int128_t(high) * high, std::numeric_limits<__int128_t>::max()};
	for (auto a : large) for (auto b : large) check(a, b);
	std::mt19937_64 random(141101);
	for (int t = 0; t < 20000; ++t) {
		check(std::bit_cast<I>(random()), std::bit_cast<I>(random()));
		check(random(), random());
		check((__uint128_t(random()) << 64) | random(), (__uint128_t(random()) << 64) | random());
	}
	for (bool ceil : {false, true}) {
		bool caught = false;
		try { (void)(ceil ? ceil_div(0, 0) : floor_div(0, 0)); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	assert(ceil_div(std::numeric_limits<std::uint64_t>::max(), std::uint64_t(2)) == (std::uint64_t(1) << 63));
}
