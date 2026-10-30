#ifndef XOR_SELECT_HPP
#define XOR_SELECT_HPP

#include <bit>
#include <cassert>
#include <cstdint>

namespace xor_select_detail {
inline unsigned argmin(std::uint64_t word, unsigned key) {
	assert(word && key < 64);
	if (std::has_single_bit(word)) return std::countr_zero(word);
	if ((word >> key) & 1) return key;
	unsigned shift = key & 32;
	word = std::rotl(word, shift);
	auto exchange = [&](unsigned shift, std::uint64_t mask) {
		word = ((word & mask) << shift) | ((word & ~mask) >> shift);
	};
	exchange(key & 16, 0x0000ffff0000ffffULL);
	exchange(key & 8, 0x00ff00ff00ff00ffULL);
	exchange(key & 4, 0x0f0f0f0f0f0f0f0fULL);
	exchange(key & 2, 0x3333333333333333ULL);
	exchange(key & 1, 0x5555555555555555ULL);
	return std::countr_zero(word) ^ key;
}
}

#endif
