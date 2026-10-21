#include "components/fps_light_ntt.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

using U = std::uint64_t;
constexpr U p = mint::mod();

std::vector<unsigned> canonical(const std::vector<mint> &a) {
	std::vector<unsigned> result(a.size());
	for (std::size_t i = 0; i < a.size(); ++i) result[i] = a[i].val();
	return result;
}

void pointwise_and_scale() {
	std::mt19937_64 rng(916623);
	const std::array<unsigned, 8> boundaries{0, 1, 2, unsigned(p - 1), unsigned(p), unsigned(p + 1), unsigned(2 * p - 2), unsigned(2 * p - 1)};
	for (int n : {0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 65, 127, 129, 1023, 1025}) {
		for (int pattern = 0; pattern < 3; ++pattern) {
			std::vector<mint> a(n), b(n);
			std::vector<unsigned> expected(n), square(n);
			for (int i = 0; i < n; ++i) {
				a[i] = mint::word(pattern == 0 ? unsigned(rng() % (2 * p)) : pattern == 1 ? boundaries[i % boundaries.size()] : unsigned(p));
				b[i] = mint::word(pattern == 0 ? unsigned(rng() % (2 * p)) : boundaries[(i + 3) % boundaries.size()]);
				expected[i] = U(a[i].val()) * b[i].val() % p;
				square[i] = U(a[i].val()) * a[i].val() % p;
			}
			for (bool vectorized : {false, fps_light_detail::simd_available()}) {
				auto product = a;
				fps_light_detail::pointwise(product, b, vectorized);
				assert(canonical(product) == expected);
				product = a;
				fps_light_detail::pointwise(product, product, vectorized);
				assert(canonical(product) == square);
				for (unsigned value : boundaries) {
					mint factor = mint::word(value);
					std::vector<unsigned> scaled(n);
					for (int i = 0; i < n; ++i) scaled[i] = U(a[i].val()) * factor.val() % p;
					auto result = a;
					fps_light_detail::scale(result, factor, vectorized);
					assert(canonical(result) == scaled);
				}
			}
		}
	}
}

void unaligned_helpers() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	if (!fps_light_detail::simd_available()) return;
	std::mt19937_64 rng(327159);
	for (int n : {1, 7, 8, 9, 31, 32, 33, 129}) {
		std::vector<mint> a(n + 2), b(n + 2);
		for (auto &x : a) x = mint::word(unsigned(rng() % (2 * p)));
		for (auto &x : b) x = mint::word(unsigned(rng() % (2 * p)));
		auto result = a;
		fps_light_detail::pointwise_avx2(result.data() + 1, b.data() + 1, n);
		assert(result.front() == a.front() && result.back() == a.back());
		for (int i = 1; i <= n; ++i) assert(result[i].val() == U(a[i].val()) * b[i].val() % p);
		mint factor = mint::word(unsigned(2 * p - 1));
		result = a;
		fps_light_detail::scale_avx2(result.data() + 1, n, factor);
		assert(result.front() == a.front() && result.back() == a.back());
		for (int i = 1; i <= n; ++i) assert(result[i].val() == U(a[i].val()) * factor.val() % p);
	}
#endif
}

U modular_power(U a, U k) {
	U result = 1;
	for (; k; k >>= 1, a = a * a % p) if (k & 1) result = result * a % p;
	return result;
}

void transform_paths() {
	std::mt19937_64 rng(963217);
	const std::array<unsigned, 8> boundaries{0, 1, 2, unsigned(p - 1), unsigned(p), unsigned(p + 1), unsigned(2 * p - 2), unsigned(2 * p - 1)};
	fps_light_detail::Roots prepared;
	for (std::size_t n = 1; n <= 65536; n *= 2) {
		std::vector<mint> positive(std::max<std::size_t>(2, n)), negative(positive.size());
		positive[1] = negative[1] = 1;
		for (std::size_t h = 1; h < n; h *= 2) {
			U step = modular_power(3, (p - 1) / (2 * h)), back = modular_power(step, p - 2), x = 1, y = 1;
			for (std::size_t j = 0; j < h; ++j) {
				positive[h + j] = mint::raw(unsigned(x));
				negative[h + j] = mint::raw(unsigned(y));
				x = x * step % p; y = y * back % p;
			}
		}
		prepared.prepare(n);
		for (std::size_t i = 0; i < n; ++i) {
			assert(prepared.forward[i] == positive[i]);
			assert(prepared.inverse[i] == negative[i]);
		}
		for (int pattern = 0; pattern < 2; ++pattern) {
			std::vector<mint> a(n);
			for (std::size_t i = 0; i < n; ++i) a[i] = mint::word(pattern ? boundaries[i % boundaries.size()] : unsigned(rng() % (2 * p)));
			auto original = canonical(a);
			auto scalar = a, native = a;
			for (std::size_t h = n / 2; h; h /= 2) fps_light_detail::scalar_stage<false>(scalar.data(), n, h, positive.data() + h);
			fps_light_detail::transform(native);
			assert(canonical(native) == canonical(scalar));
			auto scalar_inverse = native;
			for (std::size_t h = 1; h < n; h *= 2) fps_light_detail::scalar_stage<true>(scalar_inverse.data(), n, h, negative.data() + h);
			fps_light_detail::scale(scalar_inverse, mint(n).inv(), false);
			assert(canonical(scalar_inverse) == original);
			fps_light_detail::transform(scalar, true);
			assert(canonical(scalar) == original);
		}
	}
}

void products() {
	std::mt19937_64 rng(147629);
	for (int n : {33, 34, 65, 129, 257, 513, 1025}) {
		for (int m : {33, 65, n}) {
			std::vector<mint> a(n), b(m);
			std::vector<unsigned> expected(n + m - 1);
			for (auto &x : a) x = mint::word(unsigned(rng() % (2 * p)));
			for (auto &x : b) x = mint::word(unsigned(rng() % (2 * p)));
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < m; ++j) expected[i + j] = (expected[i + j] + U(a[i].val()) * b[j].val()) % p;
			assert(canonical(fps_light_detail::multiply(a, b)) == expected);
		}
	}
}

int main() {
	pointwise_and_scale();
	unaligned_helpers();
	transform_paths();
	products();
}
