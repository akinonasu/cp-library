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
	std::size_t n_, m_, len_;
	blocked_poly::Arena arena_;
	blocked_poly::Transform transform_;
	std::uint32_t *a_, *b_;
	static int log_length(std::size_t n, std::size_t m) {
		if (n == 0 || m == 0 || n > (1u << 23) || m > (1u << 23) || n + m - 1 > (1u << 23)) std::abort();
		return std::max(6, int(std::bit_width(n + m - 2)));
	}
public:
	Convolution(std::size_t n, std::size_t m) : Convolution(n, m, nullptr) {}
	Convolution(std::size_t n, std::size_t m, std::uint32_t* borrowed)
		: n_(n), m_(m), len_(std::size_t(1) << log_length(n, m)),
		  arena_(blocked_poly::Transform::words(log_length(n, m)) + (borrowed ? 1 : 2) * blocked_poly::Arena::footprint(len_)),
		  transform_(arena_, log_length(n, m)), a_(borrowed ? borrowed : arena_.take(len_).data()), b_(arena_.take(len_).data()) {
		if (n == 0 || m == 0 || log_length(n, m) > 23) std::abort();
	}
	static bool fits(std::size_t n, std::size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= std::size_t(1) << lg;
	}
	std::uint32_t* a() { return a_; }
	std::uint32_t* b() { return b_; }
	const std::uint32_t* multiply() {
		transform_.forward(std::span<const std::uint32_t>(b_, m_), 0, std::span<std::uint32_t>(b_, len_));
		transform_.cyclic_product(std::span<const std::uint32_t>(a_, n_), 0,
			std::span<std::uint32_t>(a_, len_), std::span<const std::uint32_t>(b_, len_), blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
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
	const std::uint32_t *roots_, *inverse_;
public:
	Subtrees(const std::uint32_t* r, const std::uint32_t* ir) : roots_(r), inverse_(ir) {}
	void visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
		using namespace blocked_poly::detail;
		Recursion<ForwardBottom>(roots_, inverse_, ForwardBottom{roots_}).visit(reinterpret_cast<std::uint32_t*>(b), nv, k);
		ProductBottom bottom{roots_, inverse_, reinterpret_cast<std::uint32_t*>(b), k * nv};
		Recursion<ProductBottom>(roots_, inverse_, bottom).visit(reinterpret_cast<std::uint32_t*>(a), nv, k);
	}
};
inline void forward_radix8(Vec* f, std::size_t q, const std::uint32_t* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (std::size_t j = 0; j < q; ++j) {
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
inline void copy_input(const std::vector<Mint> &in, std::uint32_t *out, std::size_t len) {
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - Mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<Mint>& in, Mint* out) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = Mint::word(std::min(in[i].v, in[i].v - Mint::mod()));
}

template <class Plan>
inline std::vector<Mint> run_borrowed_as(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	const std::size_t len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr std::size_t alignment = 32;
	const std::size_t words = len + 16 + alignment / sizeof(Mint) - 1, size = a.size() + b.size() - 1;
	std::vector<Mint> result;
	prepare_fps_output(result, words);
	result.assign(words, Mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const std::size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(Mint);
	Mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<std::uint32_t*>(to));
	copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply();
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(Mint));
	result.resize(size);
	return result;
}

inline std::vector<Mint> run_borrowed(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<Mint> run(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<Mint> result;
	const std::size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}


inline std::vector<Mint> multiply(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}
}
#pragma GCC pop_options
#endif
#endif
