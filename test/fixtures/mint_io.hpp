#ifndef TEST_MINT_IO_HPP
#define TEST_MINT_IO_HPP

#include "components/fast_io_bulk.hpp"
#include "math/mint.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

inline void read_mints(FastInput &input, std::span<mint> values) {
	std::size_t offset = 0;
	read_unsigned_blocks(input, values.size(), [&](const std::uint32_t *block, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) values[offset + i] = mint::raw(block[i]);
		offset += count;
	});
}

inline void write_mints(FastOutput &output, std::span<const mint> values) {
	write_unsigned_records<1>(output, values.size(), [&](std::size_t i) {
		return std::array{values[i].val()};
	});
}

#endif
