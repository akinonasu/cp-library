#ifndef KTH_ROOT_HPP
#define KTH_ROOT_HPP

#include <array>
#include <algorithm>
#include <limits>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace kth_root_detail {
inline constexpr auto thresholds = [] {
	std::array<std::array<std::uint64_t, 13>, 25> table{};
	for (unsigned k = 16; k <= 40; ++k) for (unsigned a = 3; a <= 15; ++a) {
		std::uint64_t p = 1;
		for (unsigned i = 0; i < k; ++i) {
			if (p > UINT64_MAX / a) { p = 0; break; }
			p *= a;
		}
		table[k - 16][a - 3] = p - 1;
	}
	return table;
}();

inline std::uint64_t cube(std::uint64_t n) {
	if constexpr (std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53 && sizeof(double) == sizeof(std::uint64_t)) {
		static constexpr auto inverse_roots = [] {
			std::array<double, 64> result{};
			for (int i = 0; i < 64; ++i) {
				double a = 1 + (i + 0.5) / 64, x = 1;
				for (int j = 0; j < 8; ++j) x += x * (1 - a * x * x * x) / 3;
				result[i] = x;
			}
			return result;
		}();
		double a = double(n);
		std::uint64_t bits = std::bit_cast<std::uint64_t>(a);
		unsigned exponent = unsigned(bits >> 52) - 1023;
		constexpr double factor[] = {1, 0.79370052598409973738, 0.62996052494743658238};
		double x = inverse_roots[(bits >> 46) & 63] * factor[exponent % 3]
			* std::bit_cast<double>(std::uint64_t(1023 - exponent / 3) << 52);
		{
			double h = 1 - a * x * x * x;
			x += x * h * (1.0 / 3 + h * (2.0 / 9 + h * (14.0 / 81)));
		}
		std::uint64_t r = std::min(std::uint64_t(1 / x), std::uint64_t(2642245));
		while (r * r * r > n) --r;
		while (r < 2642245 && (r + 1) * (r + 1) * (r + 1) <= n) ++r;
		return r;
	} else {
		std::uint64_t r = std::cbrt(double(n));
		while (__uint128_t(r) * r * r > n) --r;
		while (__uint128_t(r + 1) * (r + 1) * (r + 1) <= n) ++r;
		return r;
	}
}

template <unsigned K>
inline std::uint64_t small(std::uint64_t n) {
	auto leq = [n](std::uint64_t x) {
		std::uint64_t a = x, r = 1;
		for (unsigned k = K;;) {
			if ((k & 1) && __builtin_mul_overflow(r, a, &r)) return false;
			if (!(k >>= 1)) return r <= n;
			if (__builtin_mul_overflow(a, a, &a)) return false;
		}
	};
	std::uint64_t lo = 1, hi = std::uint64_t(1) << ((std::bit_width(n) - 1) / K + 1);
	double estimate;
	if constexpr (K == 4) estimate = std::sqrt(std::sqrt(double(n)));
	else if constexpr (K == 8) estimate = std::sqrt(std::sqrt(std::sqrt(double(n))));
	else estimate = std::pow(double(n), 1.0 / K);
	std::uint64_t r = std::uint64_t(estimate);
	if (r >= hi) r = hi - 1;
	if (r < lo) r = lo;
	if (leq(r)) {
		if (!leq(r + 1)) return r;
		lo = r + 1;
	} else {
		if (leq(r - 1)) return r - 1;
		hi = r - 1;
	}
	while (lo + 1 < hi) {
		std::uint64_t mid = (lo + hi) / 2;
		if (leq(mid)) lo = mid;
		else hi = mid;
	}
	return lo;
}
}

inline std::uint64_t kth_root(std::uint64_t n, std::uint64_t k) {
	assert(k > 0);
	if (k == 1 || n <= 1) return n;
	if (k >= unsigned(std::bit_width(n))) return 1;
	if (k == 2) {
		std::uint64_t r = std::sqrt(double(n));
		if (__uint128_t(r) * r > n) --r;
		if (__uint128_t(r + 1) * (r + 1) <= n) ++r;
		return r;
	}
	if (k >= 16) {
		if (k > 40) return 2;
		unsigned lo = 3, hi = 1U << (63 / k + 1);
		while (lo < hi) {
			unsigned mid = (lo + hi) / 2;
			if (n > kth_root_detail::thresholds[k - 16][mid - 3]) lo = mid + 1;
			else hi = mid;
		}
		return lo - 1;
	}
	switch (k) {
	case 3: return kth_root_detail::cube(n);
	case 4: return kth_root_detail::small<4>(n);
	case 5: return kth_root_detail::small<5>(n);
	case 6: return kth_root_detail::small<6>(n);
	case 7: return kth_root_detail::small<7>(n);
	case 8: return kth_root_detail::small<8>(n);
	case 9: return kth_root_detail::small<9>(n);
	case 10: return kth_root_detail::small<10>(n);
	case 11: return kth_root_detail::small<11>(n);
	case 12: return kth_root_detail::small<12>(n);
	case 13: return kth_root_detail::small<13>(n);
	case 14: return kth_root_detail::small<14>(n);
	default: return kth_root_detail::small<15>(n);
	}
}

#endif
