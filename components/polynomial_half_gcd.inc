#ifndef CP_COMPONENTS_POLYNOMIAL_HALF_GCD_HPP
#define CP_COMPONENTS_POLYNOMIAL_HALF_GCD_HPP

#include "../fps/fps.hpp"
#include <array>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct PolynomialHalfGCD {
	using T = Mint;
	using F = FormalPowerSeries<T>;
	using O = Operations<T>;
	static constexpr int direct_limit = 64;
	struct Matrix {
		std::array<F, 4> a{F{1}, F{}, F{}, F{1}};
		mutable std::array<F, 4> cache;
		int shift = 0;
		bool identity = true;
	};
	static F high(const F& a, int k) {
		return k >= int(a.size()) ? F{} : F(a.begin() + k, a.end());
	}
	static F fold(const F& a, int n) {
		if (int(a.size()) <= n) { F result = a; result.resize(n); return result; }
		F result(n);
		for (int i = 0; i < int(a.size()); ++i) result[i & (n - 1)] += a[i];
		return result;
	}
	static const F& transformed(const Matrix& matrix, int index, int n) {
		F& cache = matrix.cache[index];
		if (cache.empty()) {
			cache = fold(matrix.a[index], n);
			O::ntt(cache);
		}
		while (int(cache.size()) < n) {
			const int half = cache.size();
			F odd(half);
			const F& p = matrix.a[index];
			for (int i = 0; i < int(p.size()); ++i)
				odd[i & (half - 1)] += (i & half) ? -p[i] : p[i];
			O::geometric_scale(odd, T(1), O::ntt_roots(2 * half)[half]);
			O::ntt(odd);
			cache.insert(cache.end(), odd.begin(), odd.end());
		}
		return cache;
	}
	static F product_sum(const F& a, const F& b, const F& c, const F& d, int n, F* cache = nullptr) {
		F result(a.begin(), a.begin() + n);
		NTT::combine(O::words(result), O::words(c), O::words(d), O::words(b), n);
		if (cache) *cache = result;
		O::intt(result), O::trim(result);
		return result;
	}
	static Matrix multiply_column(const Matrix& a, const Matrix& b) {
		if (a.identity) return b;
		if (b.identity) return a;
		Matrix result;
		result.shift = a.shift + b.shift, result.identity = false;
		if (result.shift <= 64) {
			for (int i = 0; i < 2; ++i) {
				result.a[2 * i] = a.a[2 * i] * b.a[0] + a.a[2 * i + 1] * b.a[2];
				O::trim(result.a[2 * i]);
			}
			return result;
		}
		int n = 1;
		while (n <= result.shift) n <<= 1;
		std::array<const F*, 4> x{}, y{};
		for (int i = 0; i < 4; ++i) {
			x[i] = &transformed(a, i, n);
			if (!(i & 1)) y[i] = &transformed(b, i, n);
		}
		for (int i = 0; i < 2; ++i)
			result.a[2 * i] = product_sum(*x[2 * i], *y[0], *x[2 * i + 1], *y[2], n, &result.cache[2 * i]);
		return result;
	}
	static void step_column(Matrix& matrix, const F& q) {
		for (auto& cache : matrix.cache) cache.clear();
		F next = matrix.a[0] - q * matrix.a[2];
		O::trim(next);
		matrix.a[0] = std::move(matrix.a[2]), matrix.a[2] = std::move(next);
		matrix.shift += std::max(0, int(q.size()) - 1), matrix.identity = false;
	}
	static Matrix multiply(const Matrix& a, const Matrix& b, bool first_only = false) {
		if (a.identity) return b;
		if (b.identity) return a;
		Matrix result;
		result.shift = a.shift + b.shift, result.identity = false;
		if (first_only) result.a[3].clear();
		const int rows = first_only ? 1 : 2;
		if (result.shift <= 64) {
			for (int i = 0; i < rows; ++i) for (int j = 0; j < 2; ++j) {
				result.a[2 * i + j] = a.a[2 * i] * b.a[j] + a.a[2 * i + 1] * b.a[2 + j];
				O::trim(result.a[2 * i + j]);
			}
			return result;
		}
		int n = 1;
		while (n <= result.shift) n <<= 1;
		std::array<const F*, 4> x{}, y{};
		for (int i = 0; i < 4; ++i) {
			if (i < 2 * rows) x[i] = &transformed(a, i, n);
			y[i] = &transformed(b, i, n);
		}
		for (int i = 0; i < rows; ++i) for (int j = 0; j < 2; ++j)
			result.a[2 * i + j] = product_sum(*x[2 * i], *y[j], *x[2 * i + 1], *y[2 + j], n, &result.cache[2 * i + j]);
		return result;
	}
	static void step(Matrix& matrix, const F& q) {
		for (auto& cache : matrix.cache) cache.clear();
		F c = matrix.a[0] - q * matrix.a[2], d = matrix.a[1] - q * matrix.a[3];
		O::trim(c), O::trim(d);
		matrix.a[0] = std::move(matrix.a[2]), matrix.a[1] = std::move(matrix.a[3]);
		matrix.a[2] = std::move(c), matrix.a[3] = std::move(d);
		matrix.shift += std::max(0, int(q.size()) - 1), matrix.identity = false;
	}
	static void apply(const Matrix& matrix, F& a, F& b) {
		if (matrix.identity) return;
		if (a.size() <= 128) {
			F c = matrix.a[0] * a + matrix.a[1] * b;
			b = matrix.a[2] * a + matrix.a[3] * b, a = std::move(c);
			O::trim(a), O::trim(b);
			return;
		}
		int size = int(a.size()) - matrix.shift, n = 1;
		assert(size > 0);
		while (n < size) n <<= 1;
		F x = fold(a, n), y = fold(b, n);
		O::ntt(x), O::ntt(y);
		std::array<const F*, 4> m;
		for (int i = 0; i < 4; ++i) m[i] = &transformed(matrix, i, n);
		a = product_sum(*m[0], x, *m[1], y, n);
		b = product_sum(*m[2], x, *m[3], y, n);
		assert(int(a.size()) == size && b.size() < a.size());
	}
	static F apply_first(const Matrix& matrix, const F& a, const F& b) {
		if (matrix.identity) return a;
		const int size = int(a.size()) - matrix.shift;
		if (size == 1) {
			T value = (matrix.a[0].empty() || a.empty() ? T(0) : matrix.a[0][0] * a[0]) +
				(matrix.a[1].empty() || b.empty() ? T(0) : matrix.a[1][0] * b[0]);
			return value == T(0) ? F{} : F{value};
		}
		if (a.size() <= 128) {
			F result = matrix.a[0] * a + matrix.a[1] * b;
			O::trim(result);
			return result;
		}
		int n = 1;
		while (n < size) n <<= 1;
		F x = fold(a, n), y = fold(b, n);
		O::ntt(x), O::ntt(y);
		return product_sum(transformed(matrix, 0, n), x, transformed(matrix, 1, n), y, n);
	}
	static void combine(T* out, const T* a, const T* b, int n, T x, T y, T z) {
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && NTT::vectorized()) {
			combine8(out, a, b, n, x.val(), y.val(), z.val());
			return;
		}
#endif
		for (int i = 0; i < n; ++i) out[i] = x * a[i] + y * b[i - 1] + z * b[i];
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void combine8(T* out, const T* a, const T* b, int n, unsigned x, unsigned y, unsigned z) {
		using I = NTT::I8;
		I vx = _mm256_set1_epi32(x), qx = _mm256_set1_epi32(NTT::quotient(x));
		I vy = _mm256_set1_epi32(y), qy = _mm256_set1_epi32(NTT::quotient(y));
		I vz = _mm256_set1_epi32(z), qz = _mm256_set1_epi32(NTT::quotient(z));
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I u = NTT::fixed_mul8(NTT::load8(reinterpret_cast<const unsigned*>(a + i)), vx, qx);
			I v = NTT::fixed_mul8(NTT::load8(reinterpret_cast<const unsigned*>(b + i - 1)), vy, qy);
			I w = NTT::fixed_mul8(NTT::load8(reinterpret_cast<const unsigned*>(b + i)), vz, qz);
			NTT::store8(reinterpret_cast<unsigned*>(out + i), NTT::add8(u, NTT::add8(v, w)));
		}
		T tx = T::raw(x), ty = T::raw(y), tz = T::raw(z);
		for (; i < n; ++i) out[i] = tx * a[i] + ty * b[i - 1] + tz * b[i];
	}
#endif
	static Matrix direct(const F& a, const F& b, int target = -1) {
		constexpr int capacity = 2 * direct_limit + 8;
		assert(a.size() <= 2 * direct_limit + 1);
		alignas(32) T polynomials[3][capacity] = {}, rows[3][2][capacity] = {};
		T *c = polynomials[0] + 1, *d = polynomials[1] + 1, *e = polynomials[2] + 1;
		T *row[3][2];
		int sizes[3][2] = {{1, 0}, {0, 1}, {0, 0}};
		for (int i = 0; i < 3; ++i) for (int j = 0; j < 2; ++j) row[i][j] = rows[i][j] + 1;
		std::copy(a.begin(), a.end(), c);
		std::copy(b.begin(), b.end(), d);
		row[0][0][0] = row[1][1][0] = 1;
		const int n = a.size();
		if (target < 0) target = n / 2;
		int cd = n - 1, dd = int(b.size()) - 1;
		int floor = 2 * target - (n - 1);
		T scale_c = 1, scale_d = 1;
		while (dd >= target) {
			const int delta = cd - dd;
			floor += delta;
			const int low = std::max(0, floor);
			const T alpha = d[dd];
			T scale_e = scale_c;
			if (delta == 1) {
				const T x = alpha * alpha, y = -alpha * c[cd];
				const T z = c[cd] * d[dd - 1] - alpha * c[cd - 1];
				combine(e + low, c + low, d + low, dd - low, x, y, z);
				scale_e *= x;
				for (int j = 0; j < 2; ++j) {
					const int length = std::max(sizes[0][j], sizes[1][j] ? sizes[1][j] + 1 : 0);
					combine(row[2][j], row[0][j], row[1][j], length, x, y, z);
					std::fill(row[2][j] + length, row[2][j] + std::max(length, sizes[2][j]), T(0));
					sizes[2][j] = length;
				}
			} else {
				std::copy(c + low, c + cd + 1, e + low);
				for (int j = 0; j < 2; ++j) {
					std::copy_n(row[0][j], sizes[0][j], row[2][j]);
					std::fill(row[2][j] + sizes[0][j], row[2][j] + std::max(sizes[0][j], sizes[2][j]), T(0));
					sizes[2][j] = sizes[0][j];
				}
				for (int shift = delta; shift >= 0; --shift) {
					const T factor = e[dd + shift];
					for (int i = low; i <= dd + shift; ++i) e[i] = alpha * e[i] - (i >= shift ? factor * d[i - shift] : T(0));
					for (int j = 0; j < 2; ++j) {
						const int length = std::max(sizes[2][j], sizes[1][j] ? sizes[1][j] + shift : 0);
						for (int i = 0; i < length; ++i)
							row[2][j][i] = alpha * row[2][j][i] - (i >= shift ? factor * row[1][j][i - shift] : T(0));
						sizes[2][j] = length;
					}
					scale_e *= alpha;
				}
			}
			std::swap(c, d), std::swap(d, e);
			for (int j = 0; j < 2; ++j) {
				std::swap(row[0][j], row[1][j]), std::swap(row[1][j], row[2][j]);
				std::swap(sizes[0][j], sizes[1][j]), std::swap(sizes[1][j], sizes[2][j]);
			}
			scale_c = scale_d, scale_d = scale_e;
			cd = dd, dd = cd - 1;
			while (dd >= target && d[dd] == T(0)) --dd;
		}
		Matrix result;
		result.shift = n - 1 - cd, result.identity = result.shift == 0;
		T inverse = (scale_c * scale_d).inv();
		T factors[2] = {inverse * scale_d, inverse * scale_c};
		for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) {
			result.a[2 * i + j] = F(row[i][j], row[i][j] + sizes[i][j]);
			O::scale_words(result.a[2 * i + j], factors[i]);
			O::trim(result.a[2 * i + j]);
		}
		return result;
	}
	static Matrix jump(F a, F b, int k, bool first_only = false) {
		const int n = int(a.size()) - 1, target = n - k;
		assert(a.size() > b.size() && 0 <= k && k <= n);
		if (int(b.size()) <= target) return Matrix{};
		if (n > 2 * k) return jump(high(a, n - 2 * k), high(b, n - 2 * k), k, first_only);
		if (k <= direct_limit) return direct(a, b, target);
		int first = 1;
		while (2 * first < k) first *= 2;
		const int cut = std::max(0, n - 2 * first);
		Matrix left = jump(high(a, cut), high(b, cut), first);
		if (left.identity) {
			auto [q, r] = a.divmod(b);
			step(left, q), a = std::move(b), b = std::move(r);
		} else apply(left, a, b);
		if (int(b.size()) <= target) return left;
		const int second = int(a.size()) - 1 - target;
		const int skip = std::max(0, int(a.size()) - 1 - 2 * second);
		return multiply(jump(high(a, skip), high(b, skip), second, first_only), left, first_only);
	}
	static Matrix half(F a, F b) {
		assert(a.size() > b.size());
		int m = int(a.size()) / 2;
		Matrix result;
		if (int(b.size()) <= m) return result;
		if (a.size() <= 2 * direct_limit) return direct(a, b);
		result = half(high(a, m), high(b, m));
		apply(result, a, b);
		if (int(b.size()) <= m) return result;
		auto [q, r] = a.divmod(b);
		step(result, q), a = std::move(b), b = std::move(r);
		if (int(b.size()) <= m) return result;
		int k = 2 * m + 1 - int(a.size());
		return multiply(half(high(a, k), high(b, k)), result);
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
