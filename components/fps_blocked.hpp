#ifndef CP_COMPONENTS_FPS_BLOCKED_HPP
#define CP_COMPONENTS_FPS_BLOCKED_HPP
#include "fps_workspace.hpp"
#include "fps_output.hpp"
#include "../math/mint.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <span>
#include <cstring>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
using Word = std::uint32_t;
using Words = std::span<Word>;
using ConstWords = std::span<const Word>;
using std::size_t;
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP, kNI = 998244351;

}
namespace blocked_ntt {

inline constexpr Word kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec;
using kernels::kP;

constexpr Word multiply_mod(Word x, Word y) {
	return Word(std::uint64_t(x) * y % kP);
}

constexpr Word power(Word x, Word e) {
	Word r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr Word quotient(Word w) { return Word((std::uint64_t(w) << 32) / kP); }

inline constexpr Word kR = Word((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<Word, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(Word x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, Word bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(Word value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(Word value, Word value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, f.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline size_t slot(size_t k) { return (k >> 3 << 4) | (k & 7); }

inline constexpr size_t table_words(int lg) { return (size_t(1) << lg) / 8 + 16; }

inline void build_table(Word* t, size_t count, const std::array<Word, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const size_t h = size_t(1) << level;
		if (h >= count) break;
		const Word root = roots[level];
		if (h < 8) {
			for (size_t j = 0; j < h; ++j) {
				const Word v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

inline void inverse_radix4(Vec* f, size_t h, const Word* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(multiply(x, s), kP); };
	for (size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = reduce(add(p0, p1), 2 * kP), cd = reduce(add(p2, p3), 2 * kP);
		const Vec amb = reduce(diff(p0, p1), 2 * kP), cmd = multiply(diff(p2, p3), z);
		f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

}
}
namespace blocked_ntt::kernels {
using namespace detail;

template<bool Inverse, bool Identity>
inline void radix4(Vec* a, size_t h, const Word* x, const Word* y) {
	const Factor wx(Identity ? 1 : x[0], Identity ? 4 : x[8]);
	const Factor wy(Identity ? 1 : y[0], Identity ? 4 : y[8]), wz(y[1], y[9]);
#pragma GCC unroll 1
	for (size_t k = 0; k < h; ++k) {
		const Vec f0 = a[k], f1 = a[k + h], f2 = a[k + 2 * h], f3 = a[k + 3 * h];
		if constexpr (Inverse) {
			const Vec ab = reduce(add(f0, f1), 2 * kP), cd = reduce(add(f2, f3), 2 * kP);
			const Vec amb = Identity ? reduce(diff(f0, f1), 2 * kP) : multiply(diff(f0, f1), wy);
			const Vec cmd = multiply(diff(f2, f3), wz);
			a[k] = reduce(add(ab, cd), 2 * kP), a[k + h] = reduce(add(amb, cmd), 2 * kP);
			if constexpr (Identity) {
				a[k + 2 * h] = reduce(diff(ab, cd), 2 * kP);
				a[k + 3 * h] = reduce(diff(amb, cmd), 2 * kP);
			} else {
				a[k + 2 * h] = multiply(diff(ab, cd), wx);
				a[k + 3 * h] = multiply(diff(amb, cmd), wx);
			}
		} else {
			const Vec lo = reduce(f0, 2 * kP), hi = reduce(f1, 2 * kP);
			const Vec xc = Identity ? reduce(f2, 2 * kP) : multiply(f2, wx);
			const Vec xd = Identity ? reduce(f3, 2 * kP) : multiply(f3, wx);
			const Vec ac = reduce(add(lo, xc), 2 * kP), amc = reduce(diff(lo, xc), 2 * kP);
			const Vec bd = Identity ? reduce(add(hi, xd), 2 * kP) : multiply(add(hi, xd), wy);
			const Vec bmd = multiply(diff(hi, xd), wz);
			a[k] = add(ac, bd), a[k + h] = diff(ac, bd);
			a[k + 2 * h] = add(amc, bmd), a[k + 3 * h] = diff(amc, bmd);
		}
	}
}

inline void forward(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<false, false>(a, h, x, y);
}
inline void inverse(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<true, false>(a, h, x, y);
}
inline void forward_identity(Vec* a, size_t h, const Word* y) {
	radix4<false, true>(a, h, y, y);
}
inline void inverse_identity(Vec* a, size_t h, const Word* y) {
	radix4<true, true>(a, h, y, y);
}
inline void scale_radix2(Vec* a, size_t h, const Word* y) {
	const Factor s(y[1], y[9]);
	for (size_t j = 0; j < h; ++j) {
		const Vec lo = a[j], hi = a[j + h];
		a[j] = reduce(multiply(add(lo, hi), s), kP);
		a[j + h] = reduce(multiply(diff(lo, hi), s), kP);
	}
}
}

namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::forward(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::inverse(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

}

namespace blocked_poly {

inline constexpr Word kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr size_t footprint(size_t n) { return (n + 7) / 8 * 8 + 16; }
	Words take(size_t n) {
		if (size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

	bool zero_initialized() const { return storage_.get_deleter().region != nullptr; }

private:
	FpsWords storage_;
	Word *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add;
using blocked_ntt::detail::broadcast;
using blocked_ntt::detail::diff;
using blocked_ntt::detail::Factor;
using blocked_ntt::detail::kP;
using blocked_ntt::detail::kR;
using blocked_ntt::detail::reduce;
using blocked_ntt::detail::slot;
using blocked_ntt::detail::Vec;

inline constexpr auto kInverseScales = [] {
	std::array<std::array<Word, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(Word(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Vec load(const Word* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(Word* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const Word* table, size_t k) {
	const Word* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const Word* roots, size_t p) {
	const Word* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const Word* table, size_t g)
		: x(entry(table, g)), y(entry(table, 2 * g)), z(entry(table, 2 * g + 1)) {}
};

[[gnu::always_inline]] inline void forward_h1(Vec (&f)[4], const Group& w) {
	const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], w.x), d = times(f[3], w.x);
	const Vec ac = low(add(a, c)), amc = low_difference(a, c);
	const Vec bd = times(add(b, d), w.y), bmd = times(diff(b, d), w.z);
	f[0] = add(ac, bd), f[1] = diff(ac, bd), f[2] = add(amc, bmd), f[3] = diff(amc, bmd);
}

[[gnu::always_inline]] inline void inverse_h1(Vec (&f)[4], const Group& w) {
	const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
	const Vec amb = times(diff(f[0], f[1]), w.y), cmd = times(diff(f[2], f[3]), w.z);
	f[0] = low(add(ab, cd)), f[1] = low(add(amb, cmd));
	f[2] = times(diff(ab, cd), w.x), f[3] = times(diff(amb, cmd), w.x);
}

struct alignas(64) Window {
	Word word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const Word* roots,
												size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const Word* b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) {
		const Vec bi = broadcast(b[i]);
		const Vec lo = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 8 - i));
		const Vec hi = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 9 - i));
		even = _mm256_add_epi64(even, _mm256_mul_epu32(lo, bi));
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(hi, bi));
	}
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return reduce(_mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA), 2 * kP);
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Word* roots;

	void operator()(Word* a, size_t count, size_t first) const {
		for (size_t j = 0; j < count; ++j, a += 32) {
			const Group w(roots, first + j);
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, w);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
};

struct InverseBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word* inverse_roots;
	const Word* in;

	void operator()(Word* a, size_t count, size_t first) const {
		const Word* from = in + 32 * first;
		for (size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const Word* roots;
	const Word* inverse_roots;
	const Word* b;
	size_t origin = 0;

	[[gnu::always_inline]] void prepare(const Word* a, size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(Word* a, size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t - origin));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(Word* a, size_t count, size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const Word* a;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[4]) const {
		const Word* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(size_t g, Window (&window)[K][4]) const {
		for (size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t - terms[0].product.origin));
				for (size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t - terms[k].product.origin))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(Word* a, size_t count, size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const Word* b = product.b + 8 * (4 * (first + j) - product.origin);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const Word* roots, const Word* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(Word* a, size_t nv, size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

	static constexpr bool radix8(size_t nv) { return nv / 2 > kTile; }

	void below(Word* a, size_t nv, size_t k) const {
		const size_t h = nv / 4;
		for (size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr size_t kTile = 256;

	void tile(Word* a, size_t nv, size_t k) const {
		if constexpr (Bottom::kForward) {
			for (size_t h = nv / 4; h >= 16; h /= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (size_t h = 16; h < nv; h *= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(Word* a) { return reinterpret_cast<Vec*>(a); }

	void forward(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const Word *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const Word* in, size_t size, size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool in_place(const Word* out) const { return in_ == out + shift_; }

	void zero_outside(Word* out, size_t lo, size_t hi) const {
		const size_t begin = std::clamp(shift_, lo, hi), end = std::clamp(end_, begin, hi);
		std::fill(out + lo, out + begin, 0);
		std::fill(out + end, out + hi, 0);
	}

	bool within(size_t lo, size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const Word* in, size_t shift, size_t end, size_t v) {
		alignas(32) Word x[8] = {};
		for (size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const Word* in_;
	size_t shift_, end_;
	size_t first_, count_;
	size_t first_touched_, touched_;
};

template <class Input>
[[gnu::always_inline]] inline void forward_top4_body(Input in, Vec* out, size_t h, const Word* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

template <class Input>
[[gnu::always_inline]] inline void forward_top2_body(Input in, Vec* out, size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, size_t h, const Word* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, size_t h) {
	forward_top2_body(in, out, h);
}

inline bool forward_top8(const Source&, Word*, size_t, const Word*) { return false; }

template <class Bottom>
void radix2_halves(const Recursion<Bottom>& recursion, const Source& in, Word* a, size_t nv,
				   const Word* roots) {
	const size_t h = nv / 2;
	if constexpr (Bottom::kForward) {
		if (recursion.radix8(nv) && forward_top8(in, a, nv, roots)) {
			recursion.below(a, h, 0);
			recursion.below(a + 8 * h, h, 1);
			return;
		}
		forward_top2(in, reinterpret_cast<Vec*>(a), h);
	}
	recursion.visit(a, h, 0);
	recursion.visit(a + 8 * h, h, 1);
}

inline void inverse_top4(Vec* f, size_t h, Half output, const Word* inverse_roots, const Factor& s) {
	if (output == Half::kBoth) return blocked_ntt::detail::inverse_radix4(f, h, inverse_roots, s);
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output == Half::kLower) {
			f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		} else {
			f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
		}
	}
}

inline void inverse_top2(Vec* f, size_t h, Half output, Word scale) {
	if (output == Half::kBoth) {
		alignas(32) Word s[16] = {};
		s[1] = scale;
		s[9] = blocked_ntt::detail::quotient(scale);
		return blocked_ntt::kernels::scale_radix2(f, h, s);
	}
	const Factor s(scale);
	for (size_t j = 0; j < h; ++j) {
		if (output == Half::kLower) f[j] = reduce(times(add(f[j], f[j + h]), s), kP);
		else f[j + h] = reduce(times(diff(f[j], f[j + h]), s), kP);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const size_t table = blocked_ntt::detail::table_words(lg_max), entries = (size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const Word* roots() const { return roots_; }
	const Word* inverse_roots() const { return inverse_roots_; }

	void forward(ConstWords in, size_t shift, Words out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(Words a) const { forward(a, 0, a); }

	void forward_upper(ConstWords in, size_t shift, Words out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(ConstWords in, Words out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), false);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(Words a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(ConstWords in, size_t shift, Words out,
						ConstWords b, Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(Words a, ConstWords b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(ConstWords a, ConstWords b, Words out,
						Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		ConstWords a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, Words out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, Words out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), true);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(ConstWords in, size_t shift, Words out,
						ConstWords b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(Words a, ConstWords b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(Words c, ConstWords a,
					ConstWords b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

protected:
	Word inverse_scale(size_t n, bool montgomery) const {
		check_length(n);
		return detail::kInverseScales[montgomery][std::countr_zero(n)];
	}

	void check_length(size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <size_t K>
	void inverse_products(std::span<const Pair> pairs, Words out, Half output) const {
		using namespace blocked_ntt::detail;
		const Word scale = inverse_scale(out.size(), true);
		detail::InverseProductSumBottom<K> bottom{};
		for (size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	static detail::Source source(ConstWords in, size_t shift, size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(Words a, const detail::Source& in, const Bottom& bottom, Word scale,
			Half output) const {
		using namespace detail;
		check_length(a.size());
		const detail::Recursion recursion(roots_, inverse_roots_, bottom);
		const size_t nv = a.size() / 8;
		auto* v = reinterpret_cast<Vec*>(a.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const size_t h = nv / 4;
			if constexpr (Bottom::kForward) forward_top4(in, v, h, roots_);
			for (size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
			if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots_, Factor(scale));
		} else {
			const size_t h = nv / 2;
			radix2_halves(recursion, in, a.data(), nv, roots_);
			if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
		}
	}

	template <class Combine>
	void products(Word* out, const Word* a, const Word* b, size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	Word *roots_, *inverse_roots_;
};

}

namespace blocked_poly {

namespace detail {

inline Vec load_unaligned(const Word* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(Word* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec montgomery(Vec a, Vec b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b);
	Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

class Indices {
public:
	explicit Indices(size_t k) {
		alignas(32) Word x[8];
		for (Word i = 0; i < 8; ++i) x[i] = Word((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr Word kStep = Word((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline Word scalar_inverse(Word x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(size_t first, Words out, const A& a, Word factor = 1) {
	constexpr size_t kLanes = 32;
	const size_t n = out.size();
	if (n == 0) return;
	const size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(Word(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(Word(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(Word(n)), lanes);
	};
	const auto masked = [&](Vec value, size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	alignas(32) Word total[kLanes], inverse[kLanes];
	for (int v = 0; v < 4; ++v) store(total + 8 * v, canonical(prefix[v]));
	Word running = 1;
	for (size_t l = 0; l < kLanes; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (size_t l = kLanes; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	Vec inv[4];
	for (int v = 0; v < 4; ++v) inv[v] = load(inverse + 8 * v);
	const auto put = [&](size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * size_t(v) < n) put(8 * v, inv[v]);
}

}

inline Word mint_scalar(Word x) { return x; }
inline unsigned mint_scalar(mint x) { return std::min(x.v, x.v - mint::mod()); }
inline void copy_coefficients(const mint *in, Words out) {
	for (size_t i = 0; i < out.size(); ++i) out[i] = mint_scalar(in[i]);
}
template <class T> inline detail::Vec coefficients(std::span<const T> f, size_t at) {
	detail::Vec value;
	std::memcpy(&value, f.data() + at, sizeof(value));
	if constexpr (std::is_same_v<T, mint>) return detail::reduce(value, detail::kP);
	else return value;
}

template <class T> inline void derivative_impl(std::span<const T> f, Words d) {
	using namespace detail;
	const size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(coefficients(f, i + 1), index.value())));
	for (size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(Word(i + 1), mint_scalar(f[i + 1])) : 0;
}

inline void derivative(ConstWords f, Words d) { derivative_impl(f, d); }

inline void divide_by_index(ConstWords a, size_t first, Words q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	const size_t n = a.size();
	detail::divide_by_index(first, q, [a, n](size_t at) __attribute__((target("avx2"))) {
		if (at + 8 <= n) return detail::load_unaligned(a.data() + at);
		alignas(32) Word rest[8] = {};
		std::copy(a.begin() + at, a.end(), rest);
		return detail::load(rest);
	});
}

}

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>

namespace blocked_poly {

namespace detail {

inline constexpr size_t kInverseBase = 32;

inline void inverse_direct(ConstWords f, Words g) {
	using blocked_ntt::detail::multiply_mod;
	const Word inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (size_t j = 1; j <= std::min(i, f.size() - 1); ++j) {
			sum += std::uint64_t(f[j]) * g[i - j] % kP;
		}
		g[i] = multiply_mod(Word(sum % kP ? kP - sum % kP : 0), inv0);
	}
}

}

inline int inverse_log(size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<size_t>(n, 2) - 1)));
}

inline size_t inverse_scratch(size_t n) { return 2 * Arena::footprint(size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, ConstWords f, ConstWords g,
						Words to, Words gt, Words work) {
	const size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, detail::kP - 1);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, ConstWords f, Words g,
					Words scratch) {
	const size_t n = g.size();
	size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

}

namespace blocked_poly {

namespace detail {

inline constexpr size_t kLogBase = 64;

inline constexpr size_t kLogBlocks = 4;

inline void log_direct(ConstWords f, Words g) {
	using blocked_ntt::detail::multiply_mod;
	Word a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / Word(i), inv[kP % i]);
	g[0] = 0;
	for (size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = multiply_mod(Word(i), a[i]);
		for (size_t k = 1; k < i; ++k) sum += kP - std::uint64_t(kg[k]) * a[i - k] % kP;
		kg[i] = Word(sum % kP);
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline size_t log_block(size_t n) {
	return std::max<size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline size_t log_blocks(size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

template <class T> inline void subtract_derivative(std::span<const T> f, size_t first, size_t count,
									Word* out, size_t size) {
	const size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(coefficients(f, first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (size_t c = full; c < count; ++c) {
		const Word d = c < inside ? blocked_ntt::detail::multiply_mod(Word(first + 1 + c), mint_scalar(f[first + 1 + c])) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}
inline void subtract_derivative_impl(ConstWords f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative(f, first, count, out, size);
}

inline void subtract_from_derivative(ConstWords f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative_impl(f, first, count, out, size);
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	Word* w3;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (size_t t = 0; t < 4; ++t) {
			const size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const Word* side;

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + (leaf - 8 * term.product.origin)), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline size_t log_buffers(size_t n) { return std::max<size_t>(log_blocks(n) + 2, 3); }

inline Vec mint_load(std::span<const mint> f, size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(size_t lo, size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const mint> f, size_t at) { return mint_load(f, at); }
	std::span<const mint> f_;
	size_t full_, touched_;
};
[[gnu::noinline]] inline void mint_forward_top4(MintSource in, Vec* out, size_t h, const Word* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void mint_forward_top2(MintSource in, Vec* out, size_t h) {
	forward_top2_body(in, out, h);
}

inline void mint_forward(const Transform& t, std::span<const mint> f, Words out) {
	const MintSource in(f);
	const detail::Recursion recursion(t.roots(), t.inverse_roots(), ForwardBottom{t.roots()});
	const size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		mint_forward_top4(in, v, h, t.roots());
		for (size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
	} else {
		const size_t h = nv / 2;
		mint_forward_top2(in, v, h);
		recursion.visit(out.data(), h, 0);
		recursion.visit(out.data() + 8 * h, h, 1);
	}
}
inline void mint_derivative(std::span<const mint> f, Words d) { derivative_impl(f, d); }
inline void mint_subtract_from_derivative(std::span<const mint> f, size_t first, size_t count, Word* out, size_t size) {
	subtract_derivative(f, first, count, out, size);
}

struct NoLogTransform {
	void operator()(size_t, ConstWords) const {}
};

template <class Input, class Sink, class Transformed>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, Input f, size_t n,
												Words scratch, ConstWords prefix, const Sink& sink, const Transformed& transformed = {}) {
	const size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const Words ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, prefix, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (size_t s = 1; s < blocks; ++s) {
		const size_t from = std::min(f.size(), (s - 1) * k);
		if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (size_t j = 0; j < blocks; ++j) {
		const size_t first = j * k, count = std::min(k, n - 1 - first);
		const Words work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
			else derivative_impl(f.first(std::min(f.size(), count + 1)), work.first(count));
		} else {
			if (j == 1) {
				t.inverse_product(window(1), q0, work, Half::kUpper);
			} else if (blocks == 3) {
				const Transform::Pair pairs[2] = {{window(2), q0}, {window(1), work}};
				t.inverse_product_sum(pairs, work, Half::kUpper);
			} else if (j == 2) {
				t.inverse_with(ToeplitzBottom{t.roots(), t.inverse_roots(), window(1).data(), window(2).data(), q0.data(),
											work.data(), window(3).data()},
							work, Half::kUpper);
			} else {
				const InverseProductBottom product{{t.roots(), t.inverse_roots(), work.data()}, window(1).data()};
				t.inverse_with(SideProductBottom{product, window(3).data()}, work, Half::kUpper);
			}
			if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_subtract_from_derivative(f, first, count, work.data(), k);
			else subtract_derivative_impl(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, ConstWords(work.first(count)));
		if (j + 1 < blocks) {
			t.forward(work.first(k), 0, work);
			transformed(j, ConstWords(work));
		}
	}
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, ConstWords f, size_t n,
		Words scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative(t, f, n, scratch, f, sink, transformed);
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void mint_log_derivative(const Transform& t, std::span<const mint> f, size_t n,
		Words scratch, ConstWords prefix, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative(t, f, n, scratch, prefix, sink, transformed);
}

inline int log_derivative_log(size_t n) { return std::countr_zero(2 * log_block(n)); }

inline size_t log_derivative_scratch(size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}

inline int log_log(size_t n) {
	return n <= detail::kLogBase ? Transform::kMinLog : std::countr_zero(2 * detail::log_block(n));
}

inline size_t log_scratch(size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_buffers(n) * Arena::footprint(2 * detail::log_block(n));
}

inline void log(const Transform& t, ConstWords f, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

}

namespace blocked_poly {
inline void native_log(const mint *in, size_t input, std::vector<mint> &out, size_t n, bool integrate) {
	prepare_fps_output(out, n);
	out.emplace_back(0);
	const size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	const bool direct = n >= 262144;
	const auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;
	Arena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(prefix_size), scratch = arena.take(log_scratch(n));
	copy_coefficients(in, f);
	const auto sink = [&out, integrate, n](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<Word *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
		}
		const auto *values = integrate ? q.data() + detail::log_block(n) : q.data();
		if (integrate) append_fps_words<true>(out, values, q.size());
		else append_fps_words<false>(out, values, q.size());
	};
	if (direct) detail::mint_log_derivative(t, {in, c}, n, scratch, f, sink);
	else detail::log_derivative(t, f, n, scratch, sink);
}
}
}
#pragma GCC pop_options
#endif
#endif
