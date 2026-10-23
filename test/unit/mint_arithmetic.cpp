#include "components/mint_arithmetic.hpp"
#include <array>
#include <random>

using Ops = mint_detail::Ops;
using Word = Ops::U;

#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx2"))) void check_vectors(const Word *a, const Word *b, unsigned multiplier) {
	alignas(32) Word result[8];
	auto x = Ops::load8(a), y = Ops::load8(b);
	Ops::store8(result, Ops::mul8(x, y));
	for (int i = 0; i < 8; ++i) assert(mint::word(result[i]) == mint::word(a[i]) * mint::word(b[i]));
	Ops::store8(result, Ops::add8(x, y));
	for (int i = 0; i < 8; ++i) assert(mint::word(result[i]) == mint::word(a[i]) + mint::word(b[i]));
	Ops::store8(result, Ops::sub8(x, y));
	for (int i = 0; i < 8; ++i) assert(mint::word(result[i]) == mint::word(a[i]) - mint::word(b[i]));
	auto factor = _mm256_set1_epi32(multiplier), quotient = _mm256_set1_epi32(Ops::quotient(multiplier));
	Ops::store8(result, Ops::fixed_mul8(x, factor, quotient));
	for (int i = 0; i < 8; ++i) assert(mint::word(result[i]) == mint::word(a[i]) * mint(multiplier));
	Ops::store8(result, Ops::fixed_even8(x, factor, quotient));
	for (int i = 0; i < 8; i += 2) assert(mint::word(result[i]) == mint::word(a[i]) * mint(multiplier));
}
#endif

int main() {
	std::mt19937 random(20261023);
	for (int iteration = 0; iteration < 10000; ++iteration) {
		alignas(32) Word a[8], b[8];
		unsigned multiplier = random() % mint::mod();
		for (int i = 0; i < 8; ++i) {
			a[i] = random() % Ops::twice;
			b[i] = random() % Ops::twice;
			assert(mint::word(Ops::mul(a[i], b[i])) == mint::word(a[i]) * mint::word(b[i]));
			assert(mint::word(Ops::add(a[i], b[i])) == mint::word(a[i]) + mint::word(b[i]));
			assert(mint::word(Ops::sub(a[i], b[i])) == mint::word(a[i]) - mint::word(b[i]));
			assert(mint::word(Ops::fixed_mul(a[i], multiplier, Ops::quotient(multiplier))) == mint::word(a[i]) * mint(multiplier));
			assert(Ops::out(Ops::in(a[i])) == a[i] % mint::mod());
		}
#if defined(__x86_64__) || defined(__i386__)
		if (Ops::vectorized()) check_vectors(a, b, multiplier);
#endif
	}
}
