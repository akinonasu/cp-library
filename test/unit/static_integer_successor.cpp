#include "components/static_integer_successor.hpp"
#include <cassert>
#include <random>
#include <vector>
int main() {
	std::mt19937_64 rng(675321); static_integer_successor_detail::Index index;
	unsigned group = 0;
	for (int bits : {8, 16, 32, 64}) for (int n : {0, 1, 7, 8, 9, 16, 32, 63, 64, 65, 129, 255, 1000, 10000}) {
		if (bits == 8 && n > 255) continue;
		std::vector<std::uint64_t> a;
		while (int(a.size()) < n) {
			int missing = n - int(a.size());
			while (missing--) { auto x = rng(); if (bits < 64) x &= (std::uint64_t(1) << bits) - 1; a.push_back(x); }
			std::sort(a.begin(), a.end()); a.erase(std::unique(a.begin(), a.end()), a.end());
		}
		auto stride = index.add(group, a, bits);
		for (int q = 0; q < 1000; ++q) {
			auto x = q % 4 == 0 ? 0 : q % 4 == 1 ? UINT64_MAX : q % 4 == 2 && n ? a[rng() % n] : rng();
			if (bits < 64) x &= (std::uint64_t(1) << bits) - 1;
			assert(index.lower_bound<false>(group, a, bits, stride, x) == int(std::lower_bound(a.begin(), a.end(), x) - a.begin()));
			assert(index.lower_bound<true>(group, a, bits, stride, x) == int(std::upper_bound(a.begin(), a.end(), x) - a.begin()));
		}
		++group;
	}
	for (int bits : {16, 32, 64}) {
		std::vector<std::uint64_t> a(1025);
		for (int i = 0; i < 1024; ++i) a[i] = i;
		a.back() = bits == 64 ? UINT64_MAX : (std::uint64_t(1) << bits) - 1;
		auto stride = index.add(group, a, bits);
		for (std::uint64_t x = 0; x < 1100; ++x) {
			assert(index.lower_bound<false>(group, a, bits, stride, x) == int(std::lower_bound(a.begin(), a.end(), x) - a.begin()));
			assert(index.lower_bound<true>(group, a, bits, stride, x) == int(std::upper_bound(a.begin(), a.end(), x) - a.begin()));
		}
		++group;
	}

}
