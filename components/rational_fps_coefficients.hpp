#ifndef CP_COMPONENTS_RATIONAL_FPS_COEFFICIENTS_HPP
#define CP_COMPONENTS_RATIONAL_FPS_COEFFICIENTS_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace rational_fps_detail {
template <class F, class Transform>
struct Coefficients {
	using T = typename F::value_type;
	static F middle(F f, F g) {
		const int d = int(f.size()) - 1, count = int(g.size()) - d;
		if (std::min(f.size(), std::size_t(count)) <= 16) {
			F result(count);
			for (int i = 0; i < count; ++i)
				for (int j = 0; j <= d; ++j) result[i] += f[j] * g[d + i - j];
			return result;
		}
		const auto n = std::bit_ceil(g.size());
		f.resize(n), g.resize(n);
		Transform::forward(f), Transform::forward(g);
		Transform::product(g, f);
		Transform::inverse(g);
		return F(g.begin() + d, g.begin() + d + count);
	}
	static F inverse(F q, long long l, long long r) {
		if (l < 0) {
			F result = inverse(std::move(q), 0, r);
			result.insert(result.begin(), std::size_t(-l), T(0));
			return result;
		}
		if (r < (long long)q.size()) q.resize(std::size_t(r) + 1);
		const int d = int(q.size()) - 1, count = int(r - l + 1);
		if (r < 2LL * std::max(d, count) && r < (1 << 23)) {
			F result = q.inv(int(r) + 1);
			return F(result.begin() + l, result.end());
		}
		F opposite = q, frequency, v;
		for (std::size_t i = 1; i < q.size(); i += 2) opposite[i] = -opposite[i];
		if (d <= 16) {
			v = F::multiply(q, opposite);
			for (int i = 0; i <= d; ++i) v[i] = v[2 * i];
			v.resize(d + 1);
		} else {
			const auto n = std::bit_ceil(unsigned(d));
			frequency = q;
			frequency.resize(2 * n);
			Transform::forward(frequency);
			v.resize(n);
			for (unsigned i = 0; i < n; ++i) v[i] = frequency[2 * i] * frequency[2 * i + 1];
			Transform::inverse(v);
			if (unsigned(d) == n) {
				T lead = q.back() * q.back();
				if (d & 1) lead = -lead;
				v[0] -= lead;
				v.push_back(lead);
			} else v.resize(d + 1);
		}
		const long long x = l - d, next = x / 2 + (x > 0 && x % 2);
		F w = inverse(std::move(v), next, r / 2), s(count + d);
		for (std::size_t i = 0, offset = std::size_t(2 * next - x); i < w.size(); ++i) s[2 * i + offset] = w[i];
		if (!frequency.empty() && std::bit_ceil(s.size()) == frequency.size()) {
			s.resize(frequency.size());
			Transform::forward(s);
			for (std::size_t i = 0; i < frequency.size(); i += 2) std::swap(frequency[i], frequency[i + 1]);
			Transform::product(s, frequency);
			Transform::inverse(s);
			return F(s.begin() + d, s.begin() + d + count);
		}
		return middle(std::move(opposite), std::move(s));
	}
	static F get(F p, F q, long long l, long long r) {
		assert(0 <= l && l <= r && !q.empty() && q[0] != T(0));
		assert(r - l <= (1 << 22) && std::max(p.size(), q.size()) <= (1 << 22) - std::size_t(r - l));
		if (l == r) return {};
		while (!p.empty() && p.back() == T(0)) p.pop_back();
		while (q.size() > 1 && q.back() == T(0)) q.pop_back();
		if (p.empty()) return F(std::size_t(r - l));
		if (q.size() == 1) {
			F result(std::size_t(r - l));
			T scale = q[0].inv();
			for (long long i = l; i < r && i < (long long)p.size(); ++i) result[std::size_t(i - l)] = p[std::size_t(i)] * scale;
			return result;
		}
		if (r <= (long long)(2 * std::max(p.size() + q.size(), std::size_t(r - l))) && r < (1 << 23)) {
			F result = p.div_series(q, int(r));
			return F(result.begin() + l, result.end());
		}
		F w = inverse(std::move(q), l - (long long)p.size() + 1, r - 1);
		return middle(std::move(p), std::move(w));
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
