#include "math/number_theory/prime_count.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>

std::uint64_t baseline(std::uint64_t n) {
	if (n < 2) return 0;
	std::uint64_t root = std::sqrt(static_cast<long double>(n));
	while ((root + 1) * (root + 1) <= n) ++root;
	while (root * root > n) --root;
	std::vector<std::uint64_t> small(root + 1), large(root + 1);
	for (std::uint64_t i = 1; i <= root; ++i) small[i] = i - 1, large[i] = n / i - 1;
	for (std::uint64_t p = 2; p <= root; ++p) {
		if (small[p] == small[p - 1]) continue;
		const auto before = small[p - 1], square = p * p;
		const auto end = std::min(root, n / square);
		for (std::uint64_t i = 1; i <= end; ++i) {
			const auto x = n / (i * p);
			large[i] -= (x <= root ? small[x] : large[i * p]) - before;
		}
		for (auto i = root; i >= square; --i) small[i] -= small[i / p] - before;
	}
	return large[1];
}

int main(int argc, char** argv) {
	if (argc != 3) return 1;
	const bool simple = argv[1][0] == 'b';
	const auto n = std::strtoull(argv[2], nullptr, 10);
	const auto start = std::chrono::steady_clock::now();
	const auto result = simple ? baseline(n) : prime_count(n);
	const auto stop = std::chrono::steady_clock::now();
	std::printf("%llu %.6f\n", static_cast<unsigned long long>(result),
		std::chrono::duration<double, std::milli>(stop - start).count());
}
