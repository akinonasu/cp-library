#ifndef FPS_SPARSE_CORE_HPP
#define FPS_SPARSE_CORE_HPP

#include "fps_sparse_terms.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

namespace fps_detail {
template <class E> struct SparseBase : SparseTerms {
	static Terms multiply(const Terms &a, const Terms &b, int n) {
		Terms result;
		for (auto [i, x] : a) for (auto [j, y] : b) if (i < n - j) result.emplace_back(i + j, x * y);
		std::sort(result.begin(), result.end(), [](auto a, auto b) { return a.first < b.first; });
		std::size_t used = 0;
		for (auto [i, x] : result) {
			if (used && result[used - 1].first == i) result[used - 1].second += x;
			else result[used++] = {i, x};
		}
		result.resize(used);
		result.erase(std::remove_if(result.begin(), result.end(), [](auto term) { return term.second == mint(0); }), result.end());
		return result;
	}
	template <Kind kind>
	static Series get(Terms terms, int n, mint initial, mint exponent, const mint *(*inverses)(int)) {
		assert(n >= 0 && terms.size() <= 64);
		if constexpr (kind != Kind::inverse) assert(unsigned(n) < mint::mod());
		if (!n) return {};
		if (terms.empty()) { Series out(n); out[0] = initial; return out; }
		if constexpr (kind == Kind::power) {
			unsigned e = exponent.val();
			if (e <= 4 && terms.size() <= 12 && (n >= 4096 || terms.size() <= 3)) {
				Terms power{{0, 1}}, f{{0, 1}};
				f.insert(f.end(), terms.begin(), terms.end());
				while (e) {
					if (e & 1) power = multiply(power, f, n);
					e >>= 1;
					if (e) f = multiply(f, f, n);
				}
				Series out(n);
				for (auto [i, x] : power) out[i] = x * initial;
				return out;
			}
		}
		int stride = 0;
		for (auto term : terms) stride = std::gcd(stride, term.first);
		const int count = (n - 1) / stride + 1;
		for (auto &term : terms) term.first /= stride;
		const mint *inv = nullptr;
		if constexpr (kind != Kind::inverse) inv = inverses(count);
		Series compressed;
		if (terms.size() == 1 && kind != Kind::power) {
			compressed.resize(count);
			if constexpr (kind == Kind::exponential) {
				E::template geometric<true>(compressed, initial, terms[0].second, inv);
			} else {
				E::geometric(compressed, kind == Kind::logarithm ? mint(-1) : initial, -terms[0].second);
				if constexpr (kind == Kind::logarithm) compressed[0] = 0, E::scale(compressed, inv);
			}
		} else compressed = E::template solve<kind>(terms, count, initial, exponent, inv);
		if (stride == 1) return compressed;
		Series out(n);
		for (int i = 0; i < count; ++i) out[i * stride] = compressed[i];
		return out;
	}
};
}
#endif
