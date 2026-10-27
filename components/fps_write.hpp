#ifndef CP_COMPONENTS_FPS_WRITE_HPP
#define CP_COMPONENTS_FPS_WRITE_HPP

#include "fast_io_output.hpp"
#include "../math/mint.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_io_fields {

namespace detail {

struct alignas(32) Lanes {
	std::uint32_t lane[8];
};

constexpr Lanes all(std::uint32_t x) { return {{x, x, x, x, x, x, x, x}}; }
constexpr Lanes all16(std::uint32_t x) { return all(x * 0x10001); }

constexpr int Z = 0x80;

constexpr Lanes shuffle(const int (&b)[16]) {
	Lanes t{};
	for (int i = 0; i < 32; ++i) t.lane[i / 4] |= std::uint32_t(b[i % 16] & 0xFF) << 8 * (i % 4);
	return t;
}

struct Constants {
	Lanes div10 = all(429496730);
	Lanes div10e5 = all(1407374884);
	Lanes pack = all(1 - 10000 * 65536u);
	Lanes ten = all(10);
	Lanes tail = all(' ' << 8 | '0');
	Lanes by100 = all16(5243), hundred = all16(100);
	Lanes by10 = all16(6554);
	Lanes tens_out = all16(2559);
	Lanes ones = all(~0u), zero = all(0x30303030), space = all(0x20202020);

	Lanes chunk0[2] = {shuffle({0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, Z, Z, Z, Z, Z, Z})};
	Lanes chunk1[3] = {shuffle({14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, 4, 5, Z, Z, Z, Z, Z, Z, Z, Z, 8, 9, Z, Z}),
					shuffle({Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9})};
	Lanes chunk2[3] = {shuffle({10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, 12, 13, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7})};
	Lanes chunk3[3] = {shuffle({0, 1, Z, Z, Z, Z, Z, Z, Z, Z, 4, 5, Z, Z, Z, Z}),
					shuffle({Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3})};
	Lanes chunk4[2] = {shuffle({4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z}),
					shuffle({Z, Z, Z, Z, 8, 9, Z, Z, Z, Z, Z, Z, Z, Z, 12, 13})};
};

inline constexpr Constants kConstants{};

[[gnu::always_inline]] inline __m256i load(const Lanes& l) {
	return _mm256_load_si256(reinterpret_cast<const __m256i*>(&l));
}

[[gnu::always_inline]] inline __m256i shuffle(__m256i x, const Lanes& control) {
	return _mm256_shuffle_epi8(x, load(control));
}

struct Divided {
	__m256i halves, tail;
};

[[gnu::always_inline]] inline Divided divide(__m256i v, __m256i odd, const Constants& k) {
	const __m256i w = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10)), 32),
										_mm256_mul_epu32(odd, load(k.div10)), 0xAA);
	const __m256i h = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10e5)), 47),
										_mm256_srli_epi64(_mm256_mul_epu32(odd, load(k.div10e5)), 15), 0xAA);
	const __m256i halves = _mm256_add_epi32(_mm256_slli_epi32(w, 16), _mm256_mullo_epi32(h, load(k.pack)));
	const __m256i tail = _mm256_sub_epi32(_mm256_add_epi32(v, load(k.tail)), _mm256_mullo_epi32(w, load(k.ten)));
	return {halves, tail};
}

[[gnu::always_inline]] inline __m256i two_digits(__m256i z, const Constants& k) {
	const __m256i tens = _mm256_mulhi_epu16(z, load(k.by10));
	return _mm256_sub_epi16(_mm256_slli_epi16(z, 8), _mm256_mullo_epi16(tens, load(k.tens_out)));
}

[[gnu::always_inline]] inline __m256i text(__m256i digits, const Constants& k) {
	const __m256i upto = _mm256_xor_si256(digits, _mm256_add_epi64(digits, load(k.ones)));
	return _mm256_blendv_epi8(_mm256_or_si256(digits, load(k.zero)), load(k.space), upto);
}

[[gnu::always_inline]] inline void digits(const Divided& d, __m256i& lo, __m256i& hi, const Constants& k) {
	const __m256i hundreds = _mm256_srli_epi16(_mm256_mulhi_epu16(d.halves, load(k.by100)), 3);
	const __m256i rest = _mm256_sub_epi16(d.halves, _mm256_mullo_epi16(hundreds, load(k.hundred)));
	lo = text(two_digits(_mm256_unpacklo_epi16(hundreds, rest), k), k);
	hi = text(two_digits(_mm256_unpackhi_epi16(hundreds, rest), k), k);
}

[[gnu::always_inline]] inline void store(char* p, const Divided& lo, const Divided& hi, const Constants& k) {
	__m256i d01, d23, d45, d67;
	const auto either = [&](__m256i a, __m256i b) __attribute__((target("avx2"))) { return _mm256_or_si256(a, b); };
	digits(lo, d01, d23, k);
	digits(hi, d45, d67, k);
	const __m256i c0 = either(shuffle(d01, k.chunk0[0]), shuffle(lo.tail, k.chunk0[1]));
	const __m256i c1 = either(either(shuffle(d01, k.chunk1[0]), shuffle(lo.tail, k.chunk1[1])), shuffle(d23, k.chunk1[2]));
	const __m256i c2 = either(either(shuffle(d23, k.chunk2[0]), shuffle(lo.tail, k.chunk2[1])), shuffle(d45, k.chunk2[2]));
	const __m256i c3 = either(either(shuffle(hi.tail, k.chunk3[0]), shuffle(d45, k.chunk3[1])), shuffle(d67, k.chunk3[2]));
	const __m256i c4 = either(shuffle(d67, k.chunk4[0]), shuffle(hi.tail, k.chunk4[1]));

	const auto whole = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p + 64 + 16 * c), chunk); };
	const auto low = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm_store_si128(reinterpret_cast<__m128i*>(p + 16 * c), _mm256_castsi256_si128(chunk)); };
	whole(4, c4), whole(3, c3), whole(2, c2), whole(1, c1), whole(0, c0);
	low(0, c0), low(1, c1), low(2, c2), low(3, c3), low(4, c4);
}

[[gnu::always_inline]] inline void divide16(const std::uint32_t* x, Divided& lo, Divided& hi, const Constants& k) {
	const auto at = [x](int i) __attribute__((target("avx2"))) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(x + i)); };
	const auto both = [x](int i) __attribute__((target("avx2"))) { return _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i*>(x + i))); };
	const __m256i l = _mm256_blend_epi32(at(0), both(8), 0xF0), h = _mm256_blend_epi32(at(8), both(4), 0x0F);
	lo = divide(l, _mm256_srli_epi64(l, 32), k);
	hi = divide(h, _mm256_srli_epi64(h, 32), k);
}

inline void format(char* p, const std::uint32_t* x, std::size_t count, const Constants& k) {
	Divided dl, dh;
	divide16(x, dl, dh, k);
	for (std::size_t i = 16; i < count; i += 16, p += 160) {
		Divided nl, nh;
		divide16(x + i, nl, nh, k);
		store(p, dl, dh, k);
		dl = nl, dh = nh;
	}
	store(p, dl, dh, k);
}

}

inline constexpr std::size_t kBlock = 25600;
inline constexpr std::size_t kTextBytes = 10 * kBlock;

}

namespace fps_detail {
[[gnu::always_inline]] inline __m256i io_values(__m256i x) {
	const auto p = _mm256_set1_epi32(mint::mod()), ni = _mm256_set1_epi32(998244351);
	auto even = _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffULL));
	auto odd = _mm256_srli_epi64(x, 32);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	const auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
	return _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
}

inline void write_coefficients_native(FastOutput &output, const mint *in, std::size_t n) {
	alignas(32) unsigned values[25600];
	alignas(32) char text[256000];
	const auto &k = fps_io_fields::detail::kConstants;
	for (std::size_t i = 0; i < n; i += 25600) {
		const auto count = std::min<std::size_t>(25600, n - i), full = count / 16 * 16;
		char *end = text;
		static constexpr char zeros[] = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ";
		const auto empty = [&]() __attribute__((target("avx2"))) {
			std::size_t at = 0;
			const auto p = _mm256_set1_epi32(mint::mod());
			for (; at + 8 <= count; at += 8) {
				__m256i x;
				std::memcpy(&x, in + i + at, sizeof(x));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
				if (!_mm256_testz_si256(x, x)) return false;
			}
			for (; at < count; ++at) if (in[i + at] != mint(0)) return false;
			return true;
		};
		if (empty()) {
			for (std::size_t at = 0; at < count; at += 16) std::memcpy(text + 2 * at, zeros, 32);
			end += 2 * count;
		} else {
			std::size_t j = 0;
			for (; j + 8 <= count; j += 8) {
				alignas(32) unsigned words[8];
				std::memcpy(words, in + i + j, sizeof(words));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(values + j), io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words))));
			}
			for (; j < count; ++j) values[j] = in[i + j].val();
			auto zero = [&](std::size_t at) __attribute__((target("avx2"))) {
				auto x = _mm256_or_si256(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + at)), _mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + at + 8)));
				return _mm256_testz_si256(x, x);
			};
			std::size_t at = 0;
			while (at < full) {
				if (zero(at)) { std::memcpy(end, zeros, 32); end += 32; at += 16; }
				else {
					const auto first = at;
					do at += 16; while (at < full && !zero(at));
					fps_io_fields::detail::format(end, values + first, at - first, k);
					end += 10 * (at - first);
				}
			}
			if (full < count) {
				alignas(32) unsigned rest[16] = {};
				alignas(16) char last[160];
				std::copy_n(values + full, count - full, rest);
				bool empty = true;
				for (auto x : rest) empty &= x == 0;
				if (empty) { std::memcpy(end, zeros, 2 * (count - full)); end += 2 * (count - full); }
				else {
					fps_io_fields::detail::format(last, rest, 16, k);
					std::memcpy(end, last, 10 * (count - full));
					end += 10 * (count - full);
				}
			}
		}
		if (i + count == n) end[-1] = '\n';
		fast_io_detail::BulkOutput::write(output, std::string_view(text, end - text));
	}
	if (!n) output.write('\n');
}
}
#pragma GCC pop_options
#endif

namespace fps_detail {
inline void write_coefficients(FastOutput &output, const mint *in, std::size_t n) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) return write_coefficients_native(output, in, n);
#endif
	for (std::size_t i = 0; i < n; ++i) { output.write(in[i].val()); output.write(' '); }
	output.write('\n');
}
}
#endif
