#ifndef MULTIVARIATE_RANKED_HPP
#define MULTIVARIATE_RANKED_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <ranges>
#include <vector>
#include "../math/mint.hpp"

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace multivariate_ranked_detail {
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using Vec = __m256i;
constexpr u32 kP = 998244353;
constexpr u32 kPInverse = [] {
	u32 x = kP;
	for (int i = 0; i < 5; ++i) x *= 2 - kP * x;
	return x;
}();
constexpr u32 k2To32 = u32((u64(1) << 32) % kP);
constexpr u32 multiply_mod(u32 a, u32 b) { return u32(u64(a) * b % kP); }
constexpr u32 power_mod(u32 a, u64 e) {
	u32 r = 1;
	for (; e; e >>= 1, a = multiply_mod(a, a))
		if (e & 1) r = multiply_mod(r, a);
	return r;
}
constexpr u32 inverse_mod(u32 a) { return power_mod(a, kP - 2); }
constexpr u32 montgomery(u32 a) { return multiply_mod(a, k2To32); }
__attribute__((target("avx2"))) inline Vec all(u32 x) { return _mm256_set1_epi32(int(x)); }
__attribute__((target("avx2"))) inline Vec load(const void* p) { return _mm256_loadu_si256(static_cast<const Vec*>(p)); }
__attribute__((target("avx2"))) inline void store(void* p, Vec x) { _mm256_storeu_si256(static_cast<Vec*>(p), x); }
__attribute__((target("avx2"))) inline Vec add_mod(Vec x, Vec y) {
	const Vec s = _mm256_add_epi32(x, y);
	return _mm256_min_epu32(s, _mm256_sub_epi32(s, all(kP)));
}
__attribute__((target("avx2"))) inline Vec sub_mod(Vec x, Vec y) {
	const Vec d = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(d, _mm256_add_epi32(d, all(kP)));
}
template <bool Wide>
__attribute__((target("avx2"))) inline Vec reduce(Vec even, Vec odd) {
	Vec high = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
	if constexpr (Wide) {
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(2 * kP)));
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(2 * kP)));
		high = _mm256_min_epu32(high, _mm256_sub_epi32(high, all(kP)));
	}
	const Vec qe = _mm256_mul_epu32(even, all(kPInverse)), qo = _mm256_mul_epu32(odd, all(kPInverse));
	const Vec me = _mm256_mul_epu32(qe, all(kP)), mo = _mm256_mul_epu32(qo, all(kP));
	const Vec r = _mm256_sub_epi32(high, _mm256_blend_epi32(_mm256_srli_epi64(me, 32), mo, 0xAA));
	return _mm256_min_epu32(r, _mm256_add_epi32(r, all(kP)));
}
__attribute__((target("avx2"))) inline Vec fold(Vec s) {
	return _mm256_add_epi64(_mm256_and_si256(s, _mm256_set1_epi64x(0xFFFFFFFF)),
		_mm256_mul_epu32(_mm256_srli_epi64(s, 32), _mm256_set1_epi64x(k2To32)));
}
__attribute__((target("avx2"))) inline Vec montgomery_multiply(Vec x, Vec y) {
	const Vec even = _mm256_mul_epu32(x, y);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
	return reduce<false>(even, odd);
}
struct Buffer {
	Vec* data;
	explicit Buffer(std::size_t n) : data(new Vec[n]()) {}
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	~Buffer() { delete[] data; }
};
constexpr int kMaxRank = 24;
__attribute__((target("avx2"))) inline Vec by_popcount(const Vec* x) {
	const Vec low = _mm256_blend_epi32(x[0], x[1], 0x16);
	return _mm256_blend_epi32(_mm256_blend_epi32(low, x[2], 0x68), x[3], 0x80);
}
template <bool Inverse>
__attribute__((target("avx2"))) inline Vec lanes_transform(Vec x) {
	if constexpr (Inverse) {
		x = sub_mod(x, _mm256_slli_epi64(x, 32));
		x = sub_mod(x, _mm256_slli_si256(x, 8));
		return sub_mod(x, _mm256_permute2x128_si256(x, x, 0x08));
	} else {
		x = add_mod(x, _mm256_slli_epi64(x, 32));
		x = add_mod(x, _mm256_slli_si256(x, 8));
		return add_mod(x, _mm256_permute2x128_si256(x, x, 0x08));
	}
}
template <bool Inverse, int N>
__attribute__((target("avx2"))) inline void evaluate_line(Vec* v, int stride) {
	Vec &a0 = v[0], &a1 = v[stride];
	if constexpr (N == 2) {
		a1 = Inverse ? sub_mod(a1, a0) : add_mod(a1, a0);
	} else if constexpr (!Inverse) {
		Vec& a2 = v[2 * stride];
		const Vec s = add_mod(a0, a2);
		a2 = sub_mod(s, a1), a1 = add_mod(s, a1);
	} else {
		Vec& a2 = v[2 * stride];
		const Vec f0 = add_mod(a0, a0), f1 = a1, f2 = a2;
		a0 = f0, a1 = sub_mod(f1, f2), a2 = sub_mod(add_mod(f1, f2), f0);
	}
}
template <bool Inverse, int A, int B>
__attribute__((target("avx2"))) inline void variable_step(Vec* p, std::size_t sa, std::size_t sb, int first, int last) {
	for (int r = first; r <= last; ++r) {
		Vec v[A * B];
#pragma GCC unroll 9
		for (int j = 0; j < B; ++j)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) v[i + A * j] = p[r + i * sa + j * sb];
#pragma GCC unroll 3
		for (int j = 0; j < B; ++j) evaluate_line<Inverse, A>(v + A * j, 1);
		if constexpr (B > 1)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) evaluate_line<Inverse, B>(v + i, A);
#pragma GCC unroll 9
		for (int j = 0; j < B; ++j)
#pragma GCC unroll 3
			for (int i = 0; i < A; ++i) p[r + i * sa + j * sb] = v[i + A * j];
	}
}
__attribute__((target("avx2"))) inline void add_row(Vec* dst, const Vec* src, std::size_t count, bool negative, bool first) {
	const Vec zero = _mm256_setzero_si256();
	if (first && negative)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, sub_mod(zero, load(src + i)));
	else if (first)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, load(src + i));
	else if (negative)
		for (std::size_t i = 0; i < count; ++i) store(dst + i, sub_mod(load(dst + i), load(src + i)));
	else
		for (std::size_t i = 0; i < count; ++i) store(dst + i, add_mod(load(dst + i), load(src + i)));
}
struct Term {
	u32 row;
	bool negative;
	std::uint8_t rank;
	bool first = false;
};
class Ranked {
public:
	Ranked(const std::vector<u32>& n, std::size_t size) : size_(size) {
		const std::size_t k = n.size();
		std::vector<u32> order;
		for (std::size_t l = 0; l < k && order.size() < 3; ++l)
			if (n[l] == 2) order.push_back(u32(l));
		for (std::size_t l = 0; l < k; ++l)
			if (std::ranges::find(order, u32(l)) == order.end()) order.push_back(u32(l));
		for (u32 l : order) size_of_.push_back(n[l]);
		for (u32 x : n) rank_ += int(x) - 1, threes_ += x == 3;
		bottom_ = 3, bottom_size_ = 8;
		while (bottom_ < k && 8 * std::size_t(rank_ + 1) * bottom_size_ * size_of_[bottom_] <= (640 << 10))
			bottom_size_ *= size_of_[bottom_++];
		top_size_ = size_ / bottom_size_;
		int top_rank = 0;
		for (std::size_t l = bottom_; l < k; ++l) top_rank += int(size_of_[l]) - 1;
		planes_ = top_rank + 1;
		if (!std::ranges::is_sorted(order)) {
			std::vector<u32> stride(k + 1);
			for (std::size_t l = 0, s = 1; l <= k; s *= l < k ? n[l] : 1, ++l) stride[l] = u32(s);
			for (int w = 0; w < 8; ++w)
				lane_offset_[w] = (w & 1 ? stride[order[0]] : 0) + (w & 2 ? stride[order[1]] : 0) + (w & 4 ? stride[order[2]] : 0);
			std::vector<u32> base{0};
			for (u32 l = 0; l < order[2]; ++l) {
				if (l == order[0] || l == order[1]) continue;
				const std::size_t count = base.size();
				for (u32 d = 1; d < n[l]; ++d)
					for (std::size_t i = 0; i < count; ++i) base.push_back(base[i] + d * stride[l]);
			}
			block_base_ = std::move(base);
		}
		describe_vectors();
		describe_ranges();
		describe_top();
	}
	__attribute__((target("avx2"))) void multiply(u32* f, u32* g, u32* c) {
		std::vector<u32> scratch_storage(reordered() ? 8 * block_base_.size() : 0);
		u32* scratch = scratch_storage.data();
		to_our_order(f, scratch);
		to_our_order(g, scratch);
		const u32 *fo = f, *go = g;
		u32* co = c;
		const std::size_t vectors = bottom_size_ / 8, plane = std::size_t(planes_) * vectors;
		const std::size_t rows = vectors / group_;
		Buffer top_f_storage(plane), top_g_storage(plane), a_storage(std::size_t(rank_ + 1) * vectors), b_storage(std::size_t(rank_ + 1) * vectors);
		Vec *top_f = top_f_storage.data, *top_g = top_g_storage.data, *a = a_storage.data, *b = b_storage.data;
		for (std::size_t t = 0; t < top_size_; ++t) {
			evaluate_top(fo, t, top_f);
			evaluate_top(go, t, top_g);
			if (const int z = top_zeros3_[t]) {
				const Vec factor = all(montgomery(power_mod(2, z)));
				for (std::size_t i = 0; i < plane; ++i) top_f[i] = montgomery_multiply(top_f[i], factor);
			}
			const int forward_extra = top_cap_[t] + 3, inverse_extra = planes_ + 2;
			for (std::size_t s = 0; s < vectors; s += group_) {
				spread(top_f, a, s);
				steps<false>(a, forward_, 0, forward_.split, s, 1, group_, forward_extra);
				spread(top_g, b, s);
				steps<false>(b, forward_, 0, forward_.split, s, 1, group_, forward_extra);
			}
			for (std::size_t s = 0; s < group_; ++s) {
				steps<false>(a, forward_, forward_.split, forward_.steps.size(), s, group_, rows, forward_extra);
				steps<false>(b, forward_, forward_.split, forward_.steps.size(), s, group_, rows, forward_extra);
				pointwise(a, b, s, group_, rows, top_cap_[t], top_nz_[t]);
				steps<true>(a, inverse_, 0, inverse_.split, s, group_, rows, inverse_extra);
			}
			for (std::size_t s = 0; s < vectors; s += group_) {
				steps<true>(a, inverse_, inverse_.split, inverse_.steps.size(), s, 1, group_, inverse_extra);
				gather(a, top_f, s);
			}
			interpolate_top(top_f, t, co);
		}
		const Vec scale = all(montgomery(multiply_mod(k2To32, power_mod(inverse_mod(2), threes_))));
		if (!reordered()) {
			for (std::size_t i = 0; i < size_; i += 8) store(c + i, montgomery_multiply(load(c + i), scale));
			return;
		}
		const std::size_t block = 8 * block_base_.size();
		for (std::size_t s = 0; s < size_; s += block) {
			std::memcpy(scratch, c + s, block * sizeof(u32));
			for (std::size_t u = 0; u < block_base_.size(); ++u) {
				alignas(32) u32 x[8];
				store(x, montgomery_multiply(load(scratch + 8 * u), scale));
				for (int w = 0; w < 8; ++w) c[s + block_base_[u] + lane_offset_[w]] = x[w];
			}
		}
	}
private:
	struct Range {
		std::uint8_t first, span;
	};
	struct Step {
		u32 first, width;
		std::size_t stride;
	};
	struct Pass {
		std::vector<Step> steps;
		std::size_t split = 0;
		std::vector<Range> ranges;
	};
	bool reordered() const { return !block_base_.empty(); }
	__attribute__((target("avx2"))) void to_our_order(u32* x, u32* scratch) const {
		if (!reordered()) return;
		const std::size_t block = 8 * block_base_.size();
		const Vec offset = load(lane_offset_.data());
		for (std::size_t s = 0; s < size_; s += block) {
			std::memcpy(scratch, x + s, block * sizeof(u32));
			for (std::size_t u = 0; u < block_base_.size(); ++u)
				store(x + s + 8 * u,
					  _mm256_i32gather_epi32(reinterpret_cast<const int*>(scratch + block_base_[u]), offset, 4));
		}
	}
	void describe_vectors() {
		const std::size_t count = bottom_size_ / 8;
		vector_sum_.resize(count), vector_cap_.resize(count), vector_nz_.resize(count);
		std::vector<u32> digit(bottom_, 0);
		int sum = 0, cap = 0, nz = 0;
		for (std::size_t u = 0; u < count; ++u) {
			vector_sum_[u] = std::uint8_t(sum), vector_cap_[u] = std::uint8_t(cap), vector_nz_[u] = std::uint8_t(nz);
			for (std::size_t l = 3; l < bottom_; ++l) {
				const int n = int(size_of_[l]);
				if (++digit[l] < u32(n)) {
					++sum;
					if (digit[l] == 1) cap += n - 1, ++nz;
					break;
				}
				digit[l] = 0, sum -= n - 1, cap -= n - 1, --nz;
			}
		}
	}
	void describe_ranges() {
		const std::size_t count = bottom_size_ / 8;
		std::size_t boundary = 3;
		for (group_ = 1; group_ * group_ < count; group_ *= size_of_[boundary++]) {
		}
		const auto pairs = [this](std::size_t from, std::size_t to, bool inverse) {
			std::vector<Step> steps;
			for (std::size_t l = from; l < to;) {
				const bool pair = l + 1 < to && (!inverse || (size_of_[l] == 2 && size_of_[l + 1] == 2));
				steps.push_back({u32(l), pair ? 2u : 1u, 0});
				l += pair ? 2 : 1;
			}
			if (inverse) std::ranges::reverse(steps);
			return steps;
		};
		forward_.steps = pairs(3, boundary, false);
		forward_.split = forward_.steps.size();
		std::ranges::copy(pairs(boundary, bottom_, false), std::back_inserter(forward_.steps));
		inverse_.steps = pairs(boundary, bottom_, true);
		inverse_.split = inverse_.steps.size();
		std::ranges::copy(pairs(3, boundary, true), std::back_inserter(inverse_.steps));
		for (const bool inverse : {false, true}) {
			Pass& pass = inverse ? inverse_ : forward_;
			for (Step& step : pass.steps) {
				step.stride = 1;
				for (std::size_t m = 3; m < step.first; ++m) step.stride *= size_of_[m];
			}
			pass.ranges.resize(pass.steps.size() * count);
			std::vector<u32> digit(bottom_, 0);
			for (std::size_t u = 0; u < count; ++u) {
				for (std::size_t k = 0; k < pass.steps.size(); ++k) {
					const std::size_t l = pass.steps[k].first, e = l + pass.steps[k].width - 1;
					int above = 0, cap = 0, own = 0, up_to = 0;
					for (std::size_t m = 3; m < bottom_; ++m) {
						const int d = int(digit[m]), n = int(size_of_[m]);
						if (m > e) above += d;
						if (m < l) cap += d ? n - 1 : 0;
						if (m >= l && m <= e) own += n - 1;
						if (m <= e) up_to += n - 1;
					}
					pass.ranges[k * count + u] = inverse ? Range{std::uint8_t(above), std::uint8_t(up_to)}
														 : Range{std::uint8_t(above), std::uint8_t(own + cap)};
				}
				for (std::size_t l = 3; l < bottom_ && ++digit[l] == size_of_[l]; ++l) digit[l] = 0;
			}
		}
	}
	void describe_top() {
		struct Entry {
			u32 digit;
			bool negative;
		};
		static constexpr Entry kEvaluate[2][3][3] = {
			{{{0, false}}, {{0, false}, {1, false}}, {}},
			{{{0, false}}, {{0, false}, {1, false}, {2, false}}, {{0, false}, {1, true}, {2, false}}}};
		static constexpr Entry kInterpolate[2][3][3] = {
			{{{0, false}, {1, true}}, {{1, false}}, {}},
			{{{0, false}, {2, true}}, {{1, false}, {2, false}}, {{1, true}, {2, false}}}};
		static constexpr int kEvaluateCount[2][3] = {{1, 2, 0}, {1, 3, 3}};
		static constexpr int kInterpolateCount[2][3] = {{2, 1, 0}, {2, 2, 2}};
		const std::size_t k = size_of_.size();
		evaluate_start_.push_back(0), interpolate_start_.push_back(0);
		std::vector<u32> point(k, 0);
		for (std::size_t t = 0; t < top_size_; ++t) {
			std::vector<Term> evaluate{{0, false, 0}}, interpolate{{0, false, 0}};
			int cap = 0, nz = 0, zeros3 = 0;
			u32 stride = 1;
			for (std::size_t l = bottom_; l < k; stride *= size_of_[l++]) {
				const u32 p = point[l];
				const bool three = size_of_[l] == 3;
				if (p) cap += int(size_of_[l]) - 1, ++nz;
				zeros3 += three && p == 0;
				const auto extend = [&](std::vector<Term>& terms, const Entry* table, int count) {
					std::vector<Term> next;
					for (const Term& x : terms)
						for (int e = 0; e < count; ++e) {
							const u32 d = table[e].digit;
							next.push_back({x.row + d * stride, x.negative != table[e].negative, std::uint8_t(x.rank + d)});
						}
					terms = std::move(next);
				};
				extend(evaluate, kEvaluate[three][p], kEvaluateCount[three][p]);
				extend(interpolate, kInterpolate[three][p], kInterpolateCount[three][p]);
			}
			std::vector<bool> seen(planes_, false);
			for (Term& term : evaluate) term.first = !seen[term.rank], seen[term.rank] = true;
			terms_.insert(terms_.end(), evaluate.begin(), evaluate.end());
			evaluate_start_.push_back(u32(terms_.size()));
			inverse_terms_.insert(inverse_terms_.end(), interpolate.begin(), interpolate.end());
			interpolate_start_.push_back(u32(inverse_terms_.size()));
			top_cap_.push_back(std::uint8_t(cap)), top_nz_.push_back(std::uint8_t(nz));
			top_zeros3_.push_back(std::uint8_t(zeros3));
			for (std::size_t l = bottom_; l < k && ++point[l] == size_of_[l]; ++l) point[l] = 0;
		}
	}
	__attribute__((target("avx2"))) void evaluate_top(const u32* x, std::size_t t, Vec* planes) const {
		const std::size_t vectors = bottom_size_ / 8, filled = top_cap_[t] + std::size_t(1);
		std::memset(static_cast<void*>(planes + filled * vectors), 0, (planes_ - filled) * vectors * sizeof(Vec));
		for (u32 i = evaluate_start_[t]; i < evaluate_start_[t + 1]; ++i) {
			const Term& term = terms_[i];
			add_row(planes + term.rank * vectors, reinterpret_cast<const Vec*>(x + term.row * bottom_size_), vectors,
					term.negative, term.first);
		}
	}
	__attribute__((target("avx2"))) void interpolate_top(const Vec* planes, std::size_t t, u32* c) const {
		const std::size_t vectors = bottom_size_ / 8;
		for (u32 i = interpolate_start_[t]; i < interpolate_start_[t + 1]; ++i) {
			const Term& term = inverse_terms_[i];
			add_row(reinterpret_cast<Vec*>(c + term.row * bottom_size_), planes + term.rank * vectors, vectors,
				term.negative, false);
		}
	}
	template <bool Inverse>
	__attribute__((target("avx2"))) void steps(Vec* x, const Pass& pass, std::size_t begin, std::size_t end, std::size_t o, std::size_t sigma,
			   std::size_t count, int extra) const {
		for (std::size_t k = begin; k < end; ++k) {
			const Step& s = pass.steps[k];
			const Range* range = pass.ranges.data() + k * (bottom_size_ / 8);
			const u32 a = size_of_[s.first], b = s.width == 2 ? size_of_[s.first + 1] : 1;
			switch (a * 4 + b) {
			case 9: block_step<Inverse, 2, 1>(x, s.stride, range, extra, o, sigma, count); break;
			case 13: block_step<Inverse, 3, 1>(x, s.stride, range, extra, o, sigma, count); break;
			case 10: block_step<Inverse, 2, 2>(x, s.stride, range, extra, o, sigma, count); break;
			case 11: block_step<Inverse, 2, 3>(x, s.stride, range, extra, o, sigma, count); break;
			case 14: block_step<Inverse, 3, 2>(x, s.stride, range, extra, o, sigma, count); break;
			default: block_step<Inverse, 3, 3>(x, s.stride, range, extra, o, sigma, count); break;
			}
		}
	}
	template <bool Inverse, int A, int B>
	__attribute__((target("avx2"))) void block_step(Vec* x, std::size_t stride, const Range* range, int extra, std::size_t o, std::size_t sigma,
					std::size_t count) const {
		const std::size_t ranks = std::size_t(rank_ + 1), inner = stride / sigma;
		for (std::size_t base = 0; base < count; base += inner * A * B)
			for (std::size_t q = base; q < base + inner; ++q) {
				const std::size_t u = o + sigma * q;
				variable_step<Inverse, A, B>(x + u * ranks, stride * ranks, stride * A * ranks, range[u].first,
											 std::min(rank_, range[u].first + range[u].span + extra));
			}
	}
	__attribute__((target("avx2"))) void spread(const Vec* planes, Vec* out, std::size_t s) const {
		const std::size_t vectors = bottom_size_ / 8;
		const Vec zero = _mm256_setzero_si256();
		Vec p[kMaxRank + 8];
		for (int i = 0; i < 3; ++i) p[i] = p[3 + planes_ + i] = zero;
		for (std::size_t u = s; u < s + group_; ++u) {
			for (int rho = 0; rho < planes_; ++rho) p[3 + rho] = planes[rho * vectors + u];
			const int d = vector_sum_[u], last = std::min(rank_, d + planes_ + 2);
			Vec* ranks = out + u * (rank_ + 1);
			for (int r = 0; r < d; ++r) ranks[r] = zero;
			for (int r = d; r <= last; ++r) {
				const Vec* q = p + 3 + r - d;
				const Vec x[4] = {q[0], q[-1], q[-2], q[-3]};
				ranks[r] = lanes_transform<false>(by_popcount(x));
			}
			for (int r = last + 1; r <= rank_; ++r) ranks[r] = zero;
		}
	}
	__attribute__((target("avx2"))) void gather(const Vec* in, Vec* planes, std::size_t s) const {
		const std::size_t vectors = bottom_size_ / 8;
		Vec m[kMaxRank + 4];
		for (std::size_t u = s; u < s + group_; ++u) {
			const int d = vector_sum_[u], last = std::min(rank_, d + planes_ + 2);
			for (int r = d; r <= last; ++r) m[r - d] = lanes_transform<true>(in[u * (rank_ + 1) + r]);
			for (int k = last - d + 1; k < planes_ + 3; ++k) m[k] = _mm256_setzero_si256();
			for (int rho = 0; rho < planes_; ++rho) planes[rho * vectors + u] = by_popcount(m + rho);
		}
	}
	__attribute__((target("avx2"))) void pointwise(Vec* a, const Vec* b, std::size_t o, std::size_t sigma, std::size_t count, int top_cap,
				   int top_nz) const {
		const Vec zero = _mm256_setzero_si256();
		Vec ae[kMaxRank], ao[kMaxRank], be[kMaxRank + 2 * kPad], bo[kMaxRank + 2 * kPad];
		for (int i = 0; i < kPad; ++i) be[i] = bo[i] = zero;
		for (std::size_t u = o; u < o + sigma * count; u += sigma) {
			const int cap = std::min(rank_, top_cap + vector_cap_[u] + 3);
			const int first = top_nz + vector_nz_[u], last = std::min(rank_, 2 * cap);
			Vec* x = a + u * (rank_ + 1);
			const Vec* y = b + u * (rank_ + 1);
			for (int i = 0; i <= cap; ++i) {
				ae[i] = x[i], ao[i] = _mm256_srli_epi64(ae[i], 32);
				be[kPad + i] = y[i], bo[kPad + i] = _mm256_srli_epi64(y[i], 32);
			}
			for (int i = cap + 1; i <= cap + kPad; ++i) be[kPad + i] = bo[kPad + i] = zero;
			for (int r = first; r <= last; r += kBlock) {
				const int lo = std::max(0, r - cap), count = std::min(r + kBlock - 1, cap) - lo + 1;
				Vec even[kBlock], odd[kBlock];
				block_sums(ae + lo, be + kPad + r - lo, count, even);
				block_sums(ao + lo, bo + kPad + r - lo, count, odd);
				for (int k = 0; k < kBlock && r + k <= last; ++k) x[r + k] = reduce<true>(even[k], odd[k]);
			}
		}
	}
	static constexpr int kBlock = 4, kPad = kBlock;
	__attribute__((target("avx2"))) static void block_sums(const Vec* x, const Vec* y, int count, Vec* sum) {
		Vec s0 = _mm256_setzero_si256(), s1 = s0, s2 = s0, s3 = s0;
		Vec w0 = y[0], w1 = y[1], w2 = y[2], w3 = y[3];
		const auto term = [&](Vec a, Vec y0, Vec y1, Vec y2, Vec y3) __attribute__((always_inline, target("avx2"))) {
			s0 = _mm256_add_epi64(s0, _mm256_mul_epu32(a, y0));
			s1 = _mm256_add_epi64(s1, _mm256_mul_epu32(a, y1));
			s2 = _mm256_add_epi64(s2, _mm256_mul_epu32(a, y2));
			s3 = _mm256_add_epi64(s3, _mm256_mul_epu32(a, y3));
			asm("" : "+x"(s0), "+x"(s1), "+x"(s2), "+x"(s3));
		};
		const auto run = [&](int n) __attribute__((always_inline, target("avx2"))) {
			for (; n >= 4; n -= 4, x += 4, y -= 4) {
				term(x[0], w0, w1, w2, w3), w3 = y[-1];
				term(x[1], w3, w0, w1, w2), w2 = y[-2];
				term(x[2], w2, w3, w0, w1), w1 = y[-3];
				term(x[3], w1, w2, w3, w0), w0 = y[-4];
			}
			if (n >= 1) term(x[0], w0, w1, w2, w3);
			if (n >= 2) term(x[1], y[-1], w0, w1, w2);
			if (n >= 3) term(x[2], y[-2], y[-1], w0, w1);
		};
		if (count <= 18) {
			run(count);
		} else {
			run(16);

			s0 = fold(s0), s1 = fold(s1), s2 = fold(s2), s3 = fold(s3);
			run(count - 16);
		}
		sum[0] = s0, sum[1] = s1, sum[2] = s2, sum[3] = s3;
	}
	std::size_t size_, bottom_size_ = 0, top_size_ = 0, bottom_ = 0;
	int rank_ = 0, planes_ = 0, threes_ = 0;
	std::vector<u32> size_of_;
	alignas(32) std::array<u32, 8> lane_offset_{};
	std::vector<u32> block_base_;
	std::vector<std::uint8_t> vector_sum_, vector_cap_, vector_nz_;
	std::size_t group_ = 1;
	Pass forward_, inverse_;
	std::vector<std::uint8_t> top_cap_, top_nz_, top_zeros3_;
	std::vector<Term> terms_, inverse_terms_;
	std::vector<u32> evaluate_start_, interpolate_start_;
};

__attribute__((target("avx2"))) inline std::vector<mint> multiply(const std::vector<mint>& a, const std::vector<mint>& b, const std::vector<int>& dimensions) {
	std::vector<u32> dims(dimensions.begin(), dimensions.end()), f(a.size()), g(b.size()), result(a.size());
	for (std::size_t i = 0; i < a.size(); ++i) f[i] = a[i].val(), g[i] = b[i].val();
	Ranked(dims, a.size()).multiply(f.data(), g.data(), result.data());
	std::vector<mint> out;
	out.reserve(result.size());
	for (u32 x : result) out.push_back(mint::raw(x));
	return out;
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#endif

#endif
