#include "source/components/fast_io.hpp"
#include <array>
#include <bit>
#include <cstring>

namespace candidate_output {
inline constexpr auto quads = [] {
	std::array<std::array<char, 4>, 10000> table{};
	for (int i = 0; i < 10000; ++i)
		table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
	return table;
}();
inline char *small(char *out, std::uint32_t x) {
	unsigned n = 1 + (x >= 10) + (x >= 100) + (x >= 1000);
	if constexpr (std::endian::native == std::endian::little) {
		std::uint32_t word;
		std::memcpy(&word, quads[x].data(), 4);
		word >>= (4 - n) * 8;
		std::memcpy(out, &word, 4);
	} else std::memcpy(out, quads[x].data() + 4 - n, n);
	return out + n;
}
inline char *number(char *out, std::uint32_t x) {
	if (x >= 100000000) {
		out = small(out, x / 100000000);
		x %= 100000000;
		std::memcpy(out, quads[x / 10000].data(), 4);
		std::memcpy(out + 4, quads[x % 10000].data(), 4);
		return out + 8;
	}
	if (x >= 10000) {
		out = small(out, x / 10000);
		std::memcpy(out, quads[x % 10000].data(), 4);
		return out + 4;
	}
	return small(out, x);
}
template<std::size_t Fields, class Read>
[[gnu::noinline]]
void records(FastOutput &output, std::size_t count, Read read) {
	char text[65536];
	constexpr auto capacity = (sizeof(text) - 4) / (11 * Fields);
	for (std::size_t base = 0; base < count;) {
		auto end = std::min(count, base + capacity);
		char *p = text;
		for (; base < end; ++base) {
			auto values = read(base);
			for (std::size_t j = 0; j < Fields; ++j) {
				p = number(p, values[j]);
				*p++ = j + 1 == Fields ? '\n' : ' ';
			}
		}
		output.write(std::string_view(text, p - text));
	}
}
}
