#ifndef ONLINE_CONVOLUTION_LIGHT_HPP
#define ONLINE_CONVOLUTION_LIGHT_HPP

#include "../fps/fps_light.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <type_traits>
#include <vector>

template <class T = mint>
class LightOnlineConvolution {
	static_assert(T::mod() == 998244353);
	using M = mint;
	using V = std::vector<M>;
	struct Prefix { V a, b; };
	int limit;
	V a, b, c, f, g;
	std::vector<Prefix> prefixes;
	static M import(T x) {
		if constexpr (std::is_same_v<T, M>) return x;
		else return M::raw(x.val());
	}
	static T export_value(M x) {
		if constexpr (std::is_same_v<T, M>) return x;
		else return T::raw(x.val());
	}
	static void forward(V &v) { fps_light_detail::transform(v); }
	static void inverse(V &v) { fps_light_detail::transform(v, true); }
	void extend(int q, int d) {
		int end = std::min(q + d, limit);
		if (int(c.size()) < end) c.resize(end);
		if (d < 8) {
			for (int k = q; k < end; ++k) {
				M sum = 0;
				if (q == d) {
					for (int i = k - q + 1; i < q; ++i) sum += a[i] * b[k - i];
				} else {
					for (int i = q - d; i < q; ++i) sum += a[i] * b[k - i] + b[i] * a[k - i];
				}
				c[k] += sum;
			}
			return;
		}
		if (q == d) {
			int level = std::countr_zero(unsigned(q));
			prefixes.resize(level + 1);
			auto &p = prefixes[level];
			p.a.assign(a.begin(), a.end());
			p.b.assign(b.begin(), b.end());
			forward(p.a); forward(p.b);
			f = p.a;
			fps_light_detail::pointwise(f, p.b);
			inverse(f);
			for (int k = q; k < end; ++k) c[k] += f[k - q] - c[k - q];
		} else {
			int n = 2 * d, level = std::countr_zero(unsigned(n));
			const auto &p = prefixes[level];
			f.assign(n, M(0)); g.assign(n, M(0));
			std::copy_n(a.begin() + q - d, d, f.begin());
			std::copy_n(b.begin() + q - d, d, g.begin());
			forward(f); forward(g);
			fps_light_detail::pointwise(f, p.b);
			fps_light_detail::pointwise(g, p.a);
			for (int i = 0; i < n; ++i) f[i] += g[i];
			inverse(f);
			for (int k = q; k < end; ++k) c[k] += f[d + k - q];
		}
	}
public:
	explicit LightOnlineConvolution(int max_size = 1 << 23) : limit(max_size) { assert(0 <= limit && limit <= (1 << 23)); }
	int size() const { return a.size(); }
	int max_size() const { return limit; }
	bool empty() const { return a.empty(); }
	void reserve(int n) {
		assert(0 <= n && n <= limit);
		a.reserve(n); b.reserve(n); c.reserve(n);
	}
	void clear() { a.clear(); b.clear(); c.clear(); f.clear(); g.clear(); prefixes.clear(); }
	T operator[](int i) const { assert(0 <= i && i < size()); return export_value(c[i]); }
	T pending() const { assert(size() < limit); return export_value(size() < int(c.size()) ? c[size()] : M(0)); }
	T push(T x, T y) {
		int n = size();
		assert(n < limit);
		a.push_back(import(x)); b.push_back(import(y));
		if (n >= int(c.size())) c.resize(n + 1);
		c[n] += a[n] * b[0];
		if (n) c[n] += b[n] * a[0];
		int q = n + 1;
		if (q < limit) extend(q, q & -q);
		return export_value(c[n]);
	}
};

#endif
