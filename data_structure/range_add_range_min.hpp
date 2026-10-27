#ifndef RANGE_ADD_RANGE_MIN_HPP
#define RANGE_ADD_RANGE_MIN_HPP

#include "../components/aligned_allocator.hpp"
#include "../components/range_add_min_node.hpp"
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

template<class T = std::int64_t> class RangeAddRangeMin {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	using Node = range_add_min_detail::Node<T>;
	static constexpr unsigned branch = 8, bits = 3;
	static constexpr T infinity = std::numeric_limits<T>::max();
	static constexpr U umax = std::numeric_limits<U>::max();
	static constexpr U limit_ = U(infinity) / 3;
	std::vector<U, AlignedAllocator<U>> data_;
	std::array<std::size_t, (sizeof(int) * 8 + bits - 1) / bits + 1> offsets_{}, counts_{};
	int n_ = 0, minimum_pos_ = -1;
	unsigned height_ = 0;
	std::size_t width_ = 1;
	T root_ = infinity;
	U bound_ = 0;
	static T added(T value, U delta) { return std::bit_cast<T>(U(U(value) + delta)); }
	unsigned block_count(unsigned level, std::size_t index) const { return unsigned(std::min<std::size_t>(branch, counts_[level] - index * branch)); }
	template<bool Narrow>
	void propagate(unsigned level, std::size_t index, U change, bool negative) {
		for (; level < height_; ++level, index >>= bits) {
			if (!change) return;
			std::size_t block = index >> bits;
			change = Node::template change_one<Narrow>(data_.data() + offsets_[level] + block * branch, block_count(level, block), index & (branch - 1), change, negative);
		}
		root_ = added(root_, change);
	}
	template<bool Narrow>
	void update(std::size_t l, std::size_t r, U delta, bool negative) {
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
		if (a == b) {
			U change = first == last ? Node::template change_one<Narrow>(data_.data() + a * branch, block_count(0, a), first, delta, negative)
				: Node::template normalize<Narrow>(data_.data() + a * branch, block_count(0, a), negative, first, last + 1, delta);
			propagate<Narrow>(1, a, change, negative);
			return;
		}
		auto ends = Node::template normalize_boundaries<Narrow>(data_.data() + a * branch, data_.data() + b * branch, block_count(0, b), negative, first, last, delta, delta, delta);
		U left = ends.first, right = ends.second;
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			first = unsigned(l & (branch - 1));
			last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			U *values = data_.data() + offsets_[level];
			if (a == b) {
				U change;
				if (first + 1 == last) {
					change = Node::template change_one<Narrow>(values + a * branch, block_count(level, a), first, left, negative);
					change += Node::template change_one<Narrow>(values + a * branch, block_count(level, a), last, right, negative);
				} else change = Node::template normalize<Narrow>(values + a * branch, block_count(level, a), negative, first + 1, last, delta, first, left, last, right);
				propagate<Narrow>(level + 1, a, change, negative);
				return;
			}
			if (first + 1 < branch && last) {
				auto ends = Node::template normalize_boundaries<Narrow>(values + a * branch, values + b * branch, block_count(level, b), negative, first, last, delta, left, right);
				left = ends.first, right = ends.second;
			} else {
				left = first + 1 == branch ? Node::template change_one<Narrow>(values + a * branch, branch, first, left, negative)
					: Node::template normalize<Narrow>(values + a * branch, branch, negative, first + 1, branch, delta, first, left);
				right = !last ? Node::template change_one<Narrow>(values + b * branch, block_count(level, b), 0, right, negative)
					: Node::template normalize<Narrow>(values + b * branch, block_count(level, b), negative, 0, last, delta, last, right);
			}
		}
	}
	U restore(unsigned level, std::size_t index, U result) const {
		for (; level < height_; ++level, index >>= bits) result += data_[offsets_[level] + index];
		return result;
	}
	template<bool Narrow>
	U query(std::size_t l, std::size_t r) const {
		if constexpr (sizeof(T) < 8) if (!bound_) return 0;
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		if (a == b) {
			if (l == r) return restore(0, l, 0);
			return restore(1, a, Node::template minimum<Narrow>(data_.data() + a * branch, l & (branch - 1), (r & (branch - 1)) + 1));
		}
		U left = Node::template minimum<Narrow>(data_.data() + a * branch, l & (branch - 1), branch);
		U right = Node::template minimum<Narrow>(data_.data() + b * branch, 0, (r & (branch - 1)) + 1);
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			left += data_[offsets_[level] + l];
			right += data_[offsets_[level] + r];
			unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			const U *values = data_.data() + offsets_[level];
			if (a == b) {
				U result = std::min(left, right);
				if (first + 1 < last) result = std::min(result, Node::template minimum<Narrow>(values + a * branch, first + 1, last));
				return restore(level + 1, a, result);
			}
			if (first + 1 < branch) left = std::min(left, Node::template minimum<Narrow>(values + a * branch, first + 1, branch));
			if (last) right = std::min(right, Node::template minimum<Narrow>(values + b * branch, 0, last));
		}
	}
#if defined(__AVX2__) && !defined(__AVX512F__) && defined(__x86_64__)
	static __m256i query_block(const U* values, unsigned first, unsigned last) {
		return Node::template minimum_vector<true>(values, first, last, 2 * limit_);
	}
	U query_vector(std::size_t l, std::size_t r) const {
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		if (a == b) {
			if (l == r) return restore(0, l, 0);
			auto value = query_block(data_.data() + a * branch, l & (branch - 1), (r & (branch - 1)) + 1);
			U carry = restore(1, a, 0);
			return Node::reduce_min(value) + carry;
		}
		auto left = query_block(data_.data() + a * branch, l & (branch - 1), branch);
		auto right = query_block(data_.data() + b * branch, 0, (r & (branch - 1)) + 1);
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			left = _mm256_add_epi64(left, _mm256_set1_epi64x(std::int64_t(data_[offsets_[level] + l])));
			right = _mm256_add_epi64(right, _mm256_set1_epi64x(std::int64_t(data_[offsets_[level] + r])));
			unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			const U* values = data_.data() + offsets_[level];
			if (a == b) {
				auto result = Node::min_signed(left, right);
				if (first + 1 < last) result = Node::min_signed(result, query_block(values + a * branch, first + 1, last));
				U carry = restore(level + 1, a, 0);
				return Node::reduce_min(result) + carry;
			}
			if (first + 1 < branch) left = Node::min_signed(left, query_block(values + a * branch, first + 1, branch));
			if (last) right = Node::min_signed(right, query_block(values + b * branch, 0, last));
		}
	}
#endif
	template<bool Narrow>
	U assign(unsigned level, std::size_t index, std::size_t pos, U delta, bool negative) {
		unsigned child = unsigned((pos >> (bits * level)) & (branch - 1));
		U change = level ? assign<Narrow>(level - 1, index * branch + child, pos, delta, negative) : delta;
		unsigned count = unsigned(std::min<std::size_t>(branch, counts_[level] - index * branch));
		return Node::template change_one<Narrow>(data_.data() + offsets_[level] + index * branch, count, child, change, negative);
	}
	void collect(unsigned level, std::size_t index, std::size_t start, std::size_t width, U base, std::vector<T> &result) const {
		const U *values = data_.data() + offsets_[level] + index * branch;
		if (!level) {
			unsigned count = unsigned(std::min<std::size_t>(branch, std::size_t(n_) - start));
			Node::materialize(values, result.data() + start, count, base);
			return;
		}
		std::size_t part = width / branch;
		for (unsigned i = 0; i < branch && start + i * part < std::size_t(n_); ++i)
			collect(level - 1, index * branch + i, start + i * part, part, U(base + values[i]), result);
	}
	void padding(U value) {
		for (unsigned level = 0; level < height_; ++level) {
			std::size_t count = counts_[level], remainder = count & (branch - 1);
			if (remainder) {
				U* first = data_.data() + offsets_[level] + count;
				std::fill(first, first + branch - remainder, value);
			}
		}
	}
	void track(U amount) {
		if constexpr (sizeof(T) == 8) {
			bool narrow = bound_ < limit_;
			bound_ = narrow && amount < limit_ - bound_ ? U(bound_ + amount) : umax;
			if (narrow && bound_ == umax) padding(umax);
		} else bound_ = 1;
	}
	void allocate(int n, U value) {
		n_ = n;
		if (!n) return;
		std::size_t total = 0, count = n;
		for (unsigned level = 0;; ++level) {
			std::size_t groups = (count + branch - 1) / branch;
			offsets_[level] = total;
			counts_[level] = count;
			total += groups * branch;
			if (width_ > std::numeric_limits<std::size_t>::max() / branch) throw std::length_error("RangeAddRangeMin size");
			width_ *= branch;
			if (groups == 1) { height_ = level + 1; break; }
			count = groups;
		}
		data_.resize(total, value);
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
public:
	using value_type = T;
	RangeAddRangeMin() = default;
	explicit RangeAddRangeMin(int n, T value = 0) {
		assert(n >= 0);
		allocate(n, 0);
		if (n) { root_ = value; minimum_pos_ = n / 2; padding(sizeof(T) == 8 ? 2 * limit_ : umax); }
	}
	explicit RangeAddRangeMin(const std::vector<T> &values) { build(values); }
	RangeAddRangeMin(const RangeAddRangeMin &) = default;
	RangeAddRangeMin(RangeAddRangeMin &&other) noexcept { swap(other); }
	RangeAddRangeMin &operator=(RangeAddRangeMin other) noexcept { swap(other); return *this; }
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		auto extremes = std::minmax_element(values.begin(), values.end());
		if (!values.empty() && *extremes.first == *extremes.second) {
			RangeAddRangeMin next(int(values.size()), *extremes.first);
			swap(next);
			return;
		}
		RangeAddRangeMin next;
		next.allocate(int(values.size()), umax);
		if (!values.empty()) {
			std::copy(values.begin(), values.end(), next.data_.begin());
			for (unsigned level = 0; level < next.height_; ++level) {
				std::size_t groups = (next.counts_[level] + branch - 1) / branch;
				for (std::size_t group = 0; group < groups; ++group) {
					T minimum = Node::build(next.data_.data() + next.offsets_[level] + group * branch, next.block_count(level, group));
					if (level + 1 == next.height_) next.root_ = minimum;
					else next.data_[next.offsets_[level + 1] + group] = U(minimum);
				}
			}
			next.minimum_pos_ = values[values.size() / 2] == next.root_ ? int(values.size() / 2) : int(extremes.first - values.begin());
			next.bound_ = U(U(*extremes.second) - U(next.root_));
			if constexpr (sizeof(T) == 8) if (next.bound_ < limit_) next.padding(2 * limit_);
		}
		swap(next);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	void add(int l, int r, T value) {
		check(l, r);
		if (l == r || !value) return;
		if (l == 0 && r == n_) { root_ = added(root_, U(value)); return; }
		T candidate = root_;
		if (l <= minimum_pos_ && minimum_pos_ < r) candidate = added(root_, U(value));
		if constexpr (sizeof(T) == 8) {
			track(value < 0 ? U(0) - U(value) : U(value));
			if (bound_ < limit_) update<true>(l, r, U(value), value < 0);
			else update<false>(l, r, U(value), value < 0);
		} else {
			bound_ = 1;
			update<false>(l, r, U(value), value < 0);
		}
		if (minimum_pos_ >= 0 && root_ != candidate) minimum_pos_ = -1;
	}
	void apply(int l, int r, T value) { add(l, r, value); }
	T prod(int l, int r) const {
		check(l, r);
		if (l == r) return infinity;
		if ((l == 0 && r == n_) || (l <= minimum_pos_ && minimum_pos_ < r)) return root_;
		if constexpr (sizeof(T) == 8) {
			if (U(bound_ - 1) < U(limit_ - 1)) {
#if defined(__AVX2__) && !defined(__AVX512F__) && defined(__x86_64__)
				return added(root_, query_vector(l, r));
#else
				return added(root_, query<true>(l, r));
#endif
			}
			if (!bound_) return root_;
		}
		return added(root_, query<false>(l, r));
	}
	T min(int l, int r) const { return prod(l, r); }
	T all_prod() const { return root_; }
	T all_min() const { return root_; }
	T get(int p) const { assert(0 <= p && p < n_); return added(root_, restore(0, p, 0)); }
	void set(int p, T value) {
		assert(0 <= p && p < n_);
		T old = get(p);
		if (value == old) { if (value == root_) minimum_pos_ = p; return; }
		track(value < old ? U(U(old) - U(value)) : U(U(value) - U(old)));
		U delta = U(U(value) - U(old)), change;
		if constexpr (sizeof(T) == 8) {
			change = bound_ < limit_ ? assign<true>(height_ - 1, 0, p, delta, value < old)
				: assign<false>(height_ - 1, 0, p, delta, value < old);
		} else change = assign<false>(height_ - 1, 0, p, delta, value < old);
		root_ = added(root_, change);
		if (value == root_) minimum_pos_ = p;
		else if (minimum_pos_ == p) minimum_pos_ = -1;
	}
	std::vector<T> values() const { std::vector<T> result(n_); if (n_) collect(height_ - 1, 0, 0, width_, U(root_), result); return result; }
	std::size_t memory_usage() const { return data_.capacity() * sizeof(U); }
	void swap(RangeAddRangeMin &other) noexcept {
		data_.swap(other.data_), offsets_.swap(other.offsets_), counts_.swap(other.counts_);
		std::swap(n_, other.n_), std::swap(minimum_pos_, other.minimum_pos_), std::swap(height_, other.height_), std::swap(width_, other.width_), std::swap(root_, other.root_), std::swap(bound_, other.bound_);
	}
	friend void swap(RangeAddRangeMin &a, RangeAddRangeMin &b) noexcept { a.swap(b); }
};

#endif
