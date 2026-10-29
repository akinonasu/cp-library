#ifndef SLIDING_WINDOW_AGGREGATION_HPP
#define SLIDING_WINDOW_AGGREGATION_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

template<class S, auto op, auto e>
class SlidingWindowAggregation {
	struct Entry {
		S value, product;
		Entry(S value, S product) : value(std::move(value)), product(std::move(product)) {}
	};
	static constexpr bool inplace = std::is_trivially_destructible_v<S> && std::is_nothrow_move_constructible_v<S> && std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>;
	std::vector<Entry> left, right;
	std::size_t first = 0, middle = 0;
	[[gnu::noinline]] void transfer_inplace() {
		S saved = right.back().product;
		try {
			if (right.size() - first > 1) {
				right.back().product = right.back().value;
				for (std::size_t i = right.size() - 1; i > first + 1;) {
					--i;
					right[i].product = op(right[i].value, right[i + 1].product);
				}
			}
		} catch (...) {
			right.back().product = std::move(saved);
			throw;
		}
		middle = right.size();
	}
	void transfer() {
		left.reserve(right.size());
		try {
			for (auto it = right.rbegin(); it != right.rend(); ++it) {
				S product = left.empty() ? it->value : op(it->value, left.back().product);
				left.emplace_back(it->value, std::move(product));
			}
		} catch (...) {
			left.clear();
			throw;
		}
		right.clear();
	}
public:
	using value_type = S;
	using size_type = std::size_t;
	SlidingWindowAggregation() = default;
	explicit SlidingWindowAggregation(const std::vector<S> &values) { reserve(values.size()); for (const auto &x : values) push_back(x); }
	SlidingWindowAggregation(const SlidingWindowAggregation &other) {
		if constexpr (inplace) {
			right.reserve(other.size());
			for (std::size_t i = other.first; i < other.right.size(); ++i) right.push_back(other.right[i]);
			middle = other.middle - other.first;
		} else left = other.left, right = other.right;
	}
	SlidingWindowAggregation(SlidingWindowAggregation &&other) noexcept { swap(other); }
	SlidingWindowAggregation &operator=(SlidingWindowAggregation other) noexcept { swap(other); return *this; }
	bool empty() const {
		if constexpr (inplace) return first == right.size();
		else return left.empty() && right.empty();
	}
	size_type size() const {
		if constexpr (inplace) return right.size() - first;
		else return left.size() + right.size();
	}
	void push_back(S value) {
		bool fresh;
		if constexpr (inplace) fresh = middle == right.size();
		else fresh = right.empty();
		S product = fresh ? value : op(right.back().product, value);
		if constexpr (inplace) {
			if (first && right.size() == right.capacity() && first >= right.size() / 2) {
				std::move(right.begin() + first, right.end(), right.begin());
				const auto length = right.size() - first;
				while (right.size() > length) right.pop_back();
				middle -= first;
				first = 0;
			}
		}
		right.emplace_back(std::move(value), std::move(product));
	}
	void pop_front() {
		assert(!empty());
		if constexpr (inplace) {
			if (first == middle) transfer_inplace();
			if (++first == right.size()) clear();
		} else {
			if (left.empty()) transfer();
			left.pop_back();
		}
	}
	const S &front() const {
		assert(!empty());
		if constexpr (inplace) return right[first].value;
		else return left.empty() ? right.front().value : left.back().value;
	}
	const S &back() const {
		assert(!empty());
		if constexpr (inplace) return right.back().value;
		else return right.empty() ? left.front().value : right.back().value;
	}
	S prod() const {
		if constexpr (inplace) {
			if (first == middle) return empty() ? e() : right.back().product;
			return middle == right.size() ? right[first].product : op(right[first].product, right.back().product);
		} else {
			if (left.empty()) return right.empty() ? e() : right.back().product;
			return right.empty() ? left.back().product : op(left.back().product, right.back().product);
		}
	}
	template<class T, class Mapping>
	T apply(T value, Mapping mapping) const {
		if constexpr (inplace) {
			if (first < middle) value = mapping(right[first].product, std::move(value));
			if (middle < right.size()) value = mapping(right.back().product, std::move(value));
		} else {
			if (!left.empty()) value = mapping(left.back().product, std::move(value));
			if (!right.empty()) value = mapping(right.back().product, std::move(value));
		}
		return value;
	}
	template<class T>
	T apply(T value) const {
		return apply(std::move(value), [](const S &f, T x) { return f(std::move(x)); });
	}
	void reserve(size_type n) { if constexpr (!inplace) left.reserve(n); right.reserve(n); }
	void clear() { left.clear(); right.clear(); first = middle = 0; }
	void swap(SlidingWindowAggregation &other) noexcept { left.swap(other.left); right.swap(other.right); std::swap(first, other.first); std::swap(middle, other.middle); }
	friend void swap(SlidingWindowAggregation &a, SlidingWindowAggregation &b) noexcept { a.swap(b); }
};

#endif
