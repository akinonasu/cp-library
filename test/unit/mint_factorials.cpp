#include "components/mint_factorials.hpp"
#include <cstdint>
#include <vector>

int main() {
	for (int n : {0, 1, 7, 255, 256, 257, 511, 512, 513, 4095, 4096, 4097, 65537}) {
		std::vector<mint> fact(n + 2, mint(-1)), invfact(n + 2, mint(-1));
		mint_detail::factorials(fact.data() + 1, invfact.data() + 1, n);
		std::uint64_t expected = 1;
		for (int i = 0; i < n; ++i) {
			if (i) expected = expected * i % mint::mod();
			assert(fact[i + 1].val() == expected && fact[i + 1] * invfact[i + 1] == mint(1));
		}
		assert(fact.front() == mint(-1) && fact.back() == mint(-1));
		assert(invfact.front() == mint(-1) && invfact.back() == mint(-1));
	}
}
