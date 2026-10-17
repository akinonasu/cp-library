#include "components/fps_factorials.hpp"
#include "fps/taylor_shift.hpp"
#include "fps/polynomial_gcd.hpp"
#include <cstdlib>
#include <random>

using H = fps_detail::PolynomialHalfGCD;
using O = fps_detail::Operations<mint>;

void require(bool condition) {
	if (!condition) std::abort();
}

void check_cache(const H::Matrix& matrix) {
	for (int i = 0; i < 4; ++i) {
		const auto& cache = matrix.cache[i];
		if (cache.empty()) continue;
		auto expected = H::fold(matrix.a[i], cache.size());
		O::ntt(expected);
		require(expected == cache);
	}
}

int main() {
	std::mt19937 random(294384);
	for (int n : {0, 1, 2, 7, 8, 31, 32, 63, 64, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 65535, 65536, 65537}) {
		fps a(n), b(n);
		fps_detail::factorials(O::words(a), O::words(b), n);
		unsigned long long expected = 1;
		for (int i = 0; i < n; ++i) {
			if (i) expected = expected * i % mint::mod();
			require(a[i].val() == expected);
			require(a[i] * b[i] == mint(1));
		}
	}
	for (int n : {0, 1, 2, 7, 31, 32, 63, 64, 127, 128, 129, 255, 256, 257, 511, 513}) {
		for (mint c : {mint(0), mint(1), mint(-1), mint(random())}) {
			fps a(n);
			for (auto& x : a) x = random();
			fps expected = a;
			for (int i = n - 1; i > 0; --i)
				for (int j = i - 1; j + 1 < n; ++j) expected[j] += c * expected[j + 1];
			require(taylor_shift(a, c) == expected);
		}
	}
	for (int n : {129, 130, 255, 256, 257, 511, 512, 513, 1025}) {
		for (int trial = 0; trial < 8; ++trial) {
			fps a(n), b(n - 1);
			for (auto* f : {&a, &b}) {
				for (auto& x : *f) x = trial % 2 ? random() : random() % 2;
				f->back() = 1;
			}
			auto matrix = H::half(a, b);
			check_cache(matrix);
			for (int length : {64, 128, 256, 1024}) {
				for (int i = 0; i < 4; ++i) {
					const auto& transformed = H::transformed(matrix, i, length);
					auto expected = H::fold(matrix.a[i], length);
					O::ntt(expected);
					require(std::equal(expected.begin(), expected.end(), transformed.begin()));
				}
			}
			for (fps q : {fps{}, fps{3}, fps{1, 2}, fps{1, 2, 3, 4, 5}}) {
				auto copy = matrix;
				H::step(copy, q);
				check_cache(copy);
				auto column = matrix;
				H::step_column(column, q);
				check_cache(column);
				require(copy.a[0] == column.a[0] && copy.a[2] == column.a[2]);
			}
			auto square = H::multiply(matrix, matrix);
			check_cache(square);
			for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) {
				fps expected = matrix.a[2 * i] * matrix.a[j] + matrix.a[2 * i + 1] * matrix.a[2 + j];
				O::trim(expected);
				require(expected == square.a[2 * i + j]);
			}
			for (int progress : {0, 1, 63, 64, 65, n / 2, n - 1}) {
				if (progress >= n) continue;
				H::Matrix expected;
				fps c = a, d = b;
				while (int(d.size()) > n - 1 - progress) {
					auto [q, r] = c.divmod(d);
					H::step(expected, q), c = std::move(d), d = std::move(r);
				}
				auto actual = H::jump(a, b, progress);
				require(actual.a == expected.a && actual.shift == expected.shift);
				auto row = H::jump(a, b, progress, true);
				require(row.a[0] == expected.a[0] && row.a[1] == expected.a[1] && row.shift == expected.shift);
				require(H::apply_first(row, a, b) == c);
				check_cache(actual), check_cache(row);
			}
		}
	}
}
