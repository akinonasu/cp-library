#include "convolution/convolution_light.hpp"
#include "convolution/cyclic_convolution_light.hpp"
#include "convolution/multiplicative_convolution_light.hpp"
#include "convolution/ordered_convolution_light.hpp"
#include "convolution/max_min_convolution_light.hpp"
#include "convolution/multivariate_convolution_light.hpp"
#include <cassert>
#include <random>

struct OtherMint {
	unsigned value = 0;
	OtherMint(unsigned x = 0) : value(x % mod()) {}
	static constexpr unsigned mod() { return 1000000007; }
	static OtherMint raw(unsigned x) { return OtherMint(x); }
	unsigned val() const { return value; }
};
struct Ring {
	long long value = 0;
	Ring(long long x = 0) : value(x) {}
	Ring &operator+=(Ring x) { value += x.value; return *this; }
	Ring &operator-=(Ring x) { value -= x.value; return *this; }
	friend Ring operator*(Ring a, Ring b) { return a.value * b.value; }
	friend bool operator==(Ring, Ring) = default;
};
template<class T>
std::vector<T> naive(const std::vector<T> &a, const std::vector<T> &b) {
	if (a.empty() || b.empty()) return {};
	std::vector<T> result(a.size() + b.size() - 1);
	for (std::size_t i = 0; i < a.size(); ++i)
		for (std::size_t j = 0; j < b.size(); ++j) result[i + j] += a[i] * b[j];
	return result;
}

int main() {
	std::mt19937 rng(290931);
	for (int n : {0, 1, 2, 31, 32, 33, 65, 129, 257}) {
		for (int m : {0, 1, 2, 31, 32, 33, 65, 129}) {
			std::vector<mint> a(n), b(m);
			for (auto *v : {&a, &b}) for (auto &x : *v) x = rng();
			assert(light_convolution::convolution(a, b) == naive(a, b));
			assert(light_convolution::convolution(a, a) == naive(a, a));
			std::vector<mint> ordered(a.empty() || b.empty() ? 0 : n + m - 1);
			for (int i = 0; i < n; ++i) for (int j = i + 1; j < m; ++j) ordered[i + j] += a[i] * b[j];
			assert(light_convolution::ordered_convolution(a, b) == ordered);
			std::vector<long long> x(n), y(m);
			for (auto *v : {&x, &y}) for (auto &c : *v) c = int(rng() % 201) - 100;
			assert(light_convolution::convolution(x, y) == naive(x, y));
			std::vector<Ring> ra(x.begin(), x.end()), rb(y.begin(), y.end());
			assert(light_convolution::convolution(ra, rb) == naive(ra, rb));
			std::vector<double> fa(x.begin(), x.end()), fb(y.begin(), y.end());
			auto product = light_convolution::convolution(fa, fb), expected = naive(fa, fb);
			for (std::size_t i = 0; i < product.size(); ++i) assert(std::abs(product[i] - expected[i]) < 1e-6);
			std::vector<std::complex<double>> ca(fa.begin(), fa.end()), cb(fb.begin(), fb.end());
			auto complex_product = light_convolution::convolution(ca, cb), complex_expected = naive(ca, cb);
			for (std::size_t i = 0; i < complex_product.size(); ++i) assert(std::abs(complex_product[i] - complex_expected[i]) < 1e-6);
			std::vector<OtherMint> ma(n), mb(m);
			for (auto *v : {&ma, &mb}) for (auto &c : *v) c = rng();
			auto mod_product = light_convolution::convolution(ma, mb);
			std::vector<unsigned> mod_expected(a.empty() || b.empty() ? 0 : n + m - 1);
			for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j)
				mod_expected[i + j] = (mod_expected[i + j] + std::uint64_t(ma[i].val()) * mb[j].val()) % OtherMint::mod();
			for (std::size_t i = 0; i < mod_product.size(); ++i) assert(mod_product[i].val() == mod_expected[i]);
			for (bool minimum : {false, true}) {
				std::vector<long long> extrema(x.empty() || y.empty() ? 0 : n + m - 1, minimum ? 1000000 : -1000000);
				for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
					auto value = minimum ? std::max(x[i], y[j]) : std::min(x[i], y[j]);
					extrema[i + j] = minimum ? std::min(extrema[i + j], value) : std::max(extrema[i + j], value);
				}
				auto actual = minimum ? light_convolution::min_max_convolution(x, y) : light_convolution::max_min_convolution(x, y);
				assert(actual == extrema);
			}
			for (int size : {1, 2, 17}) {
				std::vector<mint> positive(size), negative(size);
				for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
					positive[(i + j) % size] += a[i] * b[j];
					negative[(i + j) % size] += (i + j) / size % 2 ? -a[i] * b[j] : a[i] * b[j];
				}
				assert(light_convolution::cyclic_convolution(a, b, size) == positive);
				assert(light_convolution::cyclic_convolution<true>(a, b, size) == negative);
			}
		}
	}
	for (int p : {2, 3, 17, 97}) {
		std::vector<mint> a(p), b(p), expected(p);
		for (auto *v : {&a, &b}) for (auto &x : *v) x = rng();
		for (int i = 0; i < p; ++i) for (int j = 0; j < p; ++j) expected[i * j % p] += a[i] * b[j];
		assert(light_convolution::multiplicative_convolution(a, b) == expected);
	}
	for (const std::vector<int> &dims : {std::vector<int>{}, {1}, {2, 3, 2}, {3, 1, 7}, {3, 4, 5}}) {
		int n = 1;
		for (int d : dims) n *= d;
		std::vector<mint> a(n), b(n), expected(n);
		for (auto *v : {&a, &b}) for (auto &x : *v) x = rng();
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			bool valid = true;
			int x = i, y = j;
			for (int d : dims) { valid &= x % d + y % d < d; x /= d; y /= d; }
			if (valid) expected[i + j] += a[i] * b[j];
		}
		assert(light_convolution::multivariate_convolution(a, b, dims) == expected);
		std::vector<Ring> ra(n, Ring(1)), rb(n, Ring(2)), re(n);
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			bool valid = true;
			int x = i, y = j;
			for (int d : dims) { valid &= x % d + y % d < d; x /= d; y /= d; }
			if (valid) re[i + j] += Ring(2);
		}
		assert(light_convolution::multivariate_convolution(ra, rb, dims) == re);
	}
}
