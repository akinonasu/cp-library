#define PROBLEM "https://judge.yosupo.jp/problem/multivariate_convolution"
#include "../convolution/multivariate_convolution.hpp"
#include <cstdio>
#include <charconv>
#include <cstring>
#include <cstdint>
#include <bit>
[[gnu::always_inline]] inline int read() {
	static char buffer[1 << 16];
	static std::size_t pos = 0, size = 0;
	if (std::endian::native == std::endian::little && size - pos >= 16) {
		std::uint64_t digits;
		std::memcpy(&digits, buffer + pos, 8);
		digits ^= 0x3030303030303030ULL;
		if (!(digits & 0xf0f0f0f0f0f0f0f0ULL)) {
			digits = (digits * 2561 >> 8) & 0x00ff00ff00ff00ffULL;
			digits = (digits * 6553601 >> 16) & 0x0000ffff0000ffffULL;
			int x = digits * 42949672960001ULL >> 32;
			for (pos += 8; buffer[pos] >= '0' && buffer[pos] <= '9'; ++pos) x = x * 10 + buffer[pos] - '0';
			++pos;
			return x;
		}
	}
	auto next = [&]() { if (pos == size) size = fread(buffer, 1, sizeof(buffer), stdin), pos = 0; return size ? buffer[pos++] : char(0); };
	char c;
	do c = next(); while (c && c <= ' ');
	int x = 0;
	while (c >= '0' && c <= '9') x = x * 10 + c - '0', c = next();
	return x;
}
int main() {
	int k = read(), n = 1;
	std::vector<int> dimensions(k);
	for (int &d : dimensions) d = read(), n *= d;
	std::vector<mint> a(n), b(n);
	for (auto &x : a) x = mint::raw(read());
	for (auto &x : b) x = mint::raw(read());
	auto result = multivariate_convolution(a, b, dimensions);
	std::vector<char> output(std::size_t(n) * 11);
	char *end = output.data();
	for (auto x : result) {
		end = std::to_chars(end, end + 10, x.val()).ptr;
		*end++ = ' ';
	}
	fwrite(output.data(), 1, end - output.data(), stdout);
}
