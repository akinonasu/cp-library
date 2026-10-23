#ifndef RANGE_ADD_RANGE_MIN_HPP
#define RANGE_ADD_RANGE_MIN_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__AVX2__) && defined(__x86_64__)
#include <immintrin.h>
#endif

template<class T = std::int64_t> class RangeAddRangeMin {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	static constexpr unsigned branch = 16;
	static constexpr T infinity = std::numeric_limits<T>::max();
	std::vector<T> data_;
	std::vector<U> lazy_;
	std::array<std::size_t, (sizeof(int) * 8 + 3) / 4 + 1> offsets_{};
	int n_ = 0;
	unsigned height_ = 0;
	std::size_t width_ = 1, leaves_ = 0;
	static T added(T value, U delta) { return std::bit_cast<T>(U(U(value) + delta)); }
#if defined(__AVX2__) && defined(__x86_64__)
	static __m256i minimum(__m256i a, __m256i b) { return _mm256_blendv_epi8(a, b, _mm256_cmpgt_epi64(a, b)); }
#endif
	static T minimum(const T *values, unsigned l, unsigned r) {
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			if (r - l > 3) {
				const auto inf = _mm256_set1_epi64x(infinity);
				auto result = inf;
				const auto low = _mm256_set1_epi64x(static_cast<long long>(l) - 1), high = _mm256_set1_epi64x(r);
				for (unsigned i = 0; i < branch; i += 4) {
					auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + i));
					if (l > i || r < i + 4) {
						const auto indices = _mm256_setr_epi64x(i, i + 1, i + 2, i + 3);
						const auto active = _mm256_and_si256(_mm256_cmpgt_epi64(indices, low), _mm256_cmpgt_epi64(high, indices));
						value = _mm256_blendv_epi8(inf, value, active);
					}
					result = minimum(result, value);
				}
				alignas(32) T lanes[4];
				_mm256_store_si256(reinterpret_cast<__m256i *>(lanes), result);
				return std::min(std::min(lanes[0], lanes[1]), std::min(lanes[2], lanes[3]));
			}
		}
#endif
		T result = infinity;
		for (unsigned i = l; i < r; ++i) result = std::min(result, values[i]);
		return result;
	}
	void pull(std::size_t k, std::size_t children) { data_[k] = minimum(data_.data() + children, 0, branch); }
	void add_children(std::size_t p, unsigned l, unsigned r, U delta) {
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			if (r - l > 3) {
				const auto value = _mm256_set1_epi64x(std::bit_cast<T>(delta));
				const auto low = _mm256_set1_epi64x(static_cast<long long>(l) - 1), high = _mm256_set1_epi64x(r);
				for (unsigned i = 0; i < branch; i += 4) {
					if (i + 4 <= l || r <= i) continue;
					auto change = value;
					if (l > i || r < i + 4) {
						const auto indices = _mm256_setr_epi64x(i, i + 1, i + 2, i + 3);
						change = _mm256_and_si256(value, _mm256_and_si256(_mm256_cmpgt_epi64(indices, low), _mm256_cmpgt_epi64(high, indices)));
					}
					auto *data = reinterpret_cast<__m256i *>(data_.data() + p + i);
					_mm256_storeu_si256(data, _mm256_add_epi64(_mm256_loadu_si256(data), change));
					if (p >= leaves_) {
						auto *tags = reinterpret_cast<__m256i *>(lazy_.data() + p - leaves_ + i);
						_mm256_storeu_si256(tags, _mm256_add_epi64(_mm256_loadu_si256(tags), change));
					}
				}
				return;
			}
		}
#endif
		for (unsigned i = l; i < r; ++i) data_[p + i] = added(data_[p + i], delta);
		if (p >= leaves_) for (unsigned i = l; i < r; ++i) lazy_[p - leaves_ + i] += delta;
	}
	void push(std::size_t k, std::size_t children) {
		U delta = lazy_[k - leaves_];
		if (!delta) return;
		add_children(children, 0, branch, delta);
		lazy_[k - leaves_] = 0;
	}
	void update(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t ql, std::size_t qr, U delta) {
		std::size_t k = offsets_[level] + index;
		if (ql <= l && l + width <= qr) {
			data_[k] = added(data_[k], delta);
			if (level) lazy_[k - leaves_] += delta;
			return;
		}
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		push(k, p);
		unsigned shift = 4 * (level - 1);
		unsigned a = ql <= l ? 0 : unsigned((ql - l) >> shift);
		unsigned b = qr >= l + width ? branch : unsigned(((qr - l - 1) >> shift) + 1);
		if (a + 1 == b) update(level - 1, index * branch + a, l + a * part, part, ql, qr, delta);
		else {
			if (ql > l + a * part) { update(level - 1, index * branch + a, l + a * part, part, ql, qr, delta); ++a; }
			if (qr < l + b * part) { --b; update(level - 1, index * branch + b, l + b * part, part, ql, qr, delta); }
			add_children(p, a, b, delta);
		}
		pull(k, p);
	}
	T query(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t ql, std::size_t qr, U carry) const {
		std::size_t k = offsets_[level] + index;
		if (ql <= l && l + width <= qr) return added(data_[k], carry);
		carry += lazy_[k - leaves_];
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		unsigned shift = 4 * (level - 1);
		unsigned a = ql <= l ? 0 : unsigned((ql - l) >> shift);
		unsigned b = qr >= l + width ? branch : unsigned(((qr - l - 1) >> shift) + 1);
		if (a + 1 == b) return query(level - 1, index * branch + a, l + a * part, part, ql, qr, carry);
		T result = infinity;
		if (ql > l + a * part) { result = query(level - 1, index * branch + a, l + a * part, part, ql, qr, carry); ++a; }
		if (qr < l + b * part) { --b; result = std::min(result, query(level - 1, index * branch + b, l + b * part, part, ql, qr, carry)); }
		if (a < b) result = std::min(result, added(minimum(data_.data() + p, a, b), carry));
		return result;
	}
	void assign(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t pos, T value) {
		std::size_t k = offsets_[level] + index;
		if (!level) { data_[k] = value; return; }
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		push(k, p);
		unsigned child = unsigned((pos - l) >> (4 * (level - 1)));
		assign(level - 1, index * branch + child, l + child * part, part, pos, value);
		pull(k, p);
	}
	void collect(unsigned level, std::size_t index, std::size_t l, std::size_t width, U carry, std::vector<T> &result) const {
		std::size_t k = offsets_[level] + index;
		if (!level) { result[l] = added(data_[k], carry); return; }
		carry += lazy_[k - leaves_];
		std::size_t part = width / branch;
		for (unsigned child = 0; child < branch && l + child * part < std::size_t(n_); ++child)
			collect(level - 1, index * branch + child, l + child * part, part, carry, result);
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }

  public:
	using value_type = T;
	RangeAddRangeMin() = default;
	explicit RangeAddRangeMin(int n, T value = 0) { assert(n >= 0); build(std::vector<T>(n, value)); }
	explicit RangeAddRangeMin(const std::vector<T> &values) { build(values); }
	RangeAddRangeMin(const RangeAddRangeMin &) = default;
	RangeAddRangeMin(RangeAddRangeMin &&other) noexcept { swap(other); }
	RangeAddRangeMin &operator=(RangeAddRangeMin other) noexcept { swap(other); return *this; }
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		RangeAddRangeMin next;
		next.n_ = int(values.size());
		decltype(offsets_) counts{};
		std::size_t total = 0, real = std::max(std::size_t(1), values.size());
		for (unsigned level = 0;; ++level) {
			next.offsets_[level] = total;
			counts[level] = real;
			std::size_t padded = real == 1 ? 1 : (real + branch - 1) / branch * branch;
			total += padded;
			if (!level) next.leaves_ = padded;
			if (real == 1) { next.height_ = level; break; }
			real = (real + branch - 1) / branch;
			if (next.width_ > std::numeric_limits<std::size_t>::max() / branch) throw std::length_error("RangeAddRangeMin size");
			next.width_ *= branch;
		}
		next.data_.assign(total, infinity);
		next.lazy_.resize(total - next.leaves_);
		std::copy(values.begin(), values.end(), next.data_.begin());
		for (unsigned level = 1; level <= next.height_; ++level)
			for (std::size_t index = 0; index < counts[level]; ++index)
				next.pull(next.offsets_[level] + index, next.offsets_[level - 1] + index * branch);
		swap(next);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	void add(int l, int r, T value) { check(l, r); if (l < r && value) update(height_, 0, 0, width_, l, r, U(value)); }
	void apply(int l, int r, T value) { add(l, r, value); }
	T prod(int l, int r) const { check(l, r); return l < r ? query(height_, 0, 0, width_, l, r, 0) : infinity; }
	T min(int l, int r) const { return prod(l, r); }
	T all_prod() const { return n_ ? data_[offsets_[height_]] : infinity; }
	T all_min() const { return all_prod(); }
	T get(int p) const { assert(0 <= p && p < n_); return prod(p, p + 1); }
	void set(int p, T value) { assert(0 <= p && p < n_); assign(height_, 0, 0, width_, p, value); }
	std::vector<T> values() const { std::vector<T> result(n_); if (n_) collect(height_, 0, 0, width_, 0, result); return result; }
	std::size_t memory_usage() const { return data_.capacity() * sizeof(T) + lazy_.capacity() * sizeof(U); }
	void swap(RangeAddRangeMin &other) noexcept {
		data_.swap(other.data_), lazy_.swap(other.lazy_), offsets_.swap(other.offsets_);
		std::swap(n_, other.n_), std::swap(height_, other.height_), std::swap(width_, other.width_), std::swap(leaves_, other.leaves_);
	}
	friend void swap(RangeAddRangeMin &a, RangeAddRangeMin &b) noexcept { a.swap(b); }
};

#endif
