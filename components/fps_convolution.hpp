#ifndef CP_COMPONENTS_FPS_CONVOLUTION_HPP
#define CP_COMPONENTS_FPS_CONVOLUTION_HPP
#include "fps_blocked.hpp"
#include "fps_output.hpp"
#include "fps_convolution_unbalanced.hpp"
#include "fps_convolution_constant.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail {
namespace ntt {
namespace kernels = blocked_ntt::kernels;
namespace detail = blocked_ntt::detail;
class Convolution {
	size_t n_, m_, len_;
	blocked_poly::Arena arena_;
	blocked_poly::Transform transform_;
	Word *a_, *b_;
	static int log_length(size_t n, size_t m) {
		if (n == 0 || m == 0 || n > (1u << 23) || m > (1u << 23) || n + m - 1 > (1u << 23)) std::abort();
		return std::max(6, int(std::bit_width(n + m - 2)));
	}
public:
	Convolution(size_t n, size_t m) : Convolution(n, m, nullptr) {}
	Convolution(size_t n, size_t m, Word* borrowed)
		: n_(n), m_(m), len_(size_t(1) << log_length(n, m)),
		  arena_(blocked_poly::Transform::words(log_length(n, m)) + (borrowed ? 1 : 2) * blocked_poly::Arena::footprint(len_)),
		  transform_(arena_, log_length(n, m)), a_(borrowed ? borrowed : arena_.take(len_).data()), b_(arena_.take(len_).data()) {
		if (n == 0 || m == 0 || log_length(n, m) > 23) std::abort();
	}
	static bool fits(size_t n, size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= size_t(1) << lg;
	}
	Word* a() { return a_; }
	Word* b() { return b_; }
	const Word* multiply(bool square = false) {
		transform_.forward(ConstWords(b_, m_), 0, Words(b_, len_));
		if (square) transform_.inverse_product(ConstWords(b_, len_), ConstWords(b_, len_), Words(a_, len_),
			blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		else transform_.cyclic_product(ConstWords(a_, n_), 0,
			Words(a_, len_), ConstWords(b_, len_), blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		return a_;
	}
};
}
using Product = ntt::Convolution;
using ntt::detail::Vec;
using ntt::detail::Factor;
using ntt::detail::add;
using ntt::detail::diff;
using ntt::detail::kP;
using ntt::detail::reduce;
inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }
inline Vec diff_canonical(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y); }
class Subtrees {
	const Word *roots_, *inverse_;
public:
	Subtrees(const Word* r, const Word* ir) : roots_(r), inverse_(ir) {}
	void visit(Vec* a, Vec* b, size_t nv, size_t k) const {
		using namespace blocked_poly::detail;
		Recursion<ForwardBottom>(roots_, inverse_, ForwardBottom{roots_}).visit(reinterpret_cast<Word*>(b), nv, k);
		ProductBottom bottom{roots_, inverse_, reinterpret_cast<Word*>(b), k * nv};
		Recursion<ProductBottom>(roots_, inverse_, bottom).visit(reinterpret_cast<Word*>(a), nv, k);
	}
};
inline void forward_radix8(Vec* f, size_t q, const Word* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (size_t j = 0; j < q; ++j) {
		const Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];

		const Vec g0 = add(f0, f2), g1 = add(f1, f3);
		const Vec h0 = diff_canonical(f0, f2), ih1 = times(diff_canonical(f1, f3), i);
		f[j] = add(g0, g1), f[j + q] = diff(g0, g1);
		f[j + 2 * q] = add(h0, ih1), f[j + 3 * q] = diff(h0, ih1);

		const Vec if2 = times(f2, i), if3 = times(f3, i);
		const Vec u0 = reduce(add(f0, if2), 2 * kP), v0 = reduce(diff(f0, if2), 2 * kP);
		const Vec yu1 = times(add(f1, if3), y), zv1 = times(diff(f1, if3), z);
		f[j + 4 * q] = add(u0, yu1), f[j + 5 * q] = diff(u0, yu1);
		f[j + 6 * q] = add(v0, zv1), f[j + 7 * q] = diff(v0, zv1);
	}
}
inline void copy_input(const std::vector<mint> &in, Word *out, size_t len) {
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<mint>& in, mint* out) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = mint::word(std::min(in[i].v, in[i].v - mint::mod()));
}

template <class Plan>
inline std::vector<mint> run_borrowed_as(const std::vector<mint>& a, const std::vector<mint>& b) {
	const size_t len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr size_t alignment = 32;
	const size_t words = len + 16 + alignment / sizeof(mint) - 1, size = a.size() + b.size() - 1;
	std::vector<mint> result;
	prepare_fps_output(result, words);
	result.assign(words, mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(mint);
	mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<Word*>(to));
	if (&a != &b) copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply(&a == &b);
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(mint));
	result.resize(size);
	return result;
}

inline std::vector<mint> run_borrowed(const std::vector<mint>& a, const std::vector<mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<mint> run(const std::vector<mint> &a, const std::vector<mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<mint> result;
	const size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}


inline std::vector<mint> multiply(const std::vector<mint>& a, const std::vector<mint>& b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}
}
#pragma GCC pop_options
#endif
#endif
