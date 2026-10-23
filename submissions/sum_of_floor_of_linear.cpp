#line 1 "test/monoid_floor_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/sum_of_floor_of_linear"
#line 1 "math/floor_sum/monoid_floor_sum.hpp"



#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

template <class S, auto op, auto e>
S monoid_floor_sum(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b, S x, S y) {
	assert(m > 0);
	auto append = [](S value, const S &base, std::uint64_t k) {
		if (!k) return value;
		S p = base;
		for (;;) {
			if (k & 1) value = op(std::move(value), p);
			k >>= 1;
			if (!k) return value;
			p = op(p, p);
		}
	};
	S prefix = e();
	if (b >= m) {
		prefix = append(std::move(prefix), y, b / m);
		b %= m;
	}
	if (!n) return prefix;
	S suffix = e();
	for (;;) {
		if (a >= m) {
			x = append(std::move(x), y, a / m);
			a %= m;
		}
		__uint128_t z = __uint128_t(a) * n + b;
		if (z < m) return op(append(std::move(prefix), x, n), std::move(suffix));
		std::uint64_t k, r;
		if (z >> 64) k = z / m, r = z % m;
		else {
			std::uint64_t v = z;
			k = v / m, r = v % m;
		}
		suffix = op(append(y, x, r / a), std::move(suffix));
		n = k - 1;
		std::uint64_t t = m - b - 1;
		prefix = append(std::move(prefix), x, t / a + 1);
		b = t % a;
		if (!n) return op(std::move(prefix), std::move(suffix));
		std::swap(a, m);
		std::ranges::swap(x, y);
	}
}


#line 3 "test/monoid_floor_sum.test.cpp"
#include <algorithm>
#include <array>
#line 6 "test/monoid_floor_sum.test.cpp"
#include <iostream>
#include <string>
#include <tuple>

using namespace std;

using S = array<__uint128_t, 3>;
S op(const S &a, const S &b) { return {a[0] + b[0], a[1] + b[1], a[2] + b[2] + a[1] * b[0]}; }
S e() { return {}; }

void print_integer(__int128_t value) {
	__uint128_t x = value;
	if (value < 0) cout << '-', x = -x;
	string digits;
	do {
		digits.push_back('0' + x % 10);
		x /= 10;
	} while (x);
	reverse(digits.begin(), digits.end());
	cout << digits << '\n';
}

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int q;
	cin >> q;
	for (int repeat_index = 0; repeat_index < q; ++repeat_index) {
		uint64_t n, m, a, b;
		cin >> n >> m >> a >> b;
		auto result = monoid_floor_sum<S, op, e>(n, m, a, b, {1, 0, 0}, {0, 1, 0});
		print_integer(__int128_t(result[2]));
	}
}
