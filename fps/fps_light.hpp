#ifndef FPS_LIGHT_HPP
#define FPS_LIGHT_HPP

#include "../components/fps_light_ntt.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T = mint>
struct LightFormalPowerSeries : std::vector<T> {
	static_assert(T::mod() == 998244353);
	using base = std::vector<T>;
	using F = LightFormalPowerSeries;
	using base::base;
	LightFormalPowerSeries() = default;
	LightFormalPowerSeries(base a) : base(std::move(a)) {}
	F &operator=(base a) { base::operator=(std::move(a)); return *this; }
	F &operator=(std::initializer_list<T> a) { base::operator=(a); return *this; }
	static F multiply(const F &a, const F &b) {
		if constexpr (std::is_same_v<T, mint>) return fps_light_detail::multiply(a, b);
		else {
			std::vector<mint> x(a.size()), y;
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = mint::raw(a[i].val());
			if (&a != &b) {
				y.resize(b.size());
				for (std::size_t i = 0; i < b.size(); ++i) y[i] = mint::raw(b[i].val());
			}
			auto z = fps_light_detail::multiply(x, &a == &b ? x : y);
			F result(z.size());
			for (std::size_t i = 0; i < z.size(); ++i) result[i] = T::raw(z[i].val());
			return result;
		}
	}
	F operator-() const { F r = *this; for (T &x : r) x = -x; return r; }
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
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
		const auto n = this->size();
		if (std::size_t(d) >= n) return *this = F(n);
		this->insert(this->begin(), d, T(0));
		this->resize(n);
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		const auto n = this->size();
		this->erase(this->begin(), this->begin() + std::min(n, std::size_t(d)));
		this->resize(n);
		return *this;
	}
	friend F operator<<(F a, int d) { return a <<= d; }
	friend F operator>>(F a, int d) { return a >>= d; }
	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min(std::size_t(n), this->size()));
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
		for (std::size_t i = 1; i < this->size(); ++i) result[i - 1] = (*this)[i] * T(i);
		return result;
	}
	F integral() const {
		assert(this->size() < T::mod());
		F result(this->size());
		const auto &inv = inverses(static_cast<int>(this->size()));
		for (std::size_t i = 1; i < this->size(); ++i) result[i] = (*this)[i - 1] * inv[i];
		return result;
	}
	void onemul(int d, T c) {
		assert(d >= 0);
		if (!d) { *this *= T(1) + c; return; }
		for (std::size_t i = this->size(); i > std::size_t(d); --i) (*this)[i - 1] += (*this)[i - 1 - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (!d) { *this /= T(1) + c; return; }
		for (std::size_t i = d; i < this->size(); ++i) (*this)[i] -= (*this)[i - d] * c;
	}
	F inv(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		assert(!this->empty() && (*this)[0] != T(0));
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().inv(n));
		else {
			const int work = newton_precision(n), initial = std::min(work, 32);
			F result(initial);
			const T inverse = (*this)[0].inv();
			result[0] = inverse;
			for (int i = 1; i < initial; ++i) {
				T sum = 0;
				for (int j = 1; j <= i && std::size_t(j) < this->size(); ++j) sum += (*this)[j] * result[i - j];
				result[i] = -sum * inverse;
			}
			for (int k = initial; k < work; k *= 2) inverse_step(*this, result, std::min(k * 2, work));
			result.resize(n);
			for (int i = work; i < n; ++i) {
				T sum = 0;
				for (int j = 1; j <= i && std::size_t(j) < this->size(); ++j) sum += (*this)[j] * result[i - j];
				result[i] = -sum * inverse;
			}
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
					T value = std::size_t(i) < this->size() ? (*this)[i] : T(0);
					for (int j = 1; j <= i && std::size_t(j) < g.size(); ++j) value -= g[j] * result[i - j];
					result[i] = value * inverse;
				}
				return result;
			}
			const int work = newton_precision(n), k = std::bit_floor(unsigned(work - 1)), length = k * 2;
			F inverse = g.inv(k), result = pre(k);
			const T constant = inverse[0];
			inverse.resize(length), result.resize(length);
			fps_light_detail::transform(inverse);
			fps_light_detail::transform(result);
			fps_light_detail::pointwise(result, inverse);
			fps_light_detail::transform(result, true);
			std::fill(result.begin() + k, result.end(), T(0));
			F quotient = result, error = g.pre(work);
			error.resize(length);
			fps_light_detail::transform(quotient);
			fps_light_detail::transform(error);
			fps_light_detail::pointwise(error, quotient);
			fps_light_detail::transform(error, true);
			std::fill(error.begin(), error.begin() + k, T(0));
			for (int i = k; i < work; ++i) error[i] = (std::size_t(i) < this->size() ? (*this)[i] : T(0)) - error[i];
			std::fill(error.begin() + work, error.end(), T(0));
			fps_light_detail::transform(error);
			fps_light_detail::pointwise(error, inverse);
			fps_light_detail::transform(error, true);
			result.resize(n);
			std::copy(error.begin() + k, error.begin() + work, result.begin() + k);
			for (int i = work; i < n; ++i) {
				T value = std::size_t(i) < this->size() ? (*this)[i] : T(0);
				for (int j = 1; j <= i && std::size_t(j) < g.size(); ++j) value -= g[j] * result[i - j];
				result[i] = value * constant;
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
			const int work = newton_precision(n), initial = std::min(work, 32);
			const auto &inv = inverses(n);
			F result(initial);
			result[0] = T(1);
			for (int i = 1; i < initial; ++i) {
				T sum = 0;
				for (int j = 1; j <= i && std::size_t(j) < this->size(); ++j) sum += T(j) * (*this)[j] * result[i - j];
				result[i] = sum * inv[i];
			}
			if (work == n && initial == n) return result;
			F reciprocal = result.inv(initial);
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				if (reciprocal.size() < std::size_t(k)) inverse_step(result, reciprocal, k);
				F frequency = result, inverse = reciprocal, error(length);
				frequency.resize(length), inverse.resize(length);
				fps_light_detail::transform(frequency);
				fps_light_detail::transform(inverse);
				for (int i = 1; i < m && std::size_t(i) < this->size(); ++i) error[i - 1] = (*this)[i] * T(i);
				fps_light_detail::transform(error);
				fps_light_detail::pointwise(error, frequency);
				fps_light_detail::transform(error, true);
				std::fill(error.begin(), error.begin() + k - 1, T(0));
				std::fill(error.begin() + m - 1, error.end(), T(0));
				fps_light_detail::transform(error);
				fps_light_detail::pointwise(error, inverse);
				fps_light_detail::transform(error, true);
				for (int i = m - 1; i >= k; --i) error[i] = error[i - 1] * inv[i];
				std::fill(error.begin(), error.begin() + k, T(0));
				std::fill(error.begin() + m, error.end(), T(0));
				fps_light_detail::transform(error);
				fps_light_detail::pointwise(error, frequency);
				fps_light_detail::transform(error, true);
				result.resize(m);
				std::copy(error.begin() + k, error.begin() + m, result.begin() + k);
			}
			result.resize(n);
			for (int i = work; i < n; ++i) {
				T sum = 0;
				for (int j = 1; j <= i && std::size_t(j) < this->size(); ++j) sum += T(j) * (*this)[j] * result[i - j];
				result[i] = sum * inv[i];
			}
			return result;
		}
	}
	F sqrt(int deg = -1) const {
		const int n = precision(deg);
		if (!n) return {};
		if constexpr (!std::is_same_v<T, mint>) return from_native(native().sqrt(n));
		else {
			int first = 0, last = static_cast<int>(std::min(this->size(), std::size_t(n)));
			while (first < last && (*this)[first] == T(0)) ++first;
			if (first == last) return F(n);
			if (first & 1) return {};
			auto constant = scalar_sqrt((*this)[first]);
			if (!constant) return {};
			const int shift = first / 2, target = n - shift, work = newton_precision(target), initial = std::min(work, 32);
			F f(this->begin() + first, this->begin() + last), result(initial);
			const T half = T(2).inv(), factor = (T(2) * *constant).inv();
			result[0] = *constant;
			for (int i = 1; i < initial; ++i) {
				T value = std::size_t(i) < f.size() ? f[i] : T(0);
				for (int j = 1; j < i; ++j) value -= result[j] * result[i - j];
				result[i] = value * factor;
			}
			if (initial == target) {
				result.insert(result.begin(), shift, T(0));
				return result;
			}
			F reciprocal = result.inv(initial);
			for (int k = initial; k < work; k *= 2) {
				const int length = k * 2, m = std::min(length, work);
				F error;
				if (reciprocal.size() < std::size_t(k)) inverse_step(result, reciprocal, k, &error);
				if (error.empty()) { error = result; fps_light_detail::transform(error); }
				F inverse = reciprocal;
				fps_light_detail::pointwise(error, error);
				fps_light_detail::transform(error, true);
				error.resize(length), inverse.resize(length);
				for (int i = 0; i < m - k; ++i) {
					T low = std::size_t(i) < f.size() ? f[i] : T(0);
					T high = std::size_t(k + i) < f.size() ? f[k + i] : T(0);
					error[i] = (low + high - error[i]) * half;
				}
				std::fill(error.begin() + m - k, error.end(), T(0));
				fps_light_detail::transform(error);
				fps_light_detail::transform(inverse);
				fps_light_detail::pointwise(error, inverse);
				fps_light_detail::transform(error, true);
				result.resize(m);
				std::copy_n(error.begin(), m - k, result.begin() + k);
			}
			result.resize(target);
			for (int i = work; i < target; ++i) {
				T value = std::size_t(i) < f.size() ? f[i] : T(0);
				for (int j = 1; j < i; ++j) value -= result[j] * result[i - j];
				result[i] = value * factor;
			}
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
		F f(this->begin() + first, this->end());
		f.resize(len);
		f /= lead;
		F result = (f.log(len) * T(k)).exp(len) * lead.pow(k);
		result.insert(result.begin(), shift, T(0));
		return result;
	}
	std::pair<F, F> divmod(const F &divisor) const {
		F a = *this, b = divisor;
		a.shrink(), b.shrink();
		assert(!b.empty());
		if (a.size() < b.size()) return {{}, std::move(a)};
		assert(a.size() <= (std::size_t(1) << 22));
		const int qn = static_cast<int>(a.size() - b.size() + 1), m = static_cast<int>(b.size());
		if (m <= 32 || std::size_t(m) * qn <= 2048) {
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
	static int newton_precision(int n) {
		const int prefix = std::bit_floor(unsigned(n));
		return prefix >= 4096 && n - prefix <= 4 ? prefix : n;
	}
	using Native = LightFormalPowerSeries<mint>;
	Native native() const {
		Native result(this->size());
		for (std::size_t i = 0; i < this->size(); ++i) result[i] = mint::raw((*this)[i].val());
		return result;
	}
	static F from_native(const Native &a) {
		F result(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) result[i] = T::raw(a[i].val());
		return result;
	}
	static void inverse_step(const F &f, F &result, int n, F *frequency = nullptr) {
		const int k = static_cast<int>(result.size()), length = k * 2;
		assert(k > 0 && std::has_single_bit(unsigned(k)) && k < n && n <= length);
		F error = f.pre(n), inverse = result;
		error.resize(length), inverse.resize(length);
		fps_light_detail::transform(error);
		if (frequency) *frequency = error;
		fps_light_detail::transform(inverse);
		fps_light_detail::pointwise(error, inverse);
		fps_light_detail::transform(error, true);
		std::fill(error.begin(), error.begin() + k, T(0));
		fps_light_detail::transform(error);
		fps_light_detail::pointwise(error, inverse);
		fps_light_detail::transform(error, true);
		result.resize(n);
		for (int i = k; i < n; ++i) result[i] = -error[i];
	}
	int precision(int deg) const {
		assert(this->size() <= std::size_t(std::numeric_limits<int>::max()));
		int n = deg < 0 ? static_cast<int>(this->size()) : deg;
		assert(0 <= n && n <= (1 << 22));
		return n;
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

#endif
