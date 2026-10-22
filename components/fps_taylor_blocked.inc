#ifndef CP_COMPONENTS_FPS_TAYLOR_BLOCKED_HPP
#define CP_COMPONENTS_FPS_TAYLOR_BLOCKED_HPP

#include "fps_convolution.hpp"
#include <array>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace taylor_factorials {
inline constexpr std::uint32_t kStep = 1024;
template <int Block>
inline constexpr auto factorial_block = [] {
	std::array<std::uint32_t, 16> result{};
	std::uint64_t value = 1;
	if constexpr (Block > 0) value = factorial_block<Block - 1>.back();
	for (int i = 0; i < 16; ++i) {
		for (std::uint32_t j = 1; j <= kStep; ++j) value = value * ((Block * 16 + i) * kStep + j) % 998244353;
		result[i] = std::uint32_t(value);
	}
	return result;
}();
inline constexpr auto kTable = []<std::size_t... I>(std::index_sequence<I...>) {
	return std::array<std::uint32_t, 513>{1, factorial_block<I / 16>[I % 16]...};
}(std::make_index_sequence<512>{});
}
namespace taylor_detail {
namespace ntt = multiply_detail::ntt;
using ntt::detail::add;
using ntt::detail::broadcast;
using ntt::detail::diff;
using ntt::detail::kP;
using ntt::detail::kR;
using ntt::detail::multiply_mod;
using ntt::detail::power;
using ntt::detail::reduce;
using ntt::detail::slot;
using multiply_detail::Subtrees;
using ntt::detail::Vec;
using blocked_poly::detail::load;
using blocked_poly::detail::low;
using blocked_poly::detail::store;
inline std::uint32_t inverse(std::uint32_t x) { return power(x, kP - 2); }
inline std::uint32_t mont(std::uint32_t x) { return multiply_mod(x, kR); }
inline Vec odd_lanes(Vec x) { return _mm256_srli_epi64(x, 32); }
[[gnu::always_inline]] inline Vec montgomery(Vec a, Vec a_odd, Vec b, Vec b_odd) {
	const Vec ni = broadcast(ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b), odd = _mm256_mul_epu32(a_odd, b_odd);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}
[[gnu::always_inline]] inline void transpose(Vec (&r)[8]) {
	Vec t[8], u[8];
#pragma GCC unroll 4
	for (int i = 0; i < 8; i += 2) {
		t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]);
		t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
	}
#pragma GCC unroll 2
	for (int i = 0; i < 8; i += 4) {
		u[i] = _mm256_unpacklo_epi64(t[i], t[i + 2]);
		u[i + 1] = _mm256_unpackhi_epi64(t[i], t[i + 2]);
		u[i + 2] = _mm256_unpacklo_epi64(t[i + 1], t[i + 3]);
		u[i + 3] = _mm256_unpackhi_epi64(t[i + 1], t[i + 3]);
	}
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i) {
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20);
		r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
	}
}
constexpr int kVectors = 4, kLanes = 8 * kVectors;
template <class F>
void fill(Vec (&x)[kVectors], F f) {
	alignas(32) std::uint32_t values[kLanes];
	for (int s = 0; s < kLanes; ++s) values[s] = f(s);
	for (int v = 0; v < kVectors; ++v) x[v] = load(values + 8 * v);
}
using Block = Vec[kVectors][8];
struct Chain {
	Vec x[kVectors], base[kVectors], base_odd[kVectors], u, step;
	template <class X, class Base>
	Chain(X initial, Base base_of, std::uint32_t step_value) : u(_mm256_setzero_si256()), step(broadcast(step_value)) {
		fill(x, initial);
		fill(base, base_of);
		for (int v = 0; v < kVectors; ++v) base_odd[v] = odd_lanes(base[v]);
	}
	[[gnu::always_inline]] void advance(Block& block, int t) {
#pragma GCC unroll 4
		for (int v = 0; v < kVectors; ++v) {
			block[v][t] = x[v];
			x[v] = montgomery(x[v], odd_lanes(x[v]), add(base[v], u), add(base_odd[v], u));
		}
		u = reduce(add(u, step), kP);
	}
};
class TaylorShift {
public:
	explicit TaylorShift(std::size_t n) : n_(n) {
		lg_ = std::max(10, int(std::bit_width(2 * n - 1)));
		lg_ += lg_ % 2;
		chunk_ = ((n + kLanes - 1) / kLanes + 15) / 16 * 16;
		if (chunk_ / 16 % 2 == 0) chunk_ += 16;
		const std::size_t len = length(), tables = ntt::detail::table_words(lg_);
		const std::size_t words = 2 * len + 3 * kPadding + 2 * tables;
		storage_ = allocate_fps_words(words);
		if (!storage_.get_deleter().region) std::fill_n(storage_.get(), words, 0u);
		b_ = storage_.get() + kPadding;
		a_ = b_ + len + kPadding;
		roots_ = a_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
	}
	std::uint32_t* coefficients() { return a_; }
	const std::uint32_t* shift(std::uint32_t c) {
		using namespace ntt::detail;
		const std::size_t len = length(), q = len / 64;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		lane_factorials();
		weights(c);
		auto* av = reinterpret_cast<Vec*>(a_);
		auto* bv = reinterpret_cast<Vec*>(b_);
		multiply_detail::forward_radix8(av, q, roots_);
		multiply_detail::forward_radix8(bv, q, roots_);
		for (std::size_t t = 0; t < 8; ++t) {
			std::uint32_t& x = b_[8 * q * t];
			x = (x % kP + (t < 4 ? 1 : kP - 1)) % kP;
		}
		const Subtrees subtrees(roots_, inverse_roots_);
		for (std::size_t t = 0; t < 4; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse_identity(av, q, inverse_roots_);
		for (std::size_t t = 4; t < 8; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse(av + 4 * q, q, inverse_roots_ + slot(1), inverse_roots_ + slot(2));
		output();
		return b_;
	}
private:
	static constexpr std::size_t kPadding = 1040;
	std::size_t length() const { return std::size_t(1) << lg_; }
	std::size_t start(int s) const { return s * chunk_; }
	void lane_factorials() {
		using taylor_factorials::kStep;
		const auto below = [this](int s) { return std::uint32_t(start(s + 1) / kStep * kStep); };
		Vec x[kVectors], m[kVectors], count[kVectors];
		fill(x, [&](int s) { return mont(taylor_factorials::kTable[below(s) / kStep]); });
		fill(m, [&](int s) { return mont(below(s) + 1); });
		fill(count, [&](int s) { return std::uint32_t(start(s + 1) - below(s)); });
		const Vec unit = broadcast(kR);
		for (std::uint32_t j = 0; j < kStep - 1; ++j) {
			const Vec step = broadcast(j);
#pragma GCC unroll 4
			for (int v = 0; v < kVectors; ++v) {
				const Vec factor = _mm256_blendv_epi8(unit, m[v], _mm256_cmpgt_epi32(count[v], step));
				x[v] = montgomery(x[v], odd_lanes(x[v]), factor, odd_lanes(factor));
				m[v] = reduce(add(m[v], unit), kP);
			}
		}
		alignas(32) std::uint32_t lane_x[kLanes];
		for (int v = 0; v < kVectors; ++v) store(lane_x + 8 * v, x[v]);
		const std::uint32_t unit_inverse = inverse(kR);
		factorial_[0] = 1;
		for (int s = 0; s < kLanes; ++s) factorial_[s + 1] = multiply_mod(lane_x[s] % kP, unit_inverse);
		std::uint32_t f = 1;
		for (int s = 0; s <= kLanes; ++s) inverse_start_[s] = f, f = multiply_mod(f, factorial_[s]);
		f = inverse(f);
		for (int s = kLanes; s >= 0; --s)
			inverse_start_[s] = multiply_mod(inverse_start_[s], f), f = multiply_mod(f, factorial_[s]);
	}
	void weights(std::uint32_t c) {
		const std::uint32_t c_chunk = power(c, std::uint32_t(chunk_)), c_step = mont(inverse(c));
		std::array<std::uint32_t, kLanes> c_power;
		for (int s = 0; s < kLanes; ++s) c_power[s] = s ? multiply_mod(c_power[s - 1], c_chunk) : c_chunk;
		Chain input([this](int s) { return mont(factorial_[s]); },
					[this](int s) { return mont(std::uint32_t(start(s) + 1)); }, kR);
		Chain kernel([&](int s) { return multiply_mod(c_power[s], inverse_start_[s + 1]); },
					[&](int s) { return multiply_mod(std::uint32_t(start(s + 1)), c_step); }, kP - c_step);
		const std::size_t stride = chunk_;
		Block forward[2], backward[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) input.advance(forward[0], t), kernel.advance(backward[0], t);
		for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			std::uint32_t* const a = a_ + j;
			std::uint32_t* const e = b_ + length() / 2 - stride + j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				input.advance(forward[cur ^ 1], t);
				kernel.advance(backward[cur ^ 1], t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(forward[cur][v]), transpose(backward[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const std::size_t at = (8 * v + l) * stride;
					const Vec f = load(a + at), x = forward[cur][v][l];
					store(a + at, reduce(montgomery(f, odd_lanes(f), x, odd_lanes(x)), kP));
					store(e - at, reduce(backward[cur][v][l], kP));
				}
			}
		}
	}
	void output() {
		const std::uint32_t scale = multiply_mod(mont(inverse(std::uint32_t(length() / 8))), kR);
		Chain chain([&](int s) { return multiply_mod(multiply_mod(inverse_start_[s + 1], std::uint32_t(start(s + 1))), scale); },
					[this](int s) { return mont(std::uint32_t(start(s + 1) - 1)); }, kP - kR);
		const std::size_t stride = chunk_, half = length() / 2;
		Block block[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) chain.advance(block[0], 7 - t);
		for (std::size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			const std::uint32_t* const u = a_ + stride - 8 - j;
			std::uint32_t* const out = b_ + stride - 8 - j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				chain.advance(block[cur ^ 1], 7 - t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(block[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const std::size_t at = (8 * v + l) * stride;
					const Vec g = low(diff(load(u + at), load(u + at + half))), z = block[cur][v][l];
					store(out + at, reduce(montgomery(g, odd_lanes(g), z, odd_lanes(z)), kP));
				}
			}
		}
	}
	std::size_t n_, chunk_;
	int lg_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
	FpsWords storage_;
	std::array<std::uint32_t, kLanes + 1> factorial_, inverse_start_;
};
inline void shift(std::vector<Mint> &f, Mint c) {
	TaylorShift t(f.size());
	std::size_t i = 0;
	for (; i + 8 <= f.size(); i += 8) {
		const auto x = blocked_poly::detail::load_unaligned(reinterpret_cast<const unsigned *>(f.data() + i));
		store(t.coefficients() + i, reduce(x, kP));
	}
	for (; i < f.size(); ++i) t.coefficients()[i] = std::min(f[i].v, f[i].v - kP);
	const auto *out = t.shift(c.val());
	i = 0;
	for (; i + 8 <= f.size(); i += 8)
		blocked_poly::detail::store_unaligned(reinterpret_cast<unsigned *>(f.data() + i), load(out + i));
	for (; i < f.size(); ++i) f[i] = Mint::word(out[i]);
}
}
}

#pragma GCC pop_options
#endif
#endif
