#include "components/bitset_ops.hpp"
#include <array>
#include <cassert>
#include <random>
#include <vector>

using Word = std::uint64_t;
using Op = bitset_ops_detail::Operation;

std::size_t bits(Word x) {
	std::size_t result = 0;
	for (int i = 0; i < 64; ++i) result += (x >> i) & 1;
	return result;
}

template <Op Operation> void check(const Word *a, const Word *b, std::size_t n) {
	std::size_t expected = 0;
	std::vector<Word> mutation(a, a + n), self(a, a + n), expected_mutation(n), expected_self(n);
	for (std::size_t i = 0; i < n; ++i) {
		Word x = a[i], y = b[i];
		if constexpr (Operation == Op::bit_and) x &= y;
		if constexpr (Operation == Op::bit_or) x |= y;
		if constexpr (Operation == Op::bit_xor) x ^= y;
		if constexpr (Operation == Op::and_not) x &= ~y;
		expected_mutation[i] = x;
		expected += bits(x);
		expected_self[i] = Operation == Op::bit_xor || Operation == Op::and_not ? 0 : a[i];
	}
	const Word *second = Operation == Op::identity ? nullptr : b;
	assert(bitset_ops_detail::count<Operation>(a, second, n) == expected);
	assert(bitset_ops_detail::scalar<Operation>(a, second, n) == expected);
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (bitset_ops_detail::has_avx512()) assert(bitset_ops_detail::avx512<Operation>(a, second, n) == expected);
	if (bitset_ops_detail::has_avx2()) assert(bitset_ops_detail::avx2<Operation>(a, second, n) == expected);
#endif
	if constexpr (Operation != Op::identity) {
		bitset_ops_detail::apply<Operation>(mutation.data(), b, n);
		bitset_ops_detail::apply<Operation>(self.data(), self.data(), n);
		assert(mutation == expected_mutation && self == expected_self);
	}
}

void check_all(const Word *a, const Word *b, std::size_t n) {
	check<Op::identity>(a, b, n);
	check<Op::bit_and>(a, b, n);
	check<Op::bit_or>(a, b, n);
	check<Op::bit_xor>(a, b, n);
	check<Op::and_not>(a, b, n);
}

int main() {
	assert(bitset_ops_detail::count<Op::identity>(nullptr, nullptr, 0) == 0);
	assert(bitset_ops_detail::count<Op::bit_and>(nullptr, nullptr, 0) == 0);
	bitset_ops_detail::apply<Op::bit_xor>(nullptr, nullptr, 0);
	std::mt19937_64 rng(782145333);
	for (std::size_t n = 0; n < 140; ++n) for (int pattern = 0; pattern < 4; ++pattern) {
		std::vector<Word> a(n + 8), b(n + 8);
		for (auto &x : a) x = pattern == 0 ? 0 : pattern == 1 ? ~Word(0) : rng();
		for (auto &x : b) x = pattern == 0 ? ~Word(0) : pattern == 1 ? 0 : rng();
		for (int offset = 0; offset < 4; ++offset) check_all(a.data() + offset, b.data() + 3 - offset, n);
	}
	for (int n : {1023, 1024, 1025, 65537}) {
		std::vector<Word> a(n, ~Word(0)), b(n, ~Word(0));
		check_all(a.data(), b.data(), n);
		for (auto &x : a) x = rng();
		for (auto &x : b) x = rng();
		check_all(a.data(), b.data(), n);
	}
}
