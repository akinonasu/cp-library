#ifndef CP_COMPONENTS_FPS_EVALUATION_COMMON_HPP
#define CP_COMPONENTS_FPS_EVALUATION_COMMON_HPP

#include "fps_blocked.hpp"
#include <optional>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace evaluation_poly {
using std::size_t;
using Word = std::uint32_t;
using Span = std::span<Word>;
using ConstSpan = std::span<const Word>;
using blocked_poly::Transform;
using blocked_poly::Half;
using blocked_poly::inverse;
using blocked_poly::inverse_log;
using blocked_poly::inverse_scratch;
using blocked_poly::derivative;
inline constexpr Word kModulus = blocked_poly::kModulus;
class Arena : public blocked_poly::Arena {
public:
	explicit Arena(size_t words) : blocked_poly::Arena(words) {}
	Span take(size_t n) {
		auto span = blocked_poly::Arena::take(n);
		if (!zero_initialized()) std::fill(span.begin(), span.end(), 0u);
		return span;
	}
};
namespace detail {
using namespace blocked_poly::detail;
}
}
namespace evaluation_poly {
namespace detail {
inline Vec add_mod(Vec x, Vec y) { return reduce(add(x, y), kP); }
inline Vec subtract_mod(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }
inline Vec to_montgomery(Vec x) { return reduce(times(x, Factor(kR)), kP); }
inline void forward_small(Vec* f, size_t m, size_t k, const Word* roots) {
	if (m == 1) return;
	const size_t h = m / 2;
	const Factor s = entry(roots, k);
	for (size_t j = 0; j < h; ++j) {
		const Vec lo = f[j], hi = reduce(times(f[j + h], s), kP);
		f[j] = add_mod(lo, hi), f[j + h] = subtract_mod(lo, hi);
	}
	forward_small(f, h, 2 * k, roots);
	forward_small(f + h, h, 2 * k + 1, roots);
}
inline void inverse_small(Vec* f, size_t m, size_t k, const Word* inverse_roots) {
	if (m == 1) return;
	const size_t h = m / 2;
	inverse_small(f, h, 2 * k, inverse_roots);
	inverse_small(f + h, h, 2 * k + 1, inverse_roots);
	const Factor s = entry(inverse_roots, k);
	for (size_t j = 0; j < h; ++j) {
		const Vec a = f[j], b = f[j + h];
		f[j] = add_mod(a, b), f[j + h] = reduce(times(_mm256_sub_epi32(add(a, broadcast(kP)), b), s), kP);
	}
}
inline void transpose8(Vec (&r)[8]) {
	Vec t[8], u[8];
	for (int i = 0; i < 8; i += 2) t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]), t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
	for (int i = 0; i < 8; i += 4)
		for (int k = 0; k < 2; ++k)
			u[i + 2 * k] = _mm256_unpacklo_epi64(t[i + k], t[i + k + 2]), u[i + 2 * k + 1] = _mm256_unpackhi_epi64(t[i + k], t[i + k + 2]);
	for (int i = 0; i < 4; ++i)
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20), r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
}
inline Word montgomery_scalar(Word x, Word y) {
	constexpr Word kInverseR = blocked_ntt::detail::power(kR, kP - 2);
	return blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(x, y), kInverseR);
}
inline Vec fold(Vec s) {
	return _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(s, 32), broadcast(kR)), _mm256_blend_epi32(s, _mm256_setzero_si256(), 0xAA));
}
[[gnu::always_inline]] inline Vec montgomery_sums(Vec even, Vec odd, size_t terms) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	if (terms > 12) even = fold(even), odd = fold(odd);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return canonical(_mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA));
}
[[gnu::always_inline]] inline void multiply_lanes(const Word* a, size_t m, const Word* b, size_t k,
												Word* c) {
	Vec x[33], xo[33], y[33], yo[33];
#pragma GCC unroll 33
	for (size_t i = 0; i <= m; ++i) x[i] = load(a + 8 * i), xo[i] = _mm256_srli_epi64(x[i], 32);
#pragma GCC unroll 33
	for (size_t j = 0; j <= k; ++j) y[j] = load(b + 8 * j), yo[j] = _mm256_srli_epi64(y[j], 32);
#pragma GCC unroll 65
	for (size_t t = 0; t <= m + k; ++t) {
		Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
		const size_t first = t > k ? t - k : 0, last = std::min(t, m);
#pragma GCC unroll 17
		for (size_t i = first; i <= last; ++i) {
			even = _mm256_add_epi64(even, _mm256_mul_epu32(x[i], y[t - i]));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(xo[i], yo[t - i]));
		}
		store(c + 8 * t, montgomery_sums(even, odd, last - first + 1));
	}
}
inline void multiply_lanes_any(const Word* a, size_t m, const Word* b, size_t k, Word* c) {
	if (m == k) {
		switch (m) {
			case 1: return multiply_lanes(a, 1, b, 1, c);
			case 2: return multiply_lanes(a, 2, b, 2, c);
			case 4: return multiply_lanes(a, 4, b, 4, c);
			case 8: return multiply_lanes(a, 8, b, 8, c);
			case 16: return multiply_lanes(a, 16, b, 16, c);
		}
	}
	multiply_lanes(a, m, b, k, c);
}
class Stack {
public:
	explicit Stack(Span memory) : base_(memory.data()), size_(memory.size()) {}
	size_t mark() const { return top_; }
	void release(size_t mark) { top_ = mark; }
	Word* take(size_t n) {
		const size_t need = Arena::footprint(n);
		if (top_ + need > size_) std::abort();
		Word* const p = base_ + top_;
		top_ += need;
		return p;
	}
private:
	Word* base_;
	size_t size_, top_ = 0;
};
}
class TreeTransform : private Transform {
public:
	static constexpr size_t words(int lg_max) {
		return Arena::footprint(2 * blocked_ntt::detail::table_words(std::max(lg_max, Transform::kMinLog)));
	}
	TreeTransform(Arena& arena, int lg_max) : Transform(arena, std::max(lg_max, Transform::kMinLog)), requested_log_(lg_max) {
		if (lg_max < 3 || lg_max > Transform::kMaxLog) std::abort();
	}
	int lg_max() const { return requested_log_; }
	void forward(ConstSpan in, size_t shift, Span out) const {
		using namespace detail;
		const Source src = source(in, shift, out.size());
		if (out.size() < 64) return small(src, out, 0);
		run(out, src, ForwardBottom{roots_}, 0, Half::kBoth);
	}
	void forward_upper(ConstSpan in, size_t shift, Span out) const {
		check_length(2 * out.size());
		const detail::Source src = source(in, shift, out.size());
		if (out.size() < 64) return small(src, out, 1);
		Transform::forward_upper(in, shift, out);
	}
	void inverse(ConstSpan in, Span out, Word c = 1) const {
		using namespace detail;
		check_length(out.size());
		if (out.size() < 64) {
			const size_t nv = out.size() / 8;
			Vec f[4];
			for (size_t j = 0; j < nv; ++j) f[j] = load(in.data() + 8 * j);
			inverse_small(f, nv, 0, inverse_roots_);
			const Factor s(scale(out.size(), c));
			for (size_t j = 0; j < nv; ++j) store(out.data() + 8 * j, reduce(times(f[j], s), kP));
			return;
		}
		run(out, Source(nullptr, 0, 0), InverseBottom{inverse_roots_, in.data()}, scale(out.size(), c), Half::kBoth);
	}
	void inverse_upper(ConstSpan in, Span out, Word c = 1) const {
		using namespace detail;
		check_length(2 * out.size());
		const size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		const Factor s(scale(out.size(), c));
		if (out.size() < 64) {
			Vec f[4];
			for (size_t j = 0; j < nv; ++j) f[j] = load(in.data() + 8 * j);
			inverse_small(f, nv, 1, inverse_roots_);
			for (size_t j = 0; j < nv; ++j) v[j] = reduce(times(f[j], s), kP);
			return;
		}
		const Recursion recursion(roots_, inverse_roots_, InverseBottom{inverse_roots_, in.data() - out.size()});
		if (std::countr_zero(nv) % 2 == 0) {
			const size_t h = nv / 4;
			for (size_t t = 0; t < 4; ++t) recursion.visit(out.data() + 8 * t * h, h, 4 + t);
			const Group w(inverse_roots_, 1);
			for (size_t j = 0; j < h; ++j) {
				Vec f[4] = {v[j], v[j + h], v[j + 2 * h], v[j + 3 * h]};
				inverse_h1(f, w);
				for (size_t t = 0; t < 4; ++t) v[j + t * h] = reduce(times(f[t], s), kP);
			}
			return;
		}
		const size_t h = nv / 2;
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
		const Factor scale_z(blocked_ntt::detail::multiply_mod(scale(out.size(), c), inverse_roots_[1]));
		for (size_t j = 0; j < h; ++j) {
			const Vec u = v[j], w = v[j + h];
			v[j] = reduce(times(add(u, w), s), kP), v[j + h] = reduce(times(diff(u, w), scale_z), kP);
		}
	}
	void leaf_products(const Word* a, const Word* b, Span out) const {
		using namespace detail;
		const size_t leaves = out.size() / 8;
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (size_t p = 0; p < leaves; ++p) {
			if (p + 1 < leaves) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			store(out.data() + 8 * p, reduce(leaf_product(window[p & 1], b + 8 * p), kP));
		}
	}
	void standard_to_lanes(const Word* in, size_t stride, size_t n, Word* out) const {
		using namespace detail;
		for (size_t p = 0; p < n / 8; ++p) {
			Vec f[8];
			for (size_t l = 0; l < 8; ++l) f[l] = load(in + l * stride + 8 * p);
			transpose8(f);
			const Factor z = entry(roots_, p);
			Vec lo[4], hi[4];
			for (size_t t = 0; t < 4; ++t) {
				const Vec zb = times(f[t + 4], z);
				lo[t] = add(f[t], zb), hi[t] = diff(f[t], zb);
			}
			forward_h1(lo, Group(roots_, 2 * p));
			forward_h1(hi, Group(roots_, 2 * p + 1));
			for (size_t t = 0; t < 4; ++t) store(out + 64 * p + 8 * t, canonical(lo[t])), store(out + 64 * p + 32 + 8 * t, canonical(hi[t]));
		}
	}
	void leaf_product_sums(const Word* a, const Word* b, const Word* c, const Word* d,
						Span out) const {
		using namespace detail;
		const size_t leaves = out.size() / 8;
		Window window[2][2];
		fill_window(window[0][0], load(a), leaf_weight(roots_, 0));
		fill_window(window[0][1], load(c), leaf_weight(roots_, 0));
		for (size_t p = 0; p < leaves; ++p) {
			if (p + 1 < leaves) {
				const Factor w = leaf_weight(roots_, p + 1);
				fill_window(window[(p + 1) & 1][0], load(a + 8 * p + 8), w);
				fill_window(window[(p + 1) & 1][1], load(c + 8 * p + 8), w);
			}
			const Vec ab = leaf_product(window[p & 1][0], b + 8 * p), cd = leaf_product(window[p & 1][1], d + 8 * p);
			store(out.data() + 8 * p, canonical(add(ab, cd)));
		}
	}
	static void pointwise_products(const Word* a, const Word* b, Span out) {
		using namespace detail;
		for (size_t i = 0; i < out.size(); i += 8) store(out.data() + i, reduce(montgomery(load(a + i), load(b + i)), kP));
	}
	static void pointwise_product_sums(const Word* a, const Word* b, const Word* c, const Word* d,
									Span out) {
		using namespace detail;
		for (size_t i = 0; i < out.size(); i += 8) {
			const Vec x = load(a + i), y = load(b + i), z = load(c + i), w = load(d + i);
			Vec even = _mm256_add_epi64(_mm256_mul_epu32(x, y), _mm256_mul_epu32(z, w));
			Vec odd = _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)),
									_mm256_mul_epu32(_mm256_srli_epi64(z, 32), _mm256_srli_epi64(w, 32)));
			store(out.data() + i, montgomery_sums(even, odd, 2));
		}
	}
private:
	Word scale(size_t n, Word c) const {
		const auto lg = std::countr_zero(n);
		if (lg > Transform::kMaxLog) std::abort();
		return blocked_ntt::detail::multiply_mod(detail::kInverseScales[0][lg], c);
	}
	void check_length(size_t n) const {
		if (!std::has_single_bit(n) || n < 8 || std::countr_zero(n) > requested_log_) std::abort();
	}
	detail::Source source(ConstSpan in, size_t shift, size_t n) const {
		check_length(n);
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}
	void small(const detail::Source& src, Span out, size_t block) const {
		using namespace detail;
		const size_t nv = out.size() / 8;
		Vec f[4];
		for (size_t j = 0; j < nv; ++j) f[j] = src(j);
		forward_small(f, nv, block, roots_);
		for (size_t j = 0; j < nv; ++j) store(out.data() + 8 * j, f[j]);
	}
	int requested_log_;
};
struct LaneLayout {
	static constexpr size_t kWords = 8;
	static constexpr Word kBase = 32;
	using Value = detail::Vec;
	struct Node {
		Value degree, lead;
	};
	static void multiply(const Word* a, size_t m, const Word* b, size_t k, Word* c) {
		detail::multiply_lanes_any(a, m, b, k, c);
	}
	static size_t length(Word degree) { return std::bit_ceil(std::max<Word>(degree, 1)); }
	static void product(const TreeTransform&, const Word* a, const Word* b, Span out) {
		TreeTransform::pointwise_products(a, b, out);
	}
	static Value lead(const Word* c, Value degree) {
		const Value index = _mm256_add_epi32(_mm256_slli_epi32(degree, 3), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_i32gather_epi32(reinterpret_cast<const int*>(c), index, 4);
	}
	static Value sum(Value x, Value y) { return _mm256_add_epi32(x, y); }
	static Value lead_product(Value x, Value y) { return detail::reduce(detail::montgomery(x, y), detail::kP); }
	static Value top(Value degree, Value lead, size_t length) {
		return _mm256_and_si256(_mm256_cmpeq_epi32(degree, detail::broadcast(Word(length))), lead);
	}
	static void subtract(Word* c, Value x) { detail::store(c, detail::subtract_mod(detail::load(c), x)); }
	static void add(Word* c, Value x) { detail::store(c, detail::add_mod(detail::load(c), x)); }
	static void put(Word* c, Value x) { detail::store(c, x); }
};
struct StandardLayout {
	static constexpr size_t kWords = 1;
	static constexpr Word kBase = 0;
	using Value = Word;
	struct Node {
		Value degree, lead;
	};
	static void multiply(const Word*, size_t, const Word*, size_t, Word*) {}
	static size_t length(Word degree) { return std::max<size_t>(8, std::bit_ceil(degree)); }
	static void product(const TreeTransform& t, const Word* a, const Word* b, Span out) {
		t.leaf_products(a, b, out);
	}
	static Value lead(const Word* c, Value degree) { return c[degree]; }
	static Value sum(Value x, Value y) { return x + y; }
	static Value lead_product(Value x, Value y) { return detail::montgomery_scalar(x, y); }
	static Value top(Value degree, Value lead, size_t length) { return degree == length ? lead : 0; }
	static void subtract(Word* c, Value x) { *c = *c >= x ? *c - x : *c + detail::kP - x; }
	static void add(Word* c, Value x) { *c = *c + x >= detail::kP ? *c + x - detail::kP : *c + x; }
	static void put(Word* c, Value x) { *c = x; }
};
struct DropTransforms {
	void operator()(size_t, size_t, ConstSpan) const {}
};
template <class Layout, class Leaves, class Keep = DropTransforms>
class ProductTree {
public:
	using Value = typename Layout::Value;
	using Node = typename Layout::Node;
	static constexpr bool kPlaces = requires(Keep k, size_t i) { k.place(i, i, i, i); };
	struct Root {
		Span coefficients;
		Node node;
		size_t length;
	};
	static size_t scratch_words(size_t count, size_t total) {
		return Arena::footprint(count + 1) + 8 * Arena::footprint(Layout::kWords * (Layout::length(Word(total)) + 1)) + 4096;
	}
	ProductTree(const TreeTransform& t, const Leaves& leaves, Span scratch, Keep keep = {})
		: t_(t), leaves_(leaves), keep_(keep), stack_(scratch), count_(leaves.count()), prefix_(stack_.take(count_ + 1)) {
		prefix_[0] = 0;
		for (size_t k = 0; k < count_; ++k) prefix_[k + 1] = prefix_[k] + leaves.degree(k);
	}
	Root root() {
		const size_t count = count_, length = Layout::length(degree(0, count));
		constexpr size_t w = Layout::kWords;
		Word* const c = stack_.take(w * (length + 1));
		if (count == 1 || degree(0, count) <= Layout::kBase) {
			std::fill_n(c, w * (length + 1), 0);
			return {{c, w * (length + 1)}, coefficients(0, count, c), length};
		}
		Word* const a = stack_.take(w * length);
		const size_t mid = split(0, count);
		Word *left_out = a, *right_out = c;
		if constexpr (kPlaces) left_out = keep_.place(0, mid, count, w * length), right_out = left_out + Arena::footprint(w * length);
		const Node left = build(0, mid, left_out, length), right = build(mid, count, right_out, length);
		Layout::product(t_, left_out, right_out, {a, w * length});
		if constexpr (requires { keep_.root(ConstSpan{}); }) keep_.root(ConstSpan(a, w * length));
		t_.inverse({a, w * length}, {c, w * length});
		const Node node{Layout::sum(left.degree, right.degree), Layout::lead_product(left.lead, right.lead)};
		const Value top = Layout::top(node.degree, node.lead, length);
		Layout::subtract(c, top);
		Layout::put(c + w * length, top);
		return {{c, w * (length + 1)}, node, length};
	}
	size_t split(size_t lo, size_t hi) const {
		const Word target = prefix_[lo] + (prefix_[hi] - prefix_[lo]) / 2;
		size_t mid = size_t(std::lower_bound(prefix_ + lo + 1, prefix_ + hi - 1, target) - prefix_);
		if (mid > lo + 1 && prefix_[mid] >= target && target - prefix_[mid - 1] < prefix_[mid] - target) --mid;
		return mid;
	}
	Word degree(size_t lo, size_t hi) const { return prefix_[hi] - prefix_[lo]; }
private:
	Node coefficients(size_t lo, size_t hi, Word* c) const {
		if (hi - lo == 1) return leaves_.load(lo, c);
		constexpr size_t w = Layout::kWords, kMax = std::max<size_t>(Layout::kBase, 1);
		struct Part {
			Word* c;
			size_t degree;
		};
		Part part[kMax];
		alignas(32) Word memory[2][w * 2 * kMax];
		size_t count = hi - lo;
		Word* to = memory[0];
		Value degree{};
		for (size_t k = 0; k < count; ++k) {
			const Value d = leaves_.load(lo + k, to).degree;
			degree = k ? Layout::sum(degree, d) : d;
			part[k] = {to, leaves_.degree(lo + k)};
			to += w * (part[k].degree + 1);
		}
		for (size_t level = 1; count > 1; ++level) {
			to = count == 2 ? c : memory[level & 1];
			size_t next = 0;
			for (size_t k = 0; k + 1 < count; k += 2) {
				const Part &a = part[k], &b = part[k + 1];
				Layout::multiply(a.c, a.degree, b.c, b.degree, to);
				part[next++] = {to, a.degree + b.degree};
				to += w * (a.degree + b.degree + 1);
			}
			if (count % 2) {
				const Part& a = part[count - 1];
				std::copy_n(a.c, w * (a.degree + 1), to);
				part[next++] = {to, a.degree};
			}
			count = next;
		}
		return {degree, Layout::lead(c, degree)};
	}
	Node build(size_t lo, size_t hi, Word* out, size_t out_length) {
		constexpr size_t w = Layout::kWords;
		if (hi - lo == 1 || degree(lo, hi) <= Layout::kBase) {
			const Node node = coefficients(lo, hi, out);
			t_.forward({out, w * (degree(lo, hi) + 1)}, 0, {out, w * out_length});
			if constexpr (!kPlaces) keep_(lo, hi, ConstSpan(out, w * out_length));
			return node;
		}
		const size_t length = Layout::length(degree(lo, hi)), words = w * length, mark = stack_.mark();
		Word *a, *b, *c;
		size_t mid;
		if constexpr (kPlaces) {
			mid = split(lo, hi);
			a = keep_.place(lo, mid, hi, words), b = a + Arena::footprint(words), c = stack_.take(words + w);
		} else {
			a = out_length >= 2 * length ? out + words : stack_.take(words);
			b = c = stack_.take(words + w);
			mid = split(lo, hi);
		}
		const Node left = build(lo, mid, a, length), right = build(mid, hi, b, length);
		const Node node{Layout::sum(left.degree, right.degree), Layout::lead_product(left.lead, right.lead)};
		Layout::product(t_, a, b, {out, words});
		if (out_length > length) {
			t_.inverse({out, words}, {c, words});
			const Value top = Layout::top(node.degree, node.lead, length);
			Layout::subtract(c, top);
			Layout::subtract(c, top);
			t_.forward_upper({c, words}, 0, {out + words, words});
			Layout::add(c, top);
			Layout::put(c + words, top);
			for (size_t m = 2 * length; m < out_length; m *= 2) t_.forward_upper({c, words + w}, 0, {out + w * m, w * m});
		}
		stack_.release(mark);
		if constexpr (!kPlaces) keep_(lo, hi, ConstSpan(out, w * out_length));
		return node;
	}
	const TreeTransform& t_;
	const Leaves& leaves_;
	Keep keep_;
	detail::Stack stack_;
	size_t count_;
	Word* prefix_;
};
inline void lane_columns(const Word* c, size_t count, const Word (&size)[8],
						Word* const (&out)[8]) {
	using namespace detail;
	const size_t longest = *std::max_element(size, size + 8);
	for (size_t j = 0; j < longest; j += 8) {
		Vec r[8];
		for (size_t i = 0; i < 8; ++i) r[i] = j + i < count ? load(c + 8 * (j + i)) : _mm256_setzero_si256();
		transpose8(r);
		for (int l = 0; l < 8; ++l)
			if (j < size[l]) store_unaligned(out[l] + j, r[l]);
	}
}
}
namespace evaluation_poly {
namespace detail {
template <size_t K, size_t C, size_t B>
[[gnu::always_inline]] inline void middle_lanes(const Word* w, const Word* q, Word* out) {
	static_assert(K <= 16);
	Vec wo[K + C], qo[K + 1];
#pragma GCC unroll 32
	for (size_t i = 0; i < K + C; ++i) wo[i] = _mm256_srli_epi64(load(w + 8 * i), 32);
#pragma GCC unroll 17
	for (size_t j = 0; j <= K; ++j) qo[j] = _mm256_srli_epi64(load(q + 8 * j), 32);
	for (size_t t0 = 0; t0 < C; t0 += B) {
		Vec even[B], odd[B];
#pragma GCC unroll 8
		for (size_t s = 0; s < B; ++s) even[s] = odd[s] = _mm256_setzero_si256();
#pragma GCC unroll 17
		for (size_t j = 0; j <= K; ++j) {
			const Vec y = load(q + 8 * j);
#pragma GCC unroll 8
			for (size_t s = 0; s < B && t0 + s < C; ++s) {
				even[s] = _mm256_add_epi64(even[s], _mm256_mul_epu32(load(w + 8 * (K + t0 + s - j)), y));
				odd[s] = _mm256_add_epi64(odd[s], _mm256_mul_epu32(wo[K + t0 + s - j], qo[j]));
			}
		}
#pragma GCC unroll 8
		for (size_t s = 0; s < B && t0 + s < C; ++s) store(out + 8 * (t0 + s), montgomery_sums(even[s], odd[s], K + 1));
	}
}
[[gnu::noinline]] inline void middle_lanes(const Word* w, size_t k, const Word* q, size_t count,
										Word* out) {
	for (size_t t = 0; t < count; ++t) {
		Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
		for (size_t j = 0; j <= k; ++j) {
			const Vec x = load(w + 8 * (k + t - j)), y = load(q + 8 * j);
			even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
		}
		store(out + 8 * t, montgomery_sums(even, odd, k + 1));
	}
}
struct PointSlots {
	const Word* points;
	size_t slots;
	size_t count() const { return slots; }
	Word degree(size_t) const { return 1; }
	LaneLayout::Node load(size_t k, Word* c) const {
		const Vec lead = to_montgomery(_mm256_sub_epi32(broadcast(kP), detail::load(points + 8 * k)));
		store(c, broadcast(kR));
		store(c + 8, lead);
		return {broadcast(1), lead};
	}
};
struct LaneProducts {
	const Word* c;
	size_t stride;
	Word lane_degree;
	size_t count() const { return 8; }
	Word degree(size_t) const { return lane_degree; }
	StandardLayout::Node load(size_t k, Word* out) const {
		std::copy_n(c + k * stride, lane_degree + 1, out);
		return {lane_degree, out[lane_degree]};
	}
};
class TransformStore {
public:
	struct Keeper {
		TransformStore* store;
		Word* place(size_t, size_t mid, size_t, size_t words) const { return store->place(mid, words); }
		void root(ConstSpan transform) const { store->root_ = transform; }
	};
	void reset(Span words, Span index) { words_ = words, used_ = 0, pair_ = index.data(); }
	Word* place(size_t mid, size_t words) {
		const size_t need = 2 * Arena::footprint(words);
		if (used_ + need > words_.size()) std::abort();
		pair_[mid] = Word(used_);
		used_ += need;
		return words_.data() + pair_[mid];
	}
	const Word* pair(size_t mid) const { return words_.data() + pair_[mid]; }
	ConstSpan root() const { return root_; }
private:
	ConstSpan root_;
	Span words_;
	size_t used_ = 0;
	Word* pair_ = nullptr;
};
template <class Layout, class Tree>
size_t kept_words(const Tree& tree, size_t lo, size_t hi) {
	if (hi - lo == 1 || tree.degree(lo, hi) <= Layout::kBase) return 0;
	const size_t mid = tree.split(lo, hi), words = Layout::kWords * Layout::length(tree.degree(lo, hi));
	return 2 * Arena::footprint(words) + kept_words<Layout>(tree, lo, mid) + kept_words<Layout>(tree, mid, hi);
}
template <size_t D>
void product_level(const Word* children, Word* out, size_t nodes) {
	for (size_t i = 0; i < nodes; ++i)
		multiply_lanes(children + 8 * (D + 1) * (2 * i), D, children + 8 * (D + 1) * (2 * i + 1), D, out + 8 * (2 * D + 1) * i);
}
constexpr size_t middle_block(size_t d) { return d >= 16 ? 0 : d == 8 ? 3 : d == 4 ? 4 : 2; }
template <size_t D>
void middle_level(const Word* parents, const Word* products, Word* out, size_t nodes) {
	for (size_t i = 0; i < nodes; ++i) {
		const Word *w = parents + 16 * D * (i / 2), *q = products + 8 * (D + 1) * (i ^ 1);
		if constexpr (middle_block(D) == 0) middle_lanes(w, D, q, D, out + 8 * D * i);
		else middle_lanes<D, D, middle_block(D)>(w, q, out + 8 * D * i);
	}
}
template <class F>
void with_degree(size_t d, F&& f) {
	switch (d) {
		case 1: return f.template operator()<1>();
		case 2: return f.template operator()<2>();
		case 4: return f.template operator()<4>();
		case 8: return f.template operator()<8>();
		case 16: return f.template operator()<16>();
		default: std::abort();
	}
}
class BaseTree {
protected:
	BaseTree(const PointSlots& slots, Word* values) : slots_(slots), values_(values) {}
	Word* build_levels(size_t lo, size_t count, const Word** products) {
		Word* at = memory_;
		for (size_t k = 0; k < count; ++k) slots_.load(lo + k, at + 16 * k);
		products[0] = at;
		at += 16 * count;
		for (size_t d = 1, j = 1; 2 * d < count; d *= 2, ++j) {
			with_degree(d, [&]<size_t D>() {
				if constexpr (D <= LaneLayout::kBase / 4) product_level<D>(products[j - 1], at, count / (2 * D));
			});
			products[j] = at;
			at += 8 * (2 * d + 1) * (count / (2 * d));
		}
		return at;
	}
	void descend_levels(size_t lo, size_t count, const Word* w, const Word* const* products, Word* const* windows) {
		const Word* parents = w;
		for (size_t d = count / 2, j = std::countr_zero(count) - 1;; d /= 2, --j) {
			Word* const out = d == 1 ? values_ + 8 * (lo - lo_) : windows[j & 1];
			with_degree(d, [&]<size_t D>() { middle_level<D>(parents, products[j], out, count / D); });
			if (d == 1) return;
			parents = out;
		}
	}
	const Word* build(size_t v, size_t lo, size_t hi) {
		Word* const c = free_;
		if (hi - lo == 1) {
			slots_.load(lo, c);
			free_ += 16;
			return c;
		}
		const size_t mid = lo + (hi - lo) / 2;
		product_[2 * v] = build(2 * v, lo, mid);
		product_[2 * v + 1] = build(2 * v + 1, mid, hi);
		if (v == 1) return nullptr;
		Word* const out = free_;
		multiply_lanes_any(product_[2 * v], mid - lo, product_[2 * v + 1], hi - mid, out);
		free_ += 8 * (hi - lo + 1);
		return out;
	}
	void descend(size_t v, size_t lo, size_t hi, const Word* w) {
		if (hi - lo == 1) return store(values_ + 8 * (lo - lo_), load(w));
		const size_t mid = lo + (hi - lo) / 2;
		Word* const left = free_;
		Word* const right = left + 8 * (mid - lo);
		free_ = right + 8 * (hi - mid);
		middle_lanes(w, hi - mid, product_[2 * v + 1], mid - lo, left);
		middle_lanes(w, mid - lo, product_[2 * v], hi - mid, right);
		descend(2 * v, lo, mid, left);
		descend(2 * v + 1, mid, hi, right);
		free_ = left;
	}
	alignas(32) Word memory_[8 * 320];
	const Word* product_[64];
	Word* free_ = memory_;
	const PointSlots& slots_;
	Word* values_;
	size_t lo_ = 0;
};

}
class PointTree {
public:
	static size_t words(size_t m) {
		const size_t m8 = (m + 7) / 8 * 8, count = m8 / 8, levels = std::bit_width(count) + 1;
		const size_t top_length = StandardLayout::length(Word(m8));
		const size_t kept_lanes = levels * (32 * count + 64 * (count / 32 + 1));
		const size_t kept_top = 14 * Arena::footprint(top_length);
		return TreeTransform::words(log_length(m8)) + Arena::footprint(m8) + Arena::footprint(LaneTree::scratch_words(count, count)) +
			Arena::footprint(kept_lanes) + Arena::footprint(count) +
			Arena::footprint(8 * ((count + 8) / 8 * 8)) + Arena::footprint(TopTree::scratch_words(8, m8)) +
			Arena::footprint(kept_top) + Arena::footprint(8);
	}
	static size_t descend_words(size_t m) {
		const size_t m8 = (m + 7) / 8 * 8;
		const size_t lane_length = LaneLayout::length(Word(m8 / 8)), top_length = StandardLayout::length(Word(m8));
		return 2 * Arena::footprint(8 * std::max<size_t>(8, lane_length)) + 5 * Arena::footprint(top_length) +
			4 * Arena::footprint(8 * lane_length) + 4096;
	}
	PointTree(Arena& arena, ConstSpan points)
		: m8_((points.size() + 7) / 8 * 8), count_(m8_ / 8), t_(arena, log_length(m8_)) {
		using namespace detail;
		Word* const padded = arena.take(m8_).data();
		std::copy(points.begin(), points.end(), padded);
		slots_ = {padded, count_};
		lanes_.emplace(t_, slots_, arena.take(LaneTree::scratch_words(count_, count_)), TransformStore::Keeper{&lane_store_});
		lane_store_.reset(arena.take(kept_words<LaneLayout>(*lanes_, 0, count_)), arena.take(count_));
		const LaneTree::Root lane_root = lanes_->root();
		const size_t stride = (count_ + 8) / 8 * 8;
		Word* const columns = arena.take(8 * stride).data();
		const Word size = Word(count_ + 1);
		const Word sizes[8] = {size, size, size, size, size, size, size, size};
		Word* const out[8] = {columns, columns + stride, columns + 2 * stride, columns + 3 * stride,
									columns + 4 * stride, columns + 5 * stride, columns + 6 * stride, columns + 7 * stride};
		lane_columns(lane_root.coefficients.data(), lane_root.length + 1, sizes, out);
		lane_products_ = {columns, stride, Word(count_)};
		top_.emplace(t_, lane_products_, arena.take(TopTree::scratch_words(8, m8_)), TransformStore::Keeper{&top_store_});
		top_store_.reset(arena.take(kept_words<StandardLayout>(*top_, 0, 8)), arena.take(8));
		product_ = top_->root().coefficients.first(m8_ + 1);
	}
	PointTree(const PointTree&) = delete;
	PointTree& operator=(const PointTree&) = delete;
	size_t size() const { return m8_; }
	ConstSpan product() const { return product_; }
	ConstSpan product_transform() const { return top_store_.root(); }

protected:
	using LaneTree = ProductTree<LaneLayout, detail::PointSlots, detail::TransformStore::Keeper>;
	using TopTree = ProductTree<StandardLayout, detail::LaneProducts, detail::TransformStore::Keeper>;
	static int log_length(size_t m8) {
		const size_t lanes = 8 * LaneLayout::length(Word(m8 / 8)), top = StandardLayout::length(Word(m8));
		return std::countr_zero(std::max(lanes, top));
	}
	void halve(Word* p, size_t words) const {
		using namespace detail;
		const size_t h = words / 2;
		t_.inverse_upper({p + h, h}, {p + h, h}, kP - 1);
		t_.forward({p + h, h}, 0, {p + h, h});
		for (size_t i = 0; i < h; i += 8) store(p + i, add_mod(load(p + i), load(p + h + i)));
	}
	void coefficients(Word* p, size_t words, Word u) const {
		using namespace detail;
		if (words == 8) return t_.inverse({p, 8}, {p, 8}, u);
		const size_t h = words / 2;
		const Word c = half(u);
		t_.inverse({p, h}, {p, h}, c);
		t_.inverse_upper({p + h, h}, {p + h, h}, kP - c);
		for (size_t i = 0; i < h; i += 8) {
			const Vec a = load(p + i), b = load(p + h + i);
			store(p + i, subtract_mod(a, b)), store(p + h + i, add_mod(a, b));
		}
	}
	struct LaneState {
		Word* state;
		Word u;
	};
	LaneState lane_state(ConstSpan w, Word c, detail::Stack& stack) {
		using namespace detail;
		const size_t lane_length = LaneLayout::length(Word(count_)), stride = std::max<size_t>(8, lane_length);
		const bool transform = count_ > LaneLayout::kBase;
		Word* const lanes = stack.take(8 * stride);
		const size_t mark = stack.mark();
		Word* const columns = stack.take(8 * stride);
		if (!transform) std::fill_n(columns, 8 * stride, 0);
		const size_t length = StandardLayout::length(Word(m8_));
		Word* const root = stack.take(length);
		t_.forward(w, length - m8_, {root, length});
		Word u = 1;
		descend_top(0, 8, root, c, stack, columns, lane_length, stride, transform ? &u : nullptr);
		if (transform) {
			t_.standard_to_lanes(columns, stride, lane_length, lanes);
		} else {
			for (size_t j = 0; j < stride; j += 8) {
				Vec r[8];
				for (size_t l = 0; l < 8; ++l) r[l] = load(columns + l * stride + j);
				transpose8(r);
				for (size_t i = 0; i < 8; ++i) store(lanes + 8 * (j + i), r[i]);
			}
		}
		stack.release(mark);
		return {lanes, u};
	}
	template <class Layout, class Tree, class Leaf>
	void descend_tree(const Tree& tree, const detail::TransformStore& store, size_t lo, size_t hi,
			const Word* s, Word u, detail::Stack& stack, const Leaf& leaf) {
		const size_t words = Layout::kWords * Layout::length(tree.degree(lo, hi)), mid = tree.split(lo, hi);
		const size_t sides[2][2] = {{lo, mid}, {mid, hi}};
		for (int side = 0; side < 2; ++side) {
			const size_t xlo = sides[side][0], xhi = sides[side][1], degree = tree.degree(xlo, xhi);
			const size_t inner = Layout::kWords * Layout::length(Word(degree)), mark = stack.mark();
			Word* const p = stack.take(words);
			Layout::product(t_, s, store.pair(mid) + (1 - side) * Arena::footprint(words), {p, words});
			if (leaf(xlo, xhi, degree, p, words, u)) {
			} else if (2 * inner == words) {
				halve(p, words);
				descend_tree<Layout>(tree, store, xlo, xhi, p, half(u), stack, leaf);
			} else {
				coefficients(p, words, u);
				Word* const q = stack.take(inner);
				t_.forward({p + words - inner, inner}, 0, {q, inner});
				descend_tree<Layout>(tree, store, xlo, xhi, q, 1, stack, leaf);
			}
			stack.release(mark);
		}
	}
	void descend_top(size_t lo, size_t hi, const Word* s, Word u, detail::Stack& stack,
					Word* columns, size_t lane_length, size_t stride, Word* leaf_u) {
		descend_tree<StandardLayout>(*top_, top_store_, lo, hi, s, u, stack,
			[&](size_t xlo, size_t xhi, size_t, Word* p, size_t words, Word scale) {
				if (xhi - xlo != 1) return false;
				if (leaf_u) {
					halve(p, words);
					std::copy_n(p, lane_length, columns + xlo * stride);
					*leaf_u = half(scale);
				} else {
					coefficients(p, words, scale);
					std::copy_n(p + words - lane_length, lane_length, columns + xlo * stride);
				}
				return true;
			});
	}

	static Word half(Word u) { return blocked_ntt::detail::multiply_mod(u, (detail::kP + 1) / 2); }
	size_t m8_, count_;
	TreeTransform t_;
	detail::PointSlots slots_{};
	detail::LaneProducts lane_products_{};
	detail::TransformStore lane_store_, top_store_;
	std::optional<LaneTree> lanes_;
	std::optional<TopTree> top_;
	ConstSpan product_;
};
namespace detail {
struct Division {
	size_t big, k, length;
	Division(size_t n, size_t m8)
		: big(std::max(n, m8)), k((big + 1) / 2), length(std::max<size_t>(64, std::bit_ceil(std::max(2 * k - 1, big)))) {}
	int log() const { return std::max(inverse_log(k), std::countr_zero(length)); }
	size_t scratch_words() const {
		return Arena::footprint(big) + Arena::footprint(k) + Arena::footprint(inverse_scratch(k)) + 3 * Arena::footprint(length);
	}
};
inline void divide(const Transform& t, const Division& s, ConstSpan f, ConstSpan q,
				ConstSpan qt, Span d, Span scratch) {
	const size_t big = s.big, k = s.k, rest = big - k, n = s.length;
	Stack stack(scratch);
	const Span g{stack.take(big), big}, h{stack.take(k), k};
	std::fill_n(g.begin(), big - f.size(), 0);
	std::reverse_copy(f.begin(), f.end(), g.begin() + std::ptrdiff_t(big - f.size()));
	inverse(t, q, h, {stack.take(inverse_scratch(k)), inverse_scratch(k)});
	const Span a{stack.take(n), n}, b{stack.take(n), n}, th{stack.take(n), n};
	const auto lower = [n](size_t size) { return size <= n / 2 ? Half::kLower : Half::kBoth; };
	t.forward(g.first(k), 0, a);
	t.forward(h, 0, th);
	t.inverse_product(a, th, a, lower(k));
	std::copy_n(a.begin(), k, d.begin());
	t.forward(d.first(k), 0, a);
	if (qt.size() != n) {
		const size_t terms = std::min(q.size(), n);
		std::copy_n(q.begin(), terms, b.begin());
		if (q.size() > n) b[0] = Word((std::uint64_t(b[0]) + q[n]) % kP);
		t.forward(b.first(terms), 0, b);
		qt = b;
	}
	t.inverse_product(qt, a, a, k >= n / 2 ? Half::kUpper : Half::kBoth);
	for (size_t i = 0; i < rest; ++i) b[i] = g[k + i] >= a[k + i] ? g[k + i] - a[k + i] : g[k + i] + kP - a[k + i];
	t.cyclic_product(b.first(rest), 0, a, th, lower(rest));
	std::copy_n(a.begin(), rest, d.begin() + std::ptrdiff_t(k));
}
}
inline void import_words(std::span<const mint> in, Span out) {
	using namespace detail;
	size_t i = 0;
	for (; i + 8 <= in.size(); i += 8) {
		const Vec x = load_unaligned(reinterpret_cast<const Word *>(in.data() + i));
		store(out.data() + i, canonical(montgomery(x, broadcast(1))));
	}
	for (; i < in.size(); ++i) out[i] = in[i].val();
}
inline void import_montgomery(std::span<const mint> in, Span out) {
	for (size_t i = 0; i < in.size(); ++i) {
		const unsigned x = in[i].v;
		out[i] = std::min(x, x - mint::mod());
	}
}
inline std::vector<mint> export_words(ConstSpan in) {
	std::vector<mint> out;
	append_fps_words<true>(out, in.data(), in.size());
	return out;
}

}
}

#pragma GCC pop_options
#endif
#endif
