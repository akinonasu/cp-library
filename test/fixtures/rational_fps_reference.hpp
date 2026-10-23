#ifndef TEST_RATIONAL_FPS_REFERENCE_HPP
#define TEST_RATIONAL_FPS_REFERENCE_HPP

#include "math/mint.hpp"
#include <algorithm>
#include <string>
#include <vector>

namespace rational_fps_test {
inline std::vector<mint> prefix(const std::vector<mint> &p, const std::vector<mint> &q, int n) {
	std::vector<mint> a(n);
	const mint inverse = q[0].inv();
	for (int i = 0; i < n; ++i) {
		if (i < int(p.size())) a[i] = p[i];
		for (int j = 1; j <= i && j < int(q.size()); ++j) a[i] -= q[j] * a[i - j];
		a[i] *= inverse;
	}
	return a;
}

inline std::vector<mint> range(const std::vector<mint> &p, std::vector<mint> q, long long l, int count) {
	while (q.size() > 1 && q.back() == mint(0)) q.pop_back();
	const int d = int(q.size()) - 1, start = std::max(int(p.size()), d);
	if (l < start || !d) {
		if (!d && l >= (long long)p.size()) return std::vector<mint>(count);
		auto a = prefix(p, q, std::max<long long>(start, l + count));
		return {a.begin() + l, a.begin() + l + count};
	}
	auto a = prefix(p, q, start);
	const mint inverse = q[0].inv();
	std::vector<mint> c(d);
	for (int i = 0; i < d; ++i) c[i] = -q[i + 1] * inverse;
	auto multiply = [&](const std::vector<mint> &x, const std::vector<mint> &y) {
		std::vector<mint> z(2 * d - 1);
		for (int i = 0; i < d; ++i)
			for (int j = 0; j < d; ++j) z[i + j] += x[i] * y[j];
		for (int i = 2 * d - 2; i >= d; --i)
			for (int j = 1; j <= d; ++j) z[i - j] += z[i] * c[j - 1];
		z.resize(d);
		return z;
	};
	std::vector<mint> weight(d), power(d), result(count);
	weight[0] = 1;
	if (d == 1) power[0] = c[0];
	else power[1] = 1;
	for (long long k = l - (start - d); k; k >>= 1) {
		if (k & 1) weight = multiply(weight, power);
		power = multiply(power, power);
	}
	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < d; ++j) result[i] += weight[j] * a[start - d + j];
		mint carry = weight.back();
		for (int j = d - 1; j > 0; --j) weight[j] = weight[j - 1] + carry * c[d - 1 - j];
		weight[0] = carry * c[d - 1];
	}
	return result;
}

struct StoredMint {
	mint value;
	std::string storage = "coefficient";
	static constexpr int mod() { return mint::mod(); }
	StoredMint() = default;
	StoredMint(mint x) : value(x) {}
	static StoredMint raw(unsigned x) { return StoredMint(mint::raw(x)); }
	unsigned val() const { return value.val(); }
};
}

#endif
