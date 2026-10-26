#ifndef PREDECESSOR_SET_HPP
#define PREDECESSOR_SET_HPP

#include "xor_select.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace predecessor_set_detail {
inline void pack_scalar(const char *src, std::uint64_t *dst, std::size_t words) {
	for (std::size_t i = 0; i < words; ++i) {
		std::uint64_t word = 0;
		if constexpr (std::endian::native == std::endian::little) {
			for (int j = 0; j < 8; ++j) {
				std::uint64_t x;
				std::memcpy(&x, src + i * 64 + j * 8, 8);
				assert((x & 0xfefefefefefefefeULL) == 0x3030303030303030ULL);
				word |= ((x & 0x0101010101010101ULL) * 0x0102040810204080ULL >> 56) << (j * 8);
			}
		} else {
			for (int j = 0; j < 64; ++j) {
				assert(src[i * 64 + j] == '0' || src[i * 64 + j] == '1');
				word |= std::uint64_t(src[i * 64 + j] - '0') << j;
			}
		}
		dst[i] = word;
	}
}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2"))) inline void pack_avx2(const char *src, std::uint64_t *dst, std::size_t words) {
	const auto ones = _mm256_set1_epi8('1');
	for (std::size_t i = 0; i < words; ++i) {
		auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64));
		auto b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64 + 32));
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(a, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(b, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		auto low = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(a, ones)));
		auto high = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(b, ones)));
		dst[i] = low | (std::uint64_t(high) << 32);
	}
}
#endif
inline void pack(const char *src, std::uint64_t *dst, std::size_t words) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (words >= 4 && __builtin_cpu_supports("avx2")) return pack_avx2(src, dst, words);
#endif
	pack_scalar(src, dst, words);
}
}

class PredecessorSet {
	int n = 0, count = 0, height = 0;
	std::array<int, 6> offset{}, length{};
	std::array<int, 16> small{};
	bool small_valid = false;
	std::vector<std::uint64_t> data;
	void mask_tail() {
		int bits = n;
		for (int h = 0; h < height; ++h) {
			if (bits & 63) data[offset[h] + length[h] - 1] &= ~std::uint64_t(0) >> (64 - (bits & 63));
			bits = length[h];
		}
	}
	void rebuild() {
		count = 0;
		for (int i = 0; i < length[0]; ++i) count += std::popcount(data[i]);
		for (int h = 1; h < height; ++h)
			for (int i = 0; i < length[h - 1]; ++i)
				if (data[offset[h - 1] + i]) data[offset[h] + (i >> 6)] |= std::uint64_t(1) << (i & 63);
		rebuild_small();
	}

	template <int H, bool Forward> int descend(int p) const {
		if constexpr (H == 0) return p;
		else {
			auto word = data[offset[H - 1] + p];
			int bit = Forward ? std::countr_zero(word) : 63 - std::countl_zero(word);
			return descend<H - 1, Forward>(p * 64 + bit);
		}
	}
	template <int H, bool Forward> int ascend(int p) const {
		if constexpr (H == 6) return Forward ? n : -1;
		else {
			if (H >= height) return Forward ? n : -1;
			auto word = data[offset[H] + (p >> 6)];
			auto mask = Forward ? (~std::uint64_t(1) << (p & 63)) : ~(~std::uint64_t(0) << (p & 63));
			word &= mask;
			if (word) {
				int bit = Forward ? std::countr_zero(word) : 63 - std::countl_zero(word);
				return descend<H, Forward>((p & ~63) + bit);
			}
			return ascend<H + 1, Forward>(p >> 6);
		}
	}

	void rebuild_small() {
		small_valid = count <= int(small.size());
		if (!small_valid) return;
		small.fill(n);
		for (int i = 0, p = 0; i < count; ++i) {
			auto word = data[p >> 6] >> (p & 63);
			p = word ? p + std::countr_zero(word) : ascend<1, true>(p >> 6);
			small[i] = p++;
		}
	}

	int small_rank(int p) const {
#if defined(__AVX512F__) && defined(__x86_64__)
		auto keys = _mm512_loadu_si512(small.data());
		return std::popcount(unsigned(_mm512_cmp_epi32_mask(keys, _mm512_set1_epi32(p), _MM_CMPINT_LT)));
#elif defined(__AVX2__) && defined(__x86_64__)
		auto key = _mm256_set1_epi32(p);
		auto a = _mm256_cmpgt_epi32(key, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(small.data())));
		auto b = _mm256_cmpgt_epi32(key, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(small.data() + 8)));
		return std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(a)))) + std::popcount(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(b))));
#else
		int result = 0;
		for (int x : small) result += x < p;
		return result;
#endif
	}

public:
	int argmin_xor(std::uint32_t x) const {
		if (!count) return -1;
		std::uint64_t p = std::uint64_t(x) & ((std::uint64_t(1) << (height * 6)) - 1);
		for (int h = 0; h < height; ++h, p >>= 6) {
			auto index = p >> 6;
			if (index >= unsigned(length[h])) continue;
			auto word = data[offset[h] + index];
			if (!word) continue;
			p = index * 64 + xor_select_detail::argmin(word, p & 63);
			while (h--) p = p * 64 + xor_select_detail::argmin(data[offset[h] + p], (x >> (h * 6)) & 63);
			return int(p);
		}
		return -1;
	}
	int argmax_xor(std::uint32_t x) const { return argmin_xor(~x); }
	explicit PredecessorSet(int n = 0, bool filled = false) { assign(n, filled); }
	explicit PredecessorSet(std::string_view bits) { assign(bits); }
	PredecessorSet(int n, std::span<const std::uint64_t> words) { assign(n, words); }
	PredecessorSet(const PredecessorSet &) = default;
	PredecessorSet &operator=(const PredecessorSet &) = default;
	PredecessorSet(PredecessorSet &&other) noexcept : PredecessorSet() { swap(other); }
	PredecessorSet &operator=(PredecessorSet &&other) noexcept {
		if (this != &other) { PredecessorSet moved(std::move(other)); swap(moved); }
		return *this;
	}
	void assign(int length, bool filled = false) {
		assert(length >= 0);
		n = length; count = filled ? n : 0; height = 0;
		offset.fill(0); this->length.fill(0);
		int total = 0;
		for (std::size_t bits = n; bits;) {
			std::size_t words = (bits + 63) / 64;
			offset[height] = total; this->length[height++] = words;
			total += words;
			if (words == 1) break;
			bits = words;
		}
		data.assign(total, filled ? ~std::uint64_t(0) : 0);
		if (filled) mask_tail();
		rebuild_small();
	}
	void assign(std::string_view bits) {
		assert(bits.size() <= std::size_t(std::numeric_limits<int>::max()));
		PredecessorSet next(static_cast<int>(bits.size()));
		std::size_t words = bits.size() / 64;
		predecessor_set_detail::pack(bits.data(), next.data.data(), words);
		for (std::size_t i = words * 64; i < bits.size(); ++i) {
			assert(bits[i] == '0' || bits[i] == '1');
			next.data[i >> 6] |= std::uint64_t(bits[i] - '0') << (i & 63);
		}
		next.rebuild(); swap(next);
	}
	void assign(int n, std::span<const std::uint64_t> words) {
		assert(n >= 0 && words.size() == (std::size_t(n) + 63) / 64);
		PredecessorSet next(n);
		std::copy(words.begin(), words.end(), next.data.begin());
		if (n & 63) next.data[next.length[0] - 1] &= ~std::uint64_t(0) >> (64 - (n & 63));
		next.rebuild(); swap(next);
	}
	int universe_size() const { return n; }
	int size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(int p) const { assert(0 <= p && p < n); return data[p >> 6] >> (p & 63) & 1; }
	bool insert(int p) {
		assert(0 <= p && p < n);
		int index = p >> 6;
		auto &leaf = data[index];
		auto mask = std::uint64_t(1) << (p & 63), before = leaf;
		leaf = before | mask;
		bool changed = !(before & mask);
		count += changed;
		if (!before) for (int h = 1; h < height; ++h) {
			auto &word = data[offset[h] + (index >> 6)];
			auto old = word;
			word |= std::uint64_t(1) << (index & 63);
			if (old) break;
			index >>= 6;
		}
		small_valid = false;
		return changed;
	}
	bool erase(int p) {
		assert(0 <= p && p < n);
		int index = p >> 6;
		auto &leaf = data[index];
		auto mask = std::uint64_t(1) << (p & 63), before = leaf;
		leaf = before & ~mask;
		bool changed = (before & mask) != 0;
		count -= changed;
		if (before && !leaf) for (int h = 1; h < height; ++h) {
			auto &word = data[offset[h] + (index >> 6)];
			word &= ~(std::uint64_t(1) << (index & 63));
			if (word) break;
			index >>= 6;
		}
		small_valid = false;
		return changed;
	}
	int next(int p) const {
		p = std::max(p, 0);
		if (p >= n || !count) return n;
		if (small_valid) {
			int index = small_rank(p);
			return index == count ? n : small[index];
		}
		auto local = data[p >> 6] >> (p & 63);
		if (local) return p + std::countr_zero(local);
		return ascend<1, true>(p >> 6);
	}
	int prev(int p) const {
		p = std::min(p, n - 1);
		if (p < 0 || !count) return -1;
		if (small_valid) {
			int index = small_rank(p + 1);
			return index ? small[index - 1] : -1;
		}
		auto local = data[p >> 6] << (63 - (p & 63));
		if (local) return p - std::countl_zero(local);
		return ascend<1, false>(p >> 6);
	}
	bool set(int p, bool present) {
		assert(0 <= p && p < n);
		int index = p >> 6;
		auto &leaf = data[index];
		auto mask = std::uint64_t(1) << (p & 63), before = leaf;
		auto fill = std::uint64_t(0) - present;
		leaf = before ^ ((before ^ fill) & mask);
		bool old = (before & mask) != 0;
		count += int(present) - int(old);
		if ((!before) != (!leaf)) for (int h = 1; h < height; ++h) {
			auto &word = data[offset[h] + (index >> 6)];
			auto previous = word;
			word = previous ^ ((previous ^ fill) & (std::uint64_t(1) << (index & 63)));
			if ((!previous) == (!word)) break;
			index >>= 6;
		}
		small_valid = false;
		return old != present;
	}
	int min() const { return next(0); }
	int max() const { return prev(n - 1); }
	void clear() { std::fill(data.begin(), data.end(), 0); count = 0; small_valid = false; }
	void fill() {
		std::fill(data.begin(), data.end(), ~std::uint64_t(0));
		mask_tail();
		count = n;
		rebuild_small();
	}
	template <class F> void for_each(int l, int r, F &&f) const {
		assert(0 <= l && l <= r && r <= n);
		for (int p = next(l); p < r;) {
			int base = p & ~63;
			auto word = data[p >> 6] & (~std::uint64_t(0) << (p & 63));
			if (r - base < 64) word &= ~std::uint64_t(0) >> (64 - (r - base));
			while (word) { f(base + std::countr_zero(word)); word &= word - 1; }
			if (r - base <= 64) break;
			p = next(base + 64);
		}
	}
	template <class F> void for_each(F &&f) const { for_each(0, n, f); }
	std::vector<int> to_vector() const {
		std::vector<int> out; out.reserve(count);
		for_each([&](int x) { out.push_back(x); }); return out;
	}
	std::span<const std::uint64_t> words() const & { return {data.data(), std::size_t(length[0])}; }
	void words() const && = delete;
	void swap(PredecessorSet &other) noexcept {
		using std::swap;
		swap(n, other.n); swap(count, other.count); swap(height, other.height);
		offset.swap(other.offset); length.swap(other.length); data.swap(other.data); small.swap(other.small); swap(small_valid, other.small_valid);
	}
	std::size_t memory_usage() const { return data.capacity() * sizeof(std::uint64_t); }
};

#endif
