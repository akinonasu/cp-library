#ifndef CP_FPS_LIGHT_BACKEND_HPP
#define CP_FPS_LIGHT_BACKEND_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include "../math/mint.hpp"
#include "polynomial_product_tree.hpp"
#include "polynomial_shift.hpp"

#include "fps_light_ntt.hpp"
#include "fps_sparse_scalar.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T = mint>
struct LightFormalPowerSeries : std::vector<T> {
	static_assert(T::mod() == 998244353);
	using base = std::vector<T>;
	using F = LightFormalPowerSeries;
	using Sparse = fps_detail::ScalarSparseSeries;
	using Kind = Sparse::Kind;
	using Size = std::size_t;
	using base::base;
	LightFormalPowerSeries() = default;
	LightFormalPowerSeries(base a) : base(std::move(a)) {}
	F &operator=(base a) { base::operator=(std::move(a)); return *this; }
	F &operator=(std::initializer_list<T> a) { base::operator=(a); return *this; }
	static F multiply(const F &a, const F &b) {
		if constexpr (std::is_same_v<T, mint>) return fps_light_detail::multiply(a, b);
		else {
			auto x = a.native();
			return from_native(&a == &b ? Native::multiply(x, x) : Native::multiply(x, b.native()));
		}
	}
	F operator-() const { F r = *this; for (T &x : r) x = -x; return r; }
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (Size i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (Size i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
		return *this;
	}
	F &operator+=(T x) { if (this->empty()) this->resize(1); (*this)[0] += x; return *this; }
	F &operator-=(T x) { return *this += -x; }
	F &operator*=(const F &g) { return *this = multiply(*this, g); }
	F &operator/=(const F &g) { return *this = div_series(g); }
	F &operator%=(const F &g) { return *this = divmod(g).second; }
	F &operator*=(T x) { for (T &v : *this) v *= x; return *this; }
	F &operator/=(T x) { return *this *= x.inv(); }
	friend F operator+(F a, const F &b) { return a += b; }
	friend F operator-(F a, const F &b) { return a -= b; }
	friend F operator*(const F &a, const F &b) { return multiply(a, b); }
	friend F operator/(const F &a, const F &b) { return a.div_series(b); }
	friend F operator%(const F &a, const F &b) { return a.divmod(b).second; }
	friend F operator+(F a, T b) { return a += b; }
	friend F operator-(F a, T b) { return a -= b; }
	friend F operator*(F a, T b) { return a *= b; }
	friend F operator/(F a, T b) { return a /= b; }
	friend F operator+(T a, F b) { return b += a; }
	friend F operator-(T a, const F &b) { return -b + a; }
	friend F operator*(T a, F b) { return b *= a; }
	F &operator<<=(int d) { return polynomial_detail::shift_fixed<true>(*this, d); }
	F &operator>>=(int d) { return polynomial_detail::shift_fixed<false>(*this, d); }
	friend F operator<<(F a, int d) { return a <<= d; }
	friend F operator>>(F a, int d) { return a >>= d; }

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min(Size(n), this->size()));
	}
	void shrink() { while (!this->empty() && this->back() == T(0)) this->pop_back(); }
	void trim() { shrink(); }
	T eval(T x) const {
		T result = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) result = result * x + *it;
		return result;
	}
	F diff() const {
		F result(this->size());
		for (Size i = 1; i < this->size(); ++i) result[i - 1] = (*this)[i] * T(i);
		return result;
	}
	F integral() const {
		assert(this->size() < T::mod());
		F result(this->size());
		const auto &inv = inverses(static_cast<int>(this->size()));
		for (Size i = 1; i < this->size(); ++i) result[i] = (*this)[i - 1] * inv[i];
		return result;
	}
	void onemul(int d, T c) {
		assert(d >= 0);
		if (!d) { *this *= T(1) + c; return; }
		for (Size i = this->size(); i > Size(d); --i) (*this)[i - 1] += (*this)[i - 1 - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (!d) { *this /= T(1) + c; return; }
		for (Size i = d; i < this->size(); ++i) (*this)[i] -= (*this)[i - d] * c;
	}
	F inv(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!this->empty() && (*this)[0] != T(0));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().inv(n));
		else {
			if (auto r = sparse<Kind::inverse>(0, n, 32, 12, T(0)); !r.empty()) return r;
			const int work = newton_precision(n), initial = 32;
			F result = pre(initial).inv(initial);
			const T inverse = result[0];
			for (int k = initial; k < work; k *= 2) inverse_step(*this, result, std::min(k * 2, work));
			result.resize(n);
			for (int i = work; i < n; ++i) result[i] = -dot(*this, result, i) * inverse;
			return result;
		}
	}
	F div_series(const F &g, int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!g.empty() && g[0] != T(0));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().div_series(g.native(), n));
		else {
			if (n <= 32) {
				F result(n);
				const T inverse = g[0].inv();
				for (int i = 0; i < n; ++i) {
					result[i] = (coefficient(i) - dot(g, result, i)) * inverse;
				}
				return result;
			}
			const int work = newton_precision(n), k = std::bit_floor(unsigned(work - 1)), length = k * 2;
			F inverse = g.inv(k), result = pre(k);
			const T constant = inverse[0];
			inverse.resize(length), result.resize(length);
			transform(inverse);
			inverse_product(result, inverse);
			zero(result, k);
			F quotient = result, error = g.pre(work);
			error.resize(length);
			transform(quotient);
			inverse_product(error, quotient);
			zero(error, 0, k);
			for (int i = k; i < work; ++i) error[i] = coefficient(i) - error[i];
			zero(error, work);
			inverse_product(error, inverse);
			result.resize(n);
			std::copy(error.begin() + k, error.begin() + work, result.begin() + k);
			for (int i = work; i < n; ++i) {
				result[i] = (coefficient(i) - dot(g, result, i)) * constant;
			}
			return result;
		}
	}
	F log(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!this->empty() && (*this)[0] == T(1));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().log(n));
		else {
			if (n >= 256) if (auto r = sparse<Kind::logarithm>(0, n, 12, 0, T(0)); !r.empty()) return r;
			F result = pre(n).diff().pre(n - 1).div_series(pre(n), n - 1);
			result.resize(n);
			return result.integral();
		}
	}
	F exp(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(this->empty() || (*this)[0] == T(0));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().exp(n));
		else {
			if (auto r = sparse<Kind::exponential>(0, n, 64, 24, T(1)); !r.empty()) return r;
			const int work = newton_precision(n), initial = 32;
			const auto &inv = inverses(n);
			F result = pre(initial).exp(initial);
			F reciprocal = result.inv(initial);
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				if (reciprocal.size() < Size(k)) inverse_step(result, reciprocal, k);
				F frequency = result, inverse = reciprocal, error(length);
				frequency.resize(length), inverse.resize(length);
				transform(frequency);
				transform(inverse);
				for (int i = 1; i < m && Size(i) < this->size(); ++i) error[i - 1] = (*this)[i] * T(i);
				inverse_product(error, frequency);
				zero(error, 0, k - 1);
				zero(error, m - 1);
				inverse_product(error, inverse);
				for (int i = m - 1; i >= k; --i) error[i] = error[i - 1] * inv[i];
				zero(error, 0, k);
				zero(error, m);
				inverse_product(error, frequency);
				result.resize(m);
				std::copy(error.begin() + k, error.begin() + m, result.begin() + k);
			}
			result.resize(n);
			for (int i = work; i < n; ++i) result[i] = dot<true>(*this, result, i) * inv[i];
			return result;
		}
	}
	F sqrt(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().sqrt(n));
		else {
			int first = 0, last = static_cast<int>(std::min(this->size(), Size(n)));
			while (first < last && (*this)[first] == T(0)) ++first;
			if (first == last) return F(n);
			if (first & 1) return {};
			auto constant = scalar_sqrt((*this)[first]);
			if (!constant) return {};
			const int shift = first / 2, target = n - shift, work = newton_precision(target), initial = std::min(work, 32);
			if (target >= 256) {
				F r = sparse<Kind::power>(first, target, 4, 0, *constant, T(2).inv());
				if (!r.empty()) { r.insert(r.begin(), shift, T(0)); return r; }
			}
			F f(this->begin() + first, this->begin() + last);
			const T half = T(2).inv(), factor = (T(2) * *constant).inv();
			F result = f.template sparse<Kind::power>(0, initial, 32, 0, *constant, half);
			if (initial == target) {
				result.insert(result.begin(), shift, T(0));
				return result;
			}
			F reciprocal = result.inv(initial);
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				F error;
				if (reciprocal.size() < Size(k)) inverse_step(result, reciprocal, k, &error);
				if (error.empty()) { error = result; transform(error); }
				F inverse = reciprocal;
				pointwise(error, error);
				transform(error, true);
				error.resize(length), inverse.resize(length);
				for (int i = 0; i < m - k; ++i) {
					T low = f.coefficient(i);
					T high = f.coefficient(k + i);
					error[i] = (low + high - error[i]) * half;
				}
				zero(error, m - k);
				transform(error);
				transform(inverse);
				pointwise(error, inverse);
				transform(error, true);
				result.resize(m);
				std::copy_n(error.begin(), m - k, result.begin() + k);
			}
			result.resize(target);
			for (int i = work; i < target; ++i)
				result[i] = (f.coefficient(i) - dot(result, result, i, i - 1)) * factor;
			result.insert(result.begin(), shift, T(0));
			return result;
		}
	}
	F pow(long long k) const {
		assert(k >= 0);
		const int n = precision(-1);
		if (!n) return {};
		if (!k) { F result(n); result[0] = 1; return result; }
		if (k == 1) return *this;
		int first = 0;
		while (first < n && (*this)[first] == T(0)) ++first;
		if (first == n || first > (n - 1) / k) return F(n);
		const int shift = static_cast<int>(first * k), len = n - shift;
		const T lead = (*this)[first];
		const T scale = lead.pow(k);
		if constexpr (std::is_same_v<T, mint>) {
			F r = sparse<Kind::power>(first, len, 12, 0, scale, T(k));
			if (!r.empty()) { r.insert(r.begin(), shift, T(0)); return r; }
		}
		F f(this->begin() + first, this->end());
		f.resize(len);
		f /= lead;
		F result = (f.log(len) * T(k)).exp(len) * scale;
		result.insert(result.begin(), shift, T(0));
		return result;
	}
	std::pair<F, F> divmod(const F &divisor) const {
		F a = *this, b = divisor;
		a.shrink(), b.shrink();
		assert(!b.empty());
		if (a.size() < b.size()) return {{}, std::move(a)};
		assert(a.size() <= (Size(1) << 22));
		const int qn = static_cast<int>(a.size() - b.size() + 1), m = static_cast<int>(b.size());
		if (m <= 32 || Size(m) * qn <= 2048) {
			F q(qn);
			const T inverse = b.back().inv();
			for (int i = qn - 1; i >= 0; --i) {
				T value = q[i] = a[i + m - 1] * inverse;
				for (int j = 0; j < m; ++j) a[i + j] -= value * b[j];
			}
			a.resize(m - 1);
			a.shrink(), q.shrink();
			return {std::move(q), std::move(a)};
		}
		F ra(a.rbegin(), a.rend()), rb(b.rbegin(), b.rend());
		F q = ra.div_series(rb, qn);
		std::reverse(q.begin(), q.end());
		F r = a - multiply(q, b);
		r.resize(m - 1);
		r.shrink(), q.shrink();
		return {std::move(q), std::move(r)};
	}

  private:
	template <bool Weighted = false>
	static T dot(const F &a, const F &b, int i, int last = -1) {
		T sum = 0;
		if (last < 0) last = i;
		for (int j = 1; j <= last && Size(j) < a.size(); ++j) {
			T value = a[j] * b[i - j];
			if constexpr (Weighted) value *= T(j);
			sum += value;
		}
		return sum;
	}
	T coefficient(int i) const { return Size(i) < this->size() ? (*this)[i] : T(0); }
	static void inverse_product(F &a, const F &frequency) {
		transform(a); pointwise(a, frequency); transform(a, true);
	}
	static void zero(F &a, int first, int last = -1) {
		std::fill(a.begin() + first, last < 0 ? a.end() : a.begin() + last, T(0));
	}
	static void transform(F &a, bool inverse = false) { fps_light_detail::transform(a, inverse); }
	static void pointwise(F &a, const F &b) { fps_light_detail::pointwise(a, b); }

	template <Kind kind>
	F sparse(int first, int n, int limit, int budget, T initial, T exponent = T(0)) const {
		auto terms = Sparse::collect(this->data() + first, int(this->size()) - first, n, limit);
		int stride = 0;
		for (auto [j, x] : terms) stride = std::gcd(stride, j);
		if (int(terms.size()) > limit || (budget && n > 256 && terms.size() > Size(budget * stride))) return {};
		if constexpr (kind == Kind::inverse || kind == Kind::power) {
			T inverse = (*this)[first].inv();
			for (auto &[j, x] : terms) x *= inverse;
			if constexpr (kind == Kind::inverse) initial = inverse;
		}
		return F(Sparse::get<kind>(std::move(terms), n, initial, exponent, inverse_table));
	}
	static int newton_precision(int n) {
		const int prefix = std::bit_floor(unsigned(n));
		return prefix >= 4096 && n - prefix <= 4 ? prefix : n;
	}
	using Native = LightFormalPowerSeries<mint>;
	Native native() const {
		Native result(this->size());
		for (Size i = 0; i < this->size(); ++i) result[i] = mint::raw((*this)[i].val());
		return result;
	}
	static F from_native(const Native &a) {
		F result(a.size());
		for (Size i = 0; i < a.size(); ++i) result[i] = T::raw(a[i].val());
		return result;
	}
	static void inverse_step(const F &f, F &result, int n, F *frequency = nullptr) {
		const int k = static_cast<int>(result.size()), length = k * 2;
		assert(k > 0 && std::has_single_bit(unsigned(k)) && k < n && n <= length);
		F error = f.pre(n), inverse = result;
		error.resize(length), inverse.resize(length);
		transform(error);
		if (frequency) *frequency = error;
		transform(inverse);
		pointwise(error, inverse);
		transform(error, true);
		zero(error, 0, k);
		inverse_product(error, inverse);
		result.resize(n);
		for (int i = k; i < n; ++i) result[i] = -error[i];
	}
	int precision(int deg) const {
		assert(this->size() <= Size(std::numeric_limits<int>::max()));
		int n = deg < 0 ? static_cast<int>(this->size()) : deg;
		assert(0 <= n && n <= (1 << 22));
		return n;
	}
	static const mint *inverse_table(int n) {
		if constexpr (std::is_same_v<T, mint>) return inverses(n).data();
		else return nullptr;
	}
	static const std::vector<T> &inverses(int n) {
		static std::vector<T> inv{T(0), T(1)};
		for (int i = static_cast<int>(inv.size()); i < n; ++i) inv.push_back(T(T::mod() - T::mod() / i) * inv[T::mod() % i]);
		return inv;
	}
	static std::optional<T> scalar_sqrt(T a) {
		if (a == T(0)) return T(0);
		if (a.pow((T::mod() - 1) / 2) != T(1)) return std::nullopt;
		constexpr int q = 119;
		T x = a.pow((q + 1) / 2), t = a.pow(q), c = T(3).pow(q);
		int m = 23;
		while (t != T(1)) {
			int i = 1;
			T next = t * t;
			while (i < m && next != T(1)) next *= next, ++i;
			assert(i < m);
			T b = c.pow(1LL << (m - i - 1));
			x *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return T::raw(std::min(x.val(), T::mod() - x.val()));
	}
};

using light_fps = LightFormalPowerSeries<>;
using LightFPS = light_fps;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

namespace fps_light_detail {
template<class T>
using ProductTree = polynomial_detail::ProductTree<LightFormalPowerSeries<T>>;
}

namespace light_fps_algorithms {
template<class T>
LightFormalPowerSeries<T> taylor_shift(LightFormalPowerSeries<T> f, std::type_identity_t<T> c) {
	using F = LightFormalPowerSeries<T>;
	const std::size_t n = f.size();
	if (n <= 1 || c == T(0)) return f;
	assert(n < T::mod());
	F factorial(n, T(1)), inverse(n, T(1)), a(n), b(n);
	for (std::size_t i = 1; i < n; ++i) factorial[i] = factorial[i - 1] * T(i);
	inverse[n - 1] = factorial[n - 1].inv();
	for (std::size_t i = n - 1; i; --i) inverse[i - 1] = inverse[i] * T(i);
	T power = 1;
	for (std::size_t i = 0; i < n; ++i) {
		a[n - 1 - i] = f[i] * factorial[i];
		b[i] = power * inverse[i];
		power *= c;
	}
	a = F::multiply(a, b);
	for (std::size_t i = 0; i < n; ++i) f[i] = a[n - 1 - i] * inverse[i];
	return f;
}

template<class T>
T bostan_mori(LightFormalPowerSeries<T> p, LightFormalPowerSeries<T> q, long long k) {
	using F = LightFormalPowerSeries<T>;
	assert(k >= 0 && !q.empty() && q[0] != T(0));
	p.shrink();
	q.shrink();
	while (k && !p.empty() && q.size() > 1) {
		F opposite = q;
		for (std::size_t i = 1; i < opposite.size(); i += 2) opposite[i] = -opposite[i];
		F numerator = F::multiply(p, opposite), denominator = F::multiply(q, opposite);
		p.resize((numerator.size() + 1 - std::size_t(k & 1)) / 2);
		for (std::size_t i = 0; i < p.size(); ++i) p[i] = numerator[2 * i + std::size_t(k & 1)];
		q.resize((denominator.size() + 1) / 2);
		for (std::size_t i = 0; i < q.size(); ++i) q[i] = denominator[2 * i];
		p.shrink();
		q.shrink();
		k >>= 1;
	}
	return static_cast<unsigned long long>(k) < p.size() ? p[std::size_t(k)] / q[0] : T(0);
}

template<class T>
T linear_recurrence(const std::vector<T> &initial, const std::vector<T> &coefficients, long long k) {
	using F = LightFormalPowerSeries<T>;
	assert(k >= 0 && initial.size() == coefficients.size());
	const std::size_t n = initial.size();
	if (static_cast<unsigned long long>(k) < n) return initial[std::size_t(k)];
	if (!n) return T(0);
	F q(n + 1);
	q[0] = T(1);
	for (std::size_t i = 0; i < n; ++i) q[i + 1] = -coefficients[i];
	F p = F::multiply(F(initial.begin(), initial.end()), q).pre(static_cast<int>(n));
	return bostan_mori(std::move(p), std::move(q), k);
}

template<class T>
std::vector<T> multipoint_eval(const LightFormalPowerSeries<T> &f, const std::vector<T> &xs) {
	if (f.size() <= 32 || xs.size() <= 32) {
		std::vector<T> result;
		result.reserve(xs.size());
		for (T x : xs) result.push_back(f.eval(x));
		return result;
	}
	return fps_light_detail::ProductTree<T>(xs).evaluate(f);
}

template<class T>
LightFormalPowerSeries<T> interpolate(const std::vector<T> &xs, const std::vector<T> &ys) {
	assert(xs.size() == ys.size());
	return fps_light_detail::ProductTree<T>(xs).interpolate(ys);
}
}

#endif
