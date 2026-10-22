#ifndef CP_COMPONENTS_POLYNOMIAL_HALF_GCD_LIGHT_HPP
#define CP_COMPONENTS_POLYNOMIAL_HALF_GCD_LIGHT_HPP

#include "../fps/fps_light.hpp"
#include <array>

namespace fps_light_detail {
template<class T>
struct PolynomialHalfGCD {
	using F = LightFormalPowerSeries<T>;
	struct Matrix {
		std::array<F, 4> a{F{1}, F{}, F{}, F{1}};
		bool identity = true;
	};
	static F high(const F &a, int k) {
		assert(k >= 0);
		return k >= int(a.size()) ? F{} : F(a.begin() + k, a.end());
	}
	static Matrix multiply(const Matrix &a, const Matrix &b) {
		if (a.identity) return b;
		if (b.identity) return a;
		Matrix result;
		result.identity = false;
		for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) {
			result.a[2 * i + j] = a.a[2 * i] * b.a[j] + a.a[2 * i + 1] * b.a[2 + j];
			result.a[2 * i + j].shrink();
		}
		return result;
	}
	static void step(Matrix &matrix, const F &q) {
		F c = matrix.a[0] - q * matrix.a[2], d = matrix.a[1] - q * matrix.a[3];
		c.shrink(), d.shrink();
		matrix.a[0] = std::move(matrix.a[2]), matrix.a[1] = std::move(matrix.a[3]);
		matrix.a[2] = std::move(c), matrix.a[3] = std::move(d);
		matrix.identity = false;
	}
	static void apply(const Matrix &matrix, F &a, F &b) {
		if (matrix.identity) return;
		F c = matrix.a[0] * a + matrix.a[1] * b;
		b = matrix.a[2] * a + matrix.a[3] * b, a = std::move(c);
		a.shrink(), b.shrink();
	}
	static Matrix half(F a, F b) {
		assert(a.size() > b.size());
		const int m = int(a.size()) / 2;
		Matrix result;
		if (int(b.size()) <= m) return result;
		if (a.size() <= 64) {
			while (int(b.size()) > m) {
				auto [q, r] = a.divmod(b);
				step(result, q), a = std::move(b), b = std::move(r);
			}
			return result;
		}
		result = half(high(a, m), high(b, m));
		apply(result, a, b);
		if (int(b.size()) <= m) return result;
		auto [q, r] = a.divmod(b);
		step(result, q), a = std::move(b), b = std::move(r);
		if (int(b.size()) <= m) return result;
		const int k = 2 * m + 1 - int(a.size());
		return multiply(half(high(a, k), high(b, k)), result);
	}
	static void reduce(F &a, F &b, Matrix *matrix = nullptr) {
		while (!b.empty()) {
			if (a.size() > b.size()) {
				auto part = half(a, b);
				apply(part, a, b);
				if (matrix) *matrix = multiply(part, *matrix);
				if (b.empty()) break;
			}
			auto [q, r] = a.divmod(b);
			if (matrix) step(*matrix, q);
			a = std::move(b), b = std::move(r);
		}
	}
};
}

#endif
