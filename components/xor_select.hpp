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
	shift = key & 16;
	word = ((word & 0x0000ffff0000ffffULL) << shift) | ((word & 0xffff0000ffff0000ULL) >> shift);
	shift = key & 8;
	word = ((word & 0x00ff00ff00ff00ffULL) << shift) | ((word & 0xff00ff00ff00ff00ULL) >> shift);
	shift = key & 4;
	word = ((word & 0x0f0f0f0f0f0f0f0fULL) << shift) | ((word & 0xf0f0f0f0f0f0f0f0ULL) >> shift);
	shift = key & 2;
	word = ((word & 0x3333333333333333ULL) << shift) | ((word & 0xccccccccccccccccULL) >> shift);
	shift = key & 1;
	word = ((word & 0x5555555555555555ULL) << shift) | ((word & 0xaaaaaaaaaaaaaaaaULL) >> shift);
	return std::countr_zero(word) ^ key;
}
}

#endif
