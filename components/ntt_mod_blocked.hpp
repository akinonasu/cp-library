#ifndef NTT_MOD_BLOCKED_HPP
#define NTT_MOD_BLOCKED_HPP
#include "ntt_mod_core.hpp"
#include "fps_workspace.hpp"
#include <type_traits>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace ntt_mod_detail {
template <std::uint32_t P, std::uint32_t Root>
class Blocked {
	using Word = std::uint32_t;
	using Vec = __m256i;
	static_assert(P < (Word(1) << 30));
	static constexpr Word neg_inverse = negative_inverse<P>;
	static constexpr Word radix = (std::uint64_t(1) << 32) % P;
	static constexpr Word quotient(Word x) { return (std::uint64_t(x) << 32) / P; }
	static constexpr Word product(Word x, Word y) { return std::uint64_t(x) * y % P; }
	static Vec broadcast(Word x) { return _mm256_set1_epi32(x); }
	static Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
	static Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(add(x, broadcast(2 * P)), y); }
	static Vec reduce(Vec x, Word bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }
	static Vec low(Vec x) { return reduce(x, 2 * P); }
	static Vec canonical(Vec x) { return reduce(low(x), P); }
	static Vec load(const Word *p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
	static void store(Word *p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }
	struct Factor {
		Vec w, q;
		explicit Factor(Word x) : w(broadcast(x)), q(broadcast(quotient(x))) {}
		Factor(Word x, Word y) : w(broadcast(x)), q(broadcast(y)) {}
	};
	static Vec times(Vec x, const Factor &f) {
		auto even = _mm256_mul_epu32(x, f.q);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
		auto q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
		return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.w), _mm256_mullo_epi32(q, broadcast(P)));
	}
	static Vec montgomery(Vec even, Vec odd) {
		const Vec ni = broadcast(neg_inverse), p = broadcast(P);
		even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
		return low(_mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa));
	}
	static std::size_t slot(std::size_t k) { return (k >> 3 << 4) | (k & 7); }
	static Factor entry(const Word *r, std::size_t k) { return Factor(r[slot(k)], r[slot(k) + 8]); }
	static void build_table(Word *r, std::size_t count, bool inverse) {
		r[0] = 1; r[8] = quotient(1);
		for (std::size_t h = 1; h < count; h *= 2) {
			Word root = power<P>(Root, (P - 1) / (4 * h));
			if (inverse) root = power<P>(root, P - 2);
			if (h < 8) for (std::size_t j = 0; j < h; ++j) {
				Word x = product(r[slot(j)], root);
				r[slot(h + j)] = x; r[slot(h + j) + 8] = quotient(x);
			} else {
				const Factor by(root), to_radix(radix);
				for (std::size_t j = 0; j < h; j += 8) {
					const Vec x = reduce(times(load(r + slot(j)), by), P);
					const Vec xr = reduce(times(x, to_radix), P);
					store(r + slot(h + j), x);
					store(r + slot(h + j) + 8, _mm256_mullo_epi32(xr, broadcast(neg_inverse)));
				}
			}
		}
	}
	template <bool inverse, bool identity>
	static void stage(Vec *a, std::size_t h, const Word *r, std::size_t k) {
		const Factor x = entry(r, k), y = entry(r, 2 * k), z = entry(r, 2 * k + 1);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec f0 = a[j], f1 = a[j + h], f2 = a[j + 2 * h], f3 = a[j + 3 * h];
			if constexpr (inverse) {
				const Vec ab = low(add(f0, f1)), cd = low(add(f2, f3));
				const Vec amb = identity ? low(diff(f0, f1)) : times(diff(f0, f1), y);
				const Vec cmd = times(diff(f2, f3), z);
				a[j] = low(add(ab, cd)); a[j + h] = low(add(amb, cmd));
				a[j + 2 * h] = identity ? low(diff(ab, cd)) : times(diff(ab, cd), x);
				a[j + 3 * h] = identity ? low(diff(amb, cmd)) : times(diff(amb, cmd), x);
			} else {
				const Vec f0l = low(f0), f1l = low(f1);
				const Vec xc = identity ? low(f2) : times(f2, x), xd = identity ? low(f3) : times(f3, x);
				const Vec ac = low(add(f0l, xc)), amc = low(diff(f0l, xc));
				const Vec bd = identity ? low(add(f1l, xd)) : times(add(f1l, xd), y);
				const Vec bmd = times(diff(f1l, xd), z);
				a[j] = add(ac, bd); a[j + h] = diff(ac, bd);
				a[j + 2 * h] = add(amc, bmd); a[j + 3 * h] = diff(amc, bmd);
			}
		}
	}
	template <bool inverse>
	static void stage(Vec *a, std::size_t h, const Word *r, std::size_t k) {
		if (k) stage<inverse, false>(a, h, r, k); else stage<inverse, true>(a, h, r, k);
	}
	static void forward_h1(Vec (&f)[4], const Word *r, std::size_t g) {
		const Factor x = entry(r, g), y = entry(r, 2 * g), z = entry(r, 2 * g + 1);
		const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], x), d = times(f[3], x);
		const Vec ac = low(add(a, c)), amc = low(diff(a, c));
		const Vec bd = times(add(b, d), y), bmd = times(diff(b, d), z);
		f[0] = add(ac, bd); f[1] = diff(ac, bd); f[2] = add(amc, bmd); f[3] = diff(amc, bmd);
	}
	static void inverse_h1(Vec (&f)[4], const Word *r, std::size_t g) {
		const Factor x = entry(r, g), y = entry(r, 2 * g), z = entry(r, 2 * g + 1);
		const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
		const Vec amb = times(diff(f[0], f[1]), y), cmd = times(diff(f[2], f[3]), z);
		f[0] = low(add(ab, cd)); f[1] = low(add(amb, cmd));
		f[2] = times(diff(ab, cd), x); f[3] = times(diff(amb, cmd), x);
	}
	struct alignas(32) Window { Word word[24]; };
	static void prepare(Word *a, std::size_t g, const Word *r, Window (&w)[4], bool transformed) {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		if (!transformed) forward_h1(f, r, g);
		const Factor y = entry(r, 2 * g), z = entry(r, 2 * g + 1);
		for (int t = 0; t < 4; ++t) {
			f[t] = canonical(f[t]);
			w[t].word[16] = 0;
			store(w[t].word + 8, f[t]);
			const Vec wa = times(f[t], t < 2 ? y : z);
			store(w[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * P), wa) : wa, P));
		}
	}
	static Vec leaf_product(const Window &w, const Word *b) {
		Vec even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) {
			const Vec bi = broadcast(b[i]);
			const Vec lo = _mm256_loadu_si256(reinterpret_cast<const Vec*>(w.word + 8 - i));
			const Vec hi = _mm256_loadu_si256(reinterpret_cast<const Vec*>(w.word + 9 - i));
			even = _mm256_add_epi64(even, _mm256_mul_epu32(lo, bi));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(hi, bi));
		}
		return montgomery(even, odd);
	}
	template <bool multiplying, bool transformed = false>
	void bottom(Word *a, const Word *b, std::size_t count, std::size_t first) const {
		if constexpr (multiplying) {
			Window w[2][4];
			prepare(a, first, roots_, w[0], transformed);
			for (std::size_t j = 0; j < count; ++j, a += 32, b += 32) {
				if (j + 1 < count) prepare(a + 32, first + j + 1, roots_, w[(j + 1) & 1], transformed);
				Vec f[4];
				for (int t = 0; t < 4; ++t) f[t] = leaf_product(w[j & 1][t], b + 8 * t);
				inverse_h1(f, inverse_, first + j);
				for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
			}
		} else for (std::size_t j = 0; j < count; ++j, a += 32) {
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, roots_, first + j);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
	template <bool multiplying, bool transformed = false>
	void visit(Word *a, const Word *b, std::size_t nv, std::size_t k) const {
		if (nv > 256) {
			const std::size_t h = nv / 4;
			if constexpr (!transformed) stage<false>(reinterpret_cast<Vec*>(a), h, roots_, k);
			for (std::size_t t = 0; t < 4; ++t) visit<multiplying, transformed>(a + 8 * t * h, multiplying ? b + 8 * t * h : b, h, 4 * k + t);
			if constexpr (multiplying) stage<true>(reinterpret_cast<Vec*>(a), h, inverse_, k);
			return;
		}
		if constexpr (!transformed) for (std::size_t h = nv / 4; h >= 4; h /= 4)
			for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g)
				stage<false>(reinterpret_cast<Vec*>(a + 8 * j), h, roots_, g);
		bottom<multiplying, transformed>(a, b, nv / 4, k * (nv / 4));
		if constexpr (multiplying)
			for (std::size_t h = 4; h < nv; h *= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g)
					stage<true>(reinterpret_cast<Vec*>(a + 8 * j), h, inverse_, g);
	}
	template <bool multiplying, bool transformed = false>
	void run(Word *a, const Word *b, Word factor) const {
		const std::size_t nv = n_ / 8;
		auto *v = reinterpret_cast<Vec*>(a);
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			if constexpr (!transformed) stage<false>(v, h, roots_, 0);
			for (std::size_t t = 0; t < 4; ++t) visit<multiplying, transformed>(a + 8 * t * h, multiplying ? b + 8 * t * h : b, h, t);
			if constexpr (multiplying) {
				const Factor z = entry(inverse_, 1), s(factor);
				for (std::size_t j = 0; j < h; ++j) {
					const Vec p0 = v[j], p1 = v[j + h], p2 = v[j + 2 * h], p3 = v[j + 3 * h];
					const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
					const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
					v[j] = reduce(times(add(ab, cd), s), P); v[j + h] = reduce(times(add(amb, cmd), s), P);
					v[j + 2 * h] = reduce(times(diff(ab, cd), s), P); v[j + 3 * h] = reduce(times(diff(amb, cmd), s), P);
				}
			}
		} else {
			const std::size_t h = nv / 2;
			if constexpr (!transformed) for (std::size_t j = 0; j < h; ++j) {
				const Vec x = v[j], y = v[j + h];
				v[j] = add(x, y); v[j + h] = diff(x, y);
			}
			visit<multiplying, transformed>(a, b, h, 0); visit<multiplying, transformed>(a + 8 * h, multiplying ? b + 8 * h : b, h, 1);
			if constexpr (multiplying) {
				const Factor s(factor);
				for (std::size_t j = 0; j < h; ++j) {
					const Vec x = v[j], y = v[j + h];
					v[j] = reduce(times(add(x, y), s), P); v[j + h] = reduce(times(diff(x, y), s), P);
				}
			}
		}
	}
	std::size_t n_;
	fps_detail::FpsWords table_;
	Word *roots_, *inverse_;
public:
	static constexpr std::size_t table_words(std::size_t n) { return n / 4 + 64; }
	explicit Blocked(std::size_t n, Word *workspace = nullptr) : n_(n), table_(workspace ? fps_detail::FpsWords{} : fps_detail::allocate_fps_words(table_words(n))), roots_(workspace ? workspace : table_.get()), inverse_(roots_ + n / 8 + 32) {
		assert(std::has_single_bit(n) && n >= 64 && (P - 1) % (n / 8) == 0);
		build_table(roots_, n / 16, false); build_table(inverse_, n / 16, true);
	}
	void forward(Word *a) const { run<false>(a, nullptr, 0); }
	void square(Word *a, Word factor = 1) const {
		Word scale = product(product(power<P>(n_ / 8, P - 2), radix), factor);
		run<true, true>(a, a, scale);
	}
	void multiply(Word *a, const Word *b, Word factor = 1) const {
		Word scale = product(product(power<P>(n_ / 8, P - 2), radix), factor);
		run<true>(a, b, scale);
	}
};
inline constexpr std::size_t blocked_workspace_words(std::size_t n) { return 2 * n + n / 4 + 96; }
template <std::uint32_t P, std::uint32_t Root, class T>
std::vector<std::uint32_t> multiply_blocked(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t factor, std::uint32_t *workspace = nullptr) {
	using Word = std::uint32_t;
	const auto &small = a.size() <= b.size() ? a : b;
	const auto &large = a.size() <= b.size() ? b : a;
	const bool blocked = large.size() / small.size() >= 8, square = &a == &b;
	const std::size_t length = a.size() + b.size() - 1;
	const std::size_t n = std::bit_ceil(blocked ? 2 * small.size() - 1 : length), block = n - small.size() + 1;
	auto own = workspace ? fps_detail::FpsWords{} : fps_detail::allocate_fps_words(blocked_workspace_words(n));
	Word *x = workspace ? workspace : own.get(), *y = x + n + 16;
	Blocked<P, Root> plan(n, y + n + 16);
	const auto reduce = [](T value) {
		if constexpr (std::is_unsigned_v<T>) return Word(value % P);
		else {
			auto r = value % std::int64_t(P);
			return Word(r < 0 ? r + P : r);
		}
	};
	for (std::size_t i = 0; i < small.size(); ++i) y[i] = reduce(small[i]);
	std::fill(y + small.size(), y + n, 0);
	plan.forward(y);
	if (square) {
		plan.square(y, factor);
		return std::vector<Word>(y, y + length);
	}
	std::vector<Word> result(blocked ? length : 0);
	for (std::size_t start = 0; start < large.size(); start += block) {
		const std::size_t size = std::min(block, large.size() - start);
		for (std::size_t i = 0; i < size; ++i) x[i] = reduce(large[start + i]);
		std::fill(x + size, x + n, 0);
		plan.multiply(x, y, factor);
		if (!blocked) return std::vector<Word>(x, x + length);
		for (std::size_t i = 0; i < size + small.size() - 1; ++i) {
			Word sum = result[start + i] + x[i];
			result[start + i] = sum < P ? sum : sum - P;
		}
	}
	return result;
}

}
#pragma GCC pop_options
#endif
#endif
