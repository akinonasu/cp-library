#ifndef XOR_BASIS_CORE_HPP
#define XOR_BASIS_CORE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace xor_basis_detail {
template <class UInt, bool maximum = false>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
inline UInt reduce_rows(UInt x, const UInt *row, int count) {
	int i = 0;
	auto step = [&](UInt value) {
		if constexpr (maximum) x = std::max(x, UInt(x ^ value));
		else x = std::min(x, UInt(x ^ value));
	};
	for (; i + 4 <= count; i += 4) {
		step(row[i]); step(row[i + 1]); step(row[i + 2]); step(row[i + 3]);
	}
	for (; i < count; ++i) step(row[i]);
	return x;
}

template <class UInt, int Bits>
struct Core {
	static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && !std::is_same_v<UInt, bool>);
	static constexpr int digits = std::numeric_limits<UInt>::digits;
	static_assert(0 < Bits && Bits <= digits && digits <= 64);
	static constexpr UInt mask = UInt(UInt(~UInt(0)) >> (digits - Bits));
	std::array<UInt, Bits> row{};
	std::array<std::uint8_t, Bits> at{};
	int dimension = 0;
	static bool valid(UInt x) { return (x & mask) == x; }
	static int pivot(UInt x) { return int(std::bit_width(std::uint64_t(x))) - 1; }
	int insert(UInt x) {
		assert(valid(x));
		if (dimension == Bits) return -1;
		x = reduce_rows(x, row.data(), dimension);
		if (!x) return -1;
		row[dimension] = x; at[pivot(x)] = std::uint8_t(dimension + 1);
		return dimension++;
	}
	UInt minimize(UInt x) const {
		if (dimension == Bits) return UInt(x & UInt(~mask));
		x = reduce_rows(x, row.data(), dimension);
		return x;
	}
	UInt maximize(UInt x) const {
		if (dimension == Bits) return UInt(x | mask);
		x = reduce_rows<UInt, true>(x, row.data(), dimension);
		return x;
	}
};
}

#endif
