#ifndef FAST_IO_OUTPUT_VALUES_HPP
#define FAST_IO_OUTPUT_VALUES_HPP

#include "fast_io_output.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace fast_io_detail {
struct UnsignedValues {
	static char *padded8(char *out, std::uint32_t value) {
		char digits[16];
		BulkOutput::number(digits, value + 100000000);
		std::memcpy(out, digits + 1, 8);
		return out + 8;
	}
	static char *number(char *out, std::uint64_t value) {
		constexpr std::uint64_t block = 100000000;
		if (value >= block * block) {
			out = BulkOutput::small(out, std::uint32_t(value / (block * block)));
			value %= block * block;
			out = padded8(out, std::uint32_t(value / block));
			return padded8(out, std::uint32_t(value % block));
		}
		if (value >= block) {
			out = BulkOutput::number(out, std::uint32_t(value / block));
			return padded8(out, std::uint32_t(value % block));
		}
		return BulkOutput::number(out, std::uint32_t(value));
	}
	template<class UInt, class Read> struct Block {
		std::size_t base, count;
		Read &read;
		char separator, ending;
		std::size_t decimal_capacity() const { return count * (sizeof(UInt) <= 4 ? 11 : 21); }
		char *to_chars(char *out) const {
			char *p = out;
			for (std::size_t i = 0; i < count; ++i) {
				if constexpr (sizeof(UInt) <= 4) p = BulkOutput::number(p, std::uint32_t(read(base + i)));
				else p = number(p, std::uint64_t(read(base + i)));
				*p++ = separator;
			}
			p[-1] = ending;
			return p;
		}
	};
};
}

template<class UInt = std::uint64_t, class Read>
void write_unsigned_values(FastOutput &output, std::size_t count, Read read, char separator = ' ', char ending = '\n') {
	static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && sizeof(UInt) <= 8 && !std::is_same_v<UInt, bool>);
	constexpr std::size_t block = (1 << 16) / (sizeof(UInt) <= 4 ? 11 : 21);
	for (std::size_t base = 0; base < count;) {
		const auto n = std::min(count - base, block);
		output.write(fast_io_detail::UnsignedValues::Block<UInt, Read>{base, n, read, separator, base + n == count ? ending : separator});
		base += n;
	}
}

#endif
