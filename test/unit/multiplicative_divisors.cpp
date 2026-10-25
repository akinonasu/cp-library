#include "math/multiplicative_divisors.hpp"
#include "math/mint.hpp"
#include <numeric>
#include <random>

using U = std::uint64_t;

int mobius(U n) {
	int value = 1;
	for (U p = 2; p <= n / p; ++p) if (n % p == 0) {
		n /= p;
		value = -value;
		if (n % p == 0) return 0;
	}
	return n > 1 ? -value : value;
}

struct Value {
	U value;
	explicit Value(U x) : value(x) {}
	friend Value operator*(const Value &a, const Value &b) { return Value(a.value * b.value); }
};

int main() {
	for (U n = 1; n <= 2000; ++n) {
		auto mu = multiplicative_divisors(n, [](U, int e, U) { return e == 1 ? -1 : 0; });
		auto phi = multiplicative_divisors(n, [](U p, int, U pe) { return pe - pe / p; });
		auto tau = multiplicative_divisors(n, [](U, int e, U) { return e + 1; });
		auto sigma = multiplicative_divisors(n, [](U p, int e, U) {
			U sum = 1, power = 1;
			while (e--) sum += power *= p;
			return sum;
		});
		std::vector<U> ds;
		for (U d = 1; d <= n; ++d) if (n % d == 0) ds.push_back(d);
		assert(mu.size() == ds.size() && phi.size() == ds.size());
		assert(tau.size() == ds.size() && sigma.size() == ds.size());
		int mu_sum = 0;
		for (std::size_t i = 0; i < ds.size(); ++i) {
			U d = ds[i], ph = 0, sig = 0;
			int count = 0;
			for (U x = 1; x <= d; ++x) {
				ph += std::gcd(x, d) == 1;
				if (d % x == 0) ++count, sig += x;
			}
			assert(mu[i].first == d && mu[i].second == mobius(d));
			assert(phi[i].first == d && phi[i].second == ph);
			assert(tau[i].first == d && tau[i].second == count);
			assert(sigma[i].first == d && sigma[i].second == sig);
			mu_sum += mu[i].second;
		}
		assert(mu_sum == int(n == 1));
	}
	int calls = 0;
	auto callback = [&](U p, int e, U pe) {
		++calls;
		U power = 1;
		for (int i = 0; i < e; ++i) power *= p;
		assert(power == pe && e > 0);
		return Value(pe);
	};
	for (auto factors : {std::vector<std::pair<U, int>>{}, { {5, 1}, {2, 3}, {3, 2} }, { {2, 63} }, { {18446744073709551557ULL, 1} }, { {3, 1}, {5, 1}, {17, 1}, {257, 1}, {641, 1}, {65537, 1}, {6700417, 1} }}) {
		int expected_calls = 0;
		for (auto [p, e] : factors) expected_calls += e;
		calls = 0;
		auto values = multiplicative_divisors(factors, std::ref(callback));
		assert(calls == expected_calls && values.size() == divisors(factors).size());
		for (std::size_t i = 0; i < values.size(); ++i) {
			assert(values[i].first == values[i].second.value);
			assert(!i || values[i - 1].first < values[i].first);
		}
		std::reverse(factors.begin(), factors.end());
		auto unordered = multiplicative_divisors(factors, callback, false);
		std::sort(unordered.begin(), unordered.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
		assert(values.size() == unordered.size());
		for (std::size_t i = 0; i < values.size(); ++i) assert(values[i].first == unordered[i].first && values[i].second.value == unordered[i].second.value);
	}
	std::mt19937_64 rng(834624);
	for (int trial = 0; trial < 100; ++trial) {
		U n = rng() % 100000000 + 1;
		auto zero = multiplicative_divisors(n, [](U, int, U) { return 0; }, false);
		assert(zero.size() == divisors(n).size());
		for (auto [d, value] : zero) assert(value == int(d == 1));
	}
	const U unit = 1;
	auto reference = multiplicative_divisors(12, [&](U, int, U) -> const U & { return unit; });
	static_assert(std::is_same_v<decltype(reference), std::vector<std::pair<U, U>>>);
	for (auto [d, value] : reference) assert(value == 1 && 12 % d == 0);
	for (U n : {897612484786617600ULL, 18446744073709551615ULL}) {
		auto factors = factorize_count(n);
		auto mu = multiplicative_divisors(factors, [](U, int e, U) { return e == 1 ? -1 : 0; });
		auto phi = multiplicative_divisors(factors, [](U p, int, U pe) { return pe - pe / p; });
		auto modular = multiplicative_divisors(factors, [](U, int e, U) -> mint { return e == 1 ? -1 : 0; });
		U phi_sum = 0;
		assert(mu.size() == modular.size() && mu.size() == phi.size());
		for (std::size_t i = 0; i < mu.size(); ++i) {
			auto [d, value] = mu[i];
			U remaining = d;
			int expected = 1;
			for (auto [p, exponent] : factors) {
				int e = 0;
				while (remaining % p == 0) remaining /= p, ++e;
				assert(e <= exponent);
				if (e) expected *= e == 1 ? -1 : 0;
			}
			assert(remaining == 1 && value == expected && n % d == 0);
			assert(modular[i].first == d && modular[i].second == mint(expected));
			assert(phi[i].first == d && phi[i].second <= n - phi_sum);
			phi_sum += phi[i].second;
		}
		assert(phi_sum == n);
	}
}
