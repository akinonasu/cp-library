#ifndef SLIDING_WINDOW_AGGREGATION_HPP
#define SLIDING_WINDOW_AGGREGATION_HPP

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

template<class S, auto op, auto e>
class SlidingWindowAggregation {
	struct Entry {
		S value, product;
		Entry(S value, S product) : value(std::move(value)), product(std::move(product)) {}
	};
	std::vector<Entry> left, right;
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
	SlidingWindowAggregation(const SlidingWindowAggregation &) = default;
	SlidingWindowAggregation(SlidingWindowAggregation &&other) noexcept { swap(other); }
	SlidingWindowAggregation &operator=(SlidingWindowAggregation other) noexcept { swap(other); return *this; }
	bool empty() const { return left.empty() && right.empty(); }
	size_type size() const { return left.size() + right.size(); }
	void push_back(S value) {
		S product = right.empty() ? value : op(right.back().product, value);
		right.emplace_back(std::move(value), std::move(product));
	}
	void pop_front() { assert(!empty()); if (left.empty()) transfer(); left.pop_back(); }
	const S &front() const { assert(!empty()); return left.empty() ? right.front().value : left.back().value; }
	const S &back() const { assert(!empty()); return right.empty() ? left.front().value : right.back().value; }
	S prod() const {
		if (left.empty()) return right.empty() ? e() : right.back().product;
		return right.empty() ? left.back().product : op(left.back().product, right.back().product);
	}
	void reserve(size_type n) { left.reserve(n); right.reserve(n); }
	void clear() { left.clear(); right.clear(); }
	void swap(SlidingWindowAggregation &other) noexcept { left.swap(other.left); right.swap(other.right); }
	friend void swap(SlidingWindowAggregation &a, SlidingWindowAggregation &b) noexcept { a.swap(b); }
};

#endif
