#include "math/kth_root.hpp"
#include "components/fast_io.hpp"

#ifndef KTH_ROOT_VARIANT
#define KTH_ROOT_VARIANT 0
#endif

[[maybe_unused]] bool power_leq(std::uint64_t a, unsigned k, std::uint64_t n) {
	std::uint64_t r = 1;
	for (;;) {
		if ((k & 1) && __builtin_mul_overflow(r, a, &r)) return false;
		if (!(k >>= 1)) return r <= n;
		if (__builtin_mul_overflow(a, a, &a)) return false;
	}
}

std::uint64_t solve(std::uint64_t n, unsigned k) {
#if KTH_ROOT_VARIANT == 0
	return kth_root(n, k);
#else
	if (k == 1 || n <= 1) return n;
	if (k >= 64) return 1;
#if KTH_ROOT_VARIANT == 1
	std::uint64_t lo = 1, hi = std::uint64_t(1) << ((std::bit_width(n) - 1) / k + 1);
	while (lo + 1 < hi) {
		std::uint64_t mid = (lo + hi) / 2;
		if (power_leq(mid, k, n)) lo = mid;
		else hi = mid;
	}
	return lo;
#else
	std::uint64_t r = std::pow(double(n), 1.0 / double(k));
	while (!power_leq(r, k, n)) --r;
	while (power_leq(r + 1, k, n)) ++r;
	return r;
#endif
#endif
}

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t n = 0;
		unsigned k = 0;
		input.read(n), input.read(k);
		output.println(solve(n, k));
	}
}
