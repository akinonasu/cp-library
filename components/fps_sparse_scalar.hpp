#ifndef FPS_SPARSE_SCALAR_HPP
#define FPS_SPARSE_SCALAR_HPP

#include "fps_sparse_terms.hpp"

namespace fps_detail {
struct ScalarSparseSeries : SparseTerms {
	template <Kind kind>
	static Series get(Terms terms, int n, mint initial, mint exponent, const mint *(*inverses)(int)) {
		assert(n >= 0 && terms.size() <= 64);
		if constexpr (kind != Kind::inverse) assert(unsigned(n) < mint::mod());
		if (!n) return {};
		Series out(n);
		out[0] = initial;
		if (terms.empty()) return out;
		if constexpr (kind == Kind::power) {
			unsigned e = exponent.val();
			if (e <= 4 && terms.size() <= 12 && (n >= 4096 || terms.size() <= 3)) {
				Terms power{{0, initial}};
				for (unsigned k = 0; k < e; ++k) {
					Terms next = power;
					for (auto [i, x] : power) for (auto [j, y] : terms)
						if (i < n - j) next.emplace_back(i + j, x * y);
					power = std::move(next);
				}
				out[0] = 0;
				for (auto [i, x] : power) out[i] += x;
				return out;
			}
		}
		int stride = 0;
		for (auto [j, x] : terms) stride = std::gcd(stride, j);
		const int count = (n - 1) / stride + 1;
		for (auto &[j, x] : terms) j /= stride;
		const mint *inv = nullptr;
		if constexpr (kind != Kind::inverse) inv = inverses(count);
		if (terms.size() == 1 && kind != Kind::power) {
			mint value = kind == Kind::logarithm ? mint(-1) : initial;
			mint ratio = kind == Kind::exponential ? terms[0].second : -terms[0].second;
			for (int i = 1; i < count; ++i) {
				value *= ratio;
				if constexpr (kind == Kind::exponential) value *= inv[i];
				out[i * stride] = value;
			}
		} else for (int i = 1; i < count; ++i) {
			mint sum = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				if constexpr (kind == Kind::power) x *= (exponent + mint(1)) * mint(j) - mint(i);
				if constexpr (kind == Kind::exponential) x *= mint(j);
				if constexpr (kind == Kind::inverse || kind == Kind::logarithm) x = -x;
				sum += x * out[(i - j) * stride];
				if constexpr (kind == Kind::logarithm) if (j == i) sum -= mint(j) * x;
			}
			if constexpr (kind == Kind::power || kind == Kind::exponential) sum *= inv[i];
			out[i * stride] = sum;
		}
		if constexpr (kind == Kind::logarithm) for (int i = 1; i < count; ++i) out[i * stride] *= inv[i];
		return out;
	}
};
}
#endif
