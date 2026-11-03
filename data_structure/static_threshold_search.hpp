#ifndef STATIC_THRESHOLD_SEARCH_HPP
#define STATIC_THRESHOLD_SEARCH_HPP

#include "../components/static_integer_successor.hpp"
#include <cassert>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <std::integral T = std::int64_t, bool Smaller = false, bool Reverse = false>
requires (sizeof(T) <= 8 && !std::is_same_v<T, bool>)
class StaticThresholdSearch {
	using U = std::make_unsigned_t<T>;
	static constexpr int bits = std::numeric_limits<U>::digits;
	static std::uint64_t key(T value) {
		U result = U(value);
		if constexpr (std::is_signed_v<T>) result ^= U(1) << (bits - 1);
		if constexpr (Smaller) result = U(~result);
		return result;
	}
	T value_at(int i) const { if constexpr (Reverse) return values_[std::size_t(size() - 1 - i)]; else return values_[i]; }
	int external(int i) const { if constexpr (Reverse) return i == size() ? size() : size() - 1 - i; else return i; }
	struct Chain { int begin, end, head, ancestors_begin, ancestors_end; std::uint64_t stride = 0; };
	std::vector<T> values_;
	std::vector<int> parent_, chain_, order_, ancestors_;
	std::vector<Chain> chains_;
	std::vector<std::uint64_t> keys_, blocks_;
	static_integer_successor_detail::Index index_;
	std::uint64_t maximum_key_ = 0;
	template <bool Strict> [[gnu::noinline]] int search_far(int l, std::uint64_t target) const {
		auto accepts = [&](int v) { return Strict ? key(value_at(v)) > target : key(value_at(v)) >= target; };
		const auto &current = chains_[chain_[l]];
		int group = chain_[l];
		if (!accepts(current.head)) {
			if (!accepts(chains_[ancestors_[current.ancestors_end - 1]].head)) return size();
			int begin = current.ancestors_begin + 1, end = current.ancestors_end;
			while (begin < end) { int mid = begin + (end - begin) / 2; if (accepts(chains_[ancestors_[mid]].head)) end = mid; else begin = mid + 1; }
			group = ancestors_[begin];
			int entry = parent_[chains_[ancestors_[begin - 1]].head];
			if (accepts(entry)) return entry;
		}
		const auto &c = chains_[group];
		int position = index_.template lower_bound<Strict>(group, std::span<const std::uint64_t>(keys_).subspan(c.begin, c.end - c.begin), bits, c.stride, target);
		return order_[c.begin + position];
	}
	template <bool Strict> int search(int l, T x) const {
		assert(0 <= l && l <= size());
		if (l == size()) return size();
		std::uint64_t target = key(x);
		if (Strict ? maximum_key_ <= target : maximum_key_ < target) return size();
		if (chains_.size() == 1) {
			if (Strict ? key(value_at(l)) > target : key(value_at(l)) >= target) return l;
			int position = index_.template lower_bound<Strict>(0, keys_, bits, chains_[0].stride, target);
			return order_[position];
		}
		int end = int(std::min<std::size_t>(values_.size(), (std::size_t(unsigned(l) / 32) + 1) * 32));
		for (int i = l; i < end; ++i) if (Strict ? key(value_at(i)) > target : key(value_at(i)) >= target) return i;
		for (int count = 0; count < 8 && end < size(); ++count) {
			int begin = end;
			end += std::min(32, size() - end);
			auto maximum = blocks_[unsigned(begin) / 32];
			if (Strict ? maximum > target : maximum >= target)
				for (int i = begin; i < end; ++i) if (Strict ? key(value_at(i)) > target : key(value_at(i)) >= target) return i;
		}
		if (end == size()) return size();
		return search_far<Strict>(end - 1, target);
	}
	void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }
  public:
	StaticThresholdSearch() = default;
	explicit StaticThresholdSearch(std::vector<T> values) { build(std::move(values)); }
	StaticThresholdSearch(const StaticThresholdSearch &) = default;
	StaticThresholdSearch &operator=(const StaticThresholdSearch &other) { if (this != &other) { auto copy = other; swap(copy); } return *this; }
	StaticThresholdSearch(StaticThresholdSearch &&other) noexcept { swap(other); }
	StaticThresholdSearch &operator=(StaticThresholdSearch &&other) noexcept { if (this != &other) { StaticThresholdSearch moved(std::move(other)); swap(moved); } return *this; }
	void swap(StaticThresholdSearch &other) noexcept {
		values_.swap(other.values_); parent_.swap(other.parent_); chain_.swap(other.chain_); order_.swap(other.order_); ancestors_.swap(other.ancestors_); chains_.swap(other.chains_); keys_.swap(other.keys_); blocks_.swap(other.blocks_); std::swap(index_, other.index_); std::swap(maximum_key_, other.maximum_key_);
	}
	void build(std::vector<T> values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		StaticThresholdSearch next; next.values_ = std::move(values);
		int n = next.size(); next.chains_.reserve(n); next.ancestors_.reserve(n); next.blocks_.assign((std::size_t(n) + 31) / 32, 0);
		for (int i = 0; i < n; ++i) next.blocks_[unsigned(i) / 32] = std::max(next.blocks_[unsigned(i) / 32], key(next.value_at(i)));
		next.parent_.assign(n, -1); next.chain_.resize(n);
		std::vector<int> stack, sizes(n, 1), heavy(n, -1), count; stack.reserve(n); count.reserve(n);
		for (int i = n; i-- > 0;) {
			while (!stack.empty() && key(next.value_at(stack.back())) <= key(next.value_at(i))) stack.pop_back();
			if (!stack.empty()) next.parent_[i] = stack.back();
			stack.push_back(i);
		}
		if (n) next.maximum_key_ = key(next.value_at(stack.front()));
		for (int i = 0; i < n; ++i) { int p = next.parent_[i]; if (p != -1) { sizes[p] += sizes[i]; if (heavy[p] == -1 || sizes[heavy[p]] < sizes[i]) heavy[p] = i; } }
		for (int i = n; i-- > 0;) {
			int p = next.parent_[i];
			if (p != -1 && heavy[p] == i) next.chain_[i] = next.chain_[p];
			else {
				int id = int(next.chains_.size());
				std::size_t needed = p == -1 ? 1 : std::size_t(next.chains_[next.chain_[p]].ancestors_end - next.chains_[next.chain_[p]].ancestors_begin) + 1;
				if (next.ancestors_.size() > std::size_t(std::numeric_limits<int>::max()) - needed) throw std::length_error("StaticThresholdSearch ancestors");
				int begin = int(next.ancestors_.size());
				next.ancestors_.push_back(id);
				if (p != -1) { auto c = next.chains_[next.chain_[p]]; for (int j = c.ancestors_begin; j < c.ancestors_end; ++j) { int a = next.ancestors_[j]; next.ancestors_.push_back(a); } }
				next.chains_.push_back({0, 0, i, begin, int(next.ancestors_.size()), 0});
				count.push_back(0); next.chain_[i] = id;
			}
			++count[next.chain_[i]];
		}
		int offset = 0;
		for (std::size_t i = 0; i < count.size(); ++i) { next.chains_[i].begin = offset; offset += count[i]; next.chains_[i].end = offset; count[i] = next.chains_[i].begin; }
		next.order_.resize(n); next.keys_.resize(n);
		for (int i = 0; i < n; ++i) { int p = count[next.chain_[i]]++; next.order_[p] = i; next.keys_[p] = key(next.value_at(i)); }
		for (unsigned i = 0; i < next.chains_.size(); ++i) { auto &c = next.chains_[i]; c.stride = next.index_.add(i, std::span<const std::uint64_t>(next.keys_).subspan(c.begin, c.end - c.begin), bits); }
		swap(next);
	}
	void clear() { StaticThresholdSearch next; swap(next); }
	int size() const { return int(values_.size()); }
	bool empty() const { return values_.empty(); }
	std::span<const T> values() const { return values_; }
	const T &operator[](int i) const { assert(0 <= i && i < size()); return values_[i]; }
	int next(int l, T x) const requires (!Reverse) { return search<false>(l, x); }
	int next_strict(int l, T x) const requires (!Reverse) { return search<true>(l, x); }
	int first(int l, int r, T x) const requires (!Reverse) { check_range(l, r); if (l == r) return size(); int p = next(l, x); return p < r ? p : size(); }
	int first_strict(int l, int r, T x) const requires (!Reverse) { check_range(l, r); if (l == r) return size(); int p = next_strict(l, x); return p < r ? p : size(); }
	int next_ge(int l, T x) const requires (!Smaller && !Reverse) { return next(l, x); }
	int next_gt(int l, T x) const requires (!Smaller && !Reverse) { return next_strict(l, x); }
	int next_le(int l, T x) const requires (Smaller && !Reverse) { return next(l, x); }
	int next_lt(int l, T x) const requires (Smaller && !Reverse) { return next_strict(l, x); }
	int prev(int r, T x) const requires Reverse { assert(0 <= r && r <= size()); return external(search<false>(size() - r, x)); }
	int prev_strict(int r, T x) const requires Reverse { assert(0 <= r && r <= size()); return external(search<true>(size() - r, x)); }
	int last(int l, int r, T x) const requires Reverse { check_range(l, r); if (l == r) return size(); int p = prev(r, x); return p != size() && p >= l ? p : size(); }
	int last_strict(int l, int r, T x) const requires Reverse { check_range(l, r); if (l == r) return size(); int p = prev_strict(r, x); return p != size() && p >= l ? p : size(); }
	int prev_ge(int r, T x) const requires (!Smaller && Reverse) { return prev(r, x); }
	int prev_gt(int r, T x) const requires (!Smaller && Reverse) { return prev_strict(r, x); }
	int prev_le(int r, T x) const requires (Smaller && Reverse) { return prev(r, x); }
	int prev_lt(int r, T x) const requires (Smaller && Reverse) { return prev_strict(r, x); }
	int next_record(int i) const { assert(0 <= i && i < size()); if constexpr (Reverse) i = size() - 1 - i; int p = parent_[i]; return p == -1 ? size() : external(p); }

};

#endif
