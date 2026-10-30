#include "components/xor_select.hpp"
#include <cassert>
#include <cstdint>
#include <random>

void check(std::uint64_t word) {
	for (unsigned key = 0; key < 64; ++key) {
		unsigned best = 64;
		for (unsigned p = 0; p < 64; ++p)
			if ((word >> p) & 1)
				if (best == 64 || (p ^ key) < (best ^ key)) best = p;
		assert(xor_select_detail::argmin(word, key) == best);
	}
}

int main() {
	for (unsigned word = 1; word < 65536; ++word) {
		check(word);
		check(std::uint64_t(word) << 48);
	}
	std::mt19937_64 random(42);
	for (int i = 0; i < 10000; ++i) check(random() | 1);
	check(~std::uint64_t(0));
}
