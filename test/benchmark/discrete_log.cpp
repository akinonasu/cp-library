#include "math/number_theory/discrete_log.hpp"
#include "components/fast_io.hpp"
#if DISCRETE_LOG_BASELINE
#include <unordered_map>

std::optional<std::uint32_t> baseline(std::uint64_t a, std::uint64_t b, std::uint32_t mod) {
	a %= mod, b %= mod;
	std::uint64_t k = 1 % mod, offset = 0;
	for (std::uint64_t g; (g = std::gcd(a, std::uint64_t(mod))) > 1;) {
		if (k == b) return offset;
		if (b % g) return std::nullopt;
		b /= g, mod /= g, k = k * (a / g) % mod, ++offset;
	}
	if (k == b) return offset;
	std::uint64_t step = std::sqrt(double(mod)) + 1, stride = 1, value = b;
	std::unordered_map<std::uint32_t, std::uint32_t> table;
	table.reserve(2 * step);
	for (std::uint32_t j = 0; j < step; ++j) {
		table[value] = j;
		value = value * a % mod, stride = stride * a % mod;
	}
	for (std::uint64_t i = 1; i <= step; ++i) {
		k = k * stride % mod;
		auto it = table.find(k);
		if (it != table.end()) return offset + i * step - it->second;
	}
	return std::nullopt;
}
#endif

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint32_t a = 0, b = 0, mod = 0;
		input.read(a), input.read(b), input.read(mod);
#if DISCRETE_LOG_BASELINE
		auto result = baseline(a, b, mod);
#else
		auto result = discrete_log(a, b, mod);
#endif
		if (result) output.println(*result);
		else output.println(-1);
	}
}
