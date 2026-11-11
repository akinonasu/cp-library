#ifndef CP_FPS_LIGHT_CORE_HPP
#define CP_FPS_LIGHT_CORE_HPP

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

#include "fps_light_ntt.hpp"
#include "fps_sparse_scalar.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_light_detail {
template<class T = mint>
struct [[nodiscard, gnu::warn_unused]] FormalPowerSeries : std::vector<T> {
	static_assert(T::mod() == 998244353);
	using base = std::vector<T>;
	using F = FormalPowerSeries;
	using Sparse = fps_detail::ScalarSparseSeries;
	using Kind = Sparse::Kind;
	using Size = std::size_t;
	using base::base;
	FormalPowerSeries() = default;
	FormalPowerSeries(base a) : base(std::move(a)) {}
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
	F &operator<<=(int d) {
		assert(d >= 0);
		auto edge = std::shift_right(this->begin(), this->end(), d);
		std::fill(this->begin(), edge, T(0));
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		auto edge = std::shift_left(this->begin(), this->end(), d);
		std::fill(edge, this->end(), T(0));
		return *this;
	}
	friend F operator<<(F a, int d) { return a <<= d; }
	friend F operator>>(F a, int d) { return a >>= d; }

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min(Size(n), this->size()));
	}
	void shrink() { while (!this->empty() && this->back() == T(0)) this->pop_back(); }
	void trim() { shrink(); }
	[[nodiscard]] T eval(T x) const {
		T res = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) res = res * x + *it;
		return res;
	}
	F diff() const {
		F res(this->size());
		for (Size i = 1; i < this->size(); ++i) res[i - 1] = (*this)[i] * T(i);
		return res;
	}
	F integral() const {
		assert(this->size() < T::mod());
		F res(this->size());
		const auto &inv = inverses(static_cast<int>(this->size()));
		for (Size i = 1; i < this->size(); ++i) res[i] = (*this)[i - 1] * inv[i];
		return res;
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
			F res = pre(initial).inv(initial);
			const T recip = res[0];
			for (int k = initial; k < work; k *= 2) inverse_step(*this, res, std::min(k * 2, work));
			res.resize(n);
			for (int i = work; i < n; ++i) res[i] = -dot(*this, res, i) * recip;
			return res;
		}
	}
	F div_series(const F &g, int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!g.empty() && g[0] != T(0));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().div_series(g.native(), n));
		else {
			if (n <= 32) {
				F res(n);
				const T recip = g[0].inv();
				for (int i = 0; i < n; ++i) {
					res[i] = (coefficient(i) - dot(g, res, i)) * recip;
				}
				return res;
			}
			const int work = newton_precision(n), k = std::bit_floor(unsigned(work - 1)), length = k * 2;
			F recip = g.inv(k), res = pre(k);
			const T constant = recip[0];
			recip.resize(length), res.resize(length);
			transform(recip);
			inverse_product(res, recip);
			zero(res, k);
			F quotient = res, error = g.pre(work);
			error.resize(length);
			transform(quotient);
			inverse_product(error, quotient);
			zero(error, 0, k);
			for (int i = k; i < work; ++i) error[i] = coefficient(i) - error[i];
			zero(error, work);
			inverse_product(error, recip);
			res.resize(n);
			std::copy(error.begin() + k, error.begin() + work, res.begin() + k);
			for (int i = work; i < n; ++i) {
				res[i] = (coefficient(i) - dot(g, res, i)) * constant;
			}
			return res;
		}
	}
	F log(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!this->empty() && (*this)[0] == T(1));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().log(n));
		else {
			if (n >= 256) if (auto r = sparse<Kind::logarithm>(0, n, 12, 0, T(0)); !r.empty()) return r;
			F res = pre(n).diff().div_series(*this, n - 1);
			res.resize(n);
			return res.integral();
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
			F res = pre(initial).exp(initial);
			F reciprocal = res.inv(), recip;
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				F freq = res;
				freq.resize(length);
				transform(freq);
				if (k > initial) inverse_update(freq.pre(k), recip, reciprocal, k);
				F error(k);
				for (int i = 1; i < k && Size(i) < this->size(); ++i) error[i - 1] = (*this)[i] * T(i);
				inverse_product(error, freq);
				const T last = -error.back();
				for (int i = k - 1; i; --i) error[i] = res[i] * T(i) - error[i - 1];
				error[0] = last;
				error.resize(length);
				recip = reciprocal;
				recip.resize(length);
				transform(recip);
				inverse_product(error, recip);
				for (int i = 0; i < m - k; ++i) error[i] = coefficient(k + i) - error[i] * inv[k + i];
				zero(error, m - k);
				inverse_product(error, freq);
				res.resize(m);
				std::copy_n(error.begin(), m - k, res.begin() + k);
			}
			res.resize(n);
			for (int i = work; i < n; ++i) res[i] = dot<true>(*this, res, i) * inv[i];
			return res;
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
			const T half = T(2).inv();
			F res = f.template sparse<Kind::power>(0, initial, 32, 0, *constant, half);
			if (initial == target) {
				res.insert(res.begin(), shift, T(0));
				return res;
			}
			F reciprocal = res.inv();
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				F error;
				if (k > initial) inverse_step(res, reciprocal, k, &error);
				if (error.empty()) { error = res; transform(error); }
				F recip = reciprocal;
				pointwise(error, error);
				transform(error, true);
				error.resize(length), recip.resize(length);
				for (int i = 0; i < m - k; ++i) {
					T low = f.coefficient(i);
					T high = f.coefficient(k + i);
					error[i] = (low + high - error[i]) * half;
				}
				zero(error, m - k);
				transform(error);
				transform(recip);
				pointwise(error, recip);
				transform(error, true);
				res.resize(m);
				std::copy_n(error.begin(), m - k, res.begin() + k);
			}
			res.resize(target);
			for (int i = work; i < target; ++i)
				res[i] = (f.coefficient(i) - dot(res, res, i, i - 1)) * reciprocal[0] * half;
			res.insert(res.begin(), shift, T(0));
			return res;
		}
	}
	F pow(long long k) const {
		assert(k >= 0);
		const int n = precision(-1);
		if (!n) return {};
		if (!k) { F res(n); res[0] = 1; return res; }
		if (k == 1) return *this;
		int first = 0;
		while (first < n && (*this)[first] == T(0)) ++first;
		if (first == n || first > (n - 1) / k) return F(n);
		const int shift = static_cast<int>(first * k), len = n - shift;
		const T scale = (*this)[first].pow(k);
		if constexpr (std::is_same_v<T, mint>) {
			F r = sparse<Kind::power>(first, len, 12, 0, scale, T(k));
			if (!r.empty()) { r.insert(r.begin(), shift, T(0)); return r; }
		}
		if (k <= 4 && first == 0) {
			F square = multiply(*this, *this).pre(n);
			return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
		}
		F f(this->begin() + first, this->end());
		f.resize(len);
		f /= f[0];
		F res = (f.log(len) * T(k)).exp(len) * scale;
		res.insert(res.begin(), shift, T(0));
		return res;
	}
	[[nodiscard]] std::pair<F, F> divmod(const F &divisor) const {
		F a = *this, b = divisor;
		a.shrink(), b.shrink();
		assert(!b.empty());
		if (a.size() < b.size()) return {{}, std::move(a)};
		assert(a.size() <= (Size(1) << 22));
		const int qn = static_cast<int>(a.size() - b.size() + 1), m = static_cast<int>(b.size());
		if (m <= 32 || Size(m) * qn <= 2048) {
			F q(qn);
			const T recip = b.back().inv();
			for (int i = qn - 1; i >= 0; --i) {
				T value = q[i] = a[i + m - 1] * recip;
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
	static void inverse_product(F &a, const F &freq) {
		transform(a); pointwise(a, freq); transform(a, true);
	}
	static void zero(F &a, int first, int last = -1) {
		std::fill(a.begin() + first, last < 0 ? a.end() : a.begin() + last, T(0));
	}
	static void transform(F &a, bool recip = false) { fps_light_detail::transform(a, recip); }
	static void pointwise(F &a, const F &b) { fps_light_detail::pointwise(a, b); }

	template <Kind kind>
	F sparse(int first, int n, int limit, int budget, T initial, T exponent = T(0)) const {
		auto terms = Sparse::collect(this->data() + first, int(this->size()) - first, n, limit);
		int stride = 0;
		for (auto [j, x] : terms) stride = std::gcd(stride, j);
		if (int(terms.size()) > limit || (budget && n > 256 && terms.size() > Size(budget * stride))) return {};
		if constexpr (kind == Kind::inverse || kind == Kind::power) {
			T recip = (*this)[first].inv();
			for (auto &[j, x] : terms) x *= recip;
			if constexpr (kind == Kind::inverse) initial = recip;
		}
		return F(Sparse::get<kind>(std::move(terms), n, initial, exponent, inverse_table));
	}
	static int newton_precision(int n) {
		const int prefix = std::bit_floor(unsigned(n));
		return prefix >= 4096 && n - prefix <= 4 ? prefix : n;
	}
	using Native = FormalPowerSeries<mint>;
	Native native() const {
		Native res(this->size());
		for (Size i = 0; i < this->size(); ++i) res[i] = mint::raw((*this)[i].val());
		return res;
	}
	static F from_native(const Native &a) {
		F res(a.size());
		for (Size i = 0; i < a.size(); ++i) res[i] = T::raw(a[i].val());
		return res;
	}
	static void inverse_step(const F &f, F &res, int n, F *freq = nullptr) {
		const int k = static_cast<int>(res.size()), length = k * 2;
		assert(k > 0 && std::has_single_bit(unsigned(k)) && k < n && n <= length);
		F error = f.pre(n), recip = res;
		error.resize(length), recip.resize(length);
		transform(error);
		if (freq) *freq = error;
		transform(recip);
		inverse_update(std::move(error), recip, res, n);
	}
	static void inverse_update(F error, const F &recip, F &res, int n) {
		const int k = static_cast<int>(res.size());
		pointwise(error, recip);
		transform(error, true);
		zero(error, 0, k);
		inverse_product(error, recip);
		res.resize(n);
		for (int i = k; i < n; ++i) res[i] = -error[i];
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
		for (int i = static_cast<int>(inv.size()); i < n; ++i) inv.push_back(T::raw(T::mod() - T::mod() / i) * inv[T::mod() % i]);
		return inv;
	}
	static std::optional<T> scalar_sqrt(T a) {
		if (a == T(1)) return a;
		constexpr int q = 119;
		T t = a.pow((q - 1) / 2), x = t * a, c = T(3).pow(q);
		t *= x;
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T next = t; next != T(1) && i < m; next *= next) ++i;
			if (i == m) return std::nullopt;
			T b = c.pow(1LL << (m - i - 1));
			x *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return T::raw(std::min(x.val(), T::mod() - x.val()));
	}
};
}

template<class T = mint>
using LightFormalPowerSeries = fps_light_detail::FormalPowerSeries<T>;

using light_fps = LightFormalPowerSeries<>;
using LightFPS = light_fps;

namespace light_fps_algorithms {
template<class T>
using Series = LightFormalPowerSeries<T>;
}

#ifndef CP_FPS_API
#define CP_FPS_API 1
#define CP_FPS_LIGHT_API 1
using fps_light_detail::FormalPowerSeries;
using fps = light_fps;
#endif

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
