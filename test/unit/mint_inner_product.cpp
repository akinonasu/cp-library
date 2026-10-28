#include "components/mint_inner_product.hpp"
#include <random>
#include <vector>

template<bool Reverse> void check(const mint* a, const mint* b, int size) {
	__uint128_t sum = 0;
	mint expected(0);
	for (int i = 0; i < size; ++i) {
		sum += std::uint64_t(a[i].v) * b[Reverse ? -i : i].v;
		expected += a[i] * b[Reverse ? -i : i];
	}
	assert(mint_detail::inner_product_scalar<Reverse>(a, b, size) == sum);
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
	if (mint_detail::inner_product_has_avx2()) assert(mint_detail::inner_product_avx2<Reverse>(a, b, size) == sum);
	if (mint_detail::inner_product_has_avx512()) assert(mint_detail::inner_product_avx512<Reverse>(a, b, size) == sum);
#endif
	assert(mint_detail::inner_product<Reverse>(a, b, size) == expected);
}

int main() {
	constexpr int limit = 16384;
	std::vector<mint> a(limit + 64), b(limit + 64);
	std::mt19937_64 rng(20261026);
	for (int mode = 0; mode < 3; ++mode) {
		for (std::size_t i = 0; i < a.size(); ++i) {
			unsigned x = mode == 0 ? rng() % (2 * mint::mod()) : 2 * mint::mod() - 1;
			unsigned y = mode == 0 ? rng() % (2 * mint::mod()) : 2 * mint::mod() - 1;
			if (mode == 2) {
				constexpr unsigned values[] = {0, 1, mint::mod() - 1, mint::mod(), 2 * mint::mod() - 1};
				x = values[i % 5], y = values[(i + 1) % 5];
			}
			a[i] = mint::word(x), b[i] = mint::word(y);
		}
		for (int size = 0; size <= 2048; ++size) {
			check<false>(a.data() + 17, b.data() + 19, size);
			check<true>(a.data() + 17, b.data() + 32 + size, size);
		}
		for (int size : {4095, 4096, 4097, limit - 1, limit}) {
			check<false>(a.data() + 1, b.data() + 3, size);
			check<true>(a.data() + 1, b.data() + 32 + size, size);
		}
	}
}
