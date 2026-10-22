#ifndef TOP_K_SUM_HPP
#define TOP_K_SUM_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>, class Compare = std::greater<T>>
class TopKSum {
	[[no_unique_address]] Compare order;
	std::multiset<T, Compare> selected, rest;
	std::size_t limit;
	Sum selected_sum = Sum(0);

	bool choose(const T &value) const {
		return (rest.empty() && selected.size() < limit) ||
			(!selected.empty() && order(value, *std::prev(selected.end())));
	}

	void balance() {
		const auto target = (std::min)(limit, size());
		while (selected.size() > target) {
			auto it = std::prev(selected.end());
			selected_sum -= static_cast<Sum>(*it);
			rest.insert(rest.begin(), selected.extract(it));
		}
		while (selected.size() < target) {
			auto it = rest.begin();
			selected_sum += static_cast<Sum>(*it);
			selected.insert(selected.end(), rest.extract(it));
		}
	}

public:
	using value_type = T;
	using sum_type = Sum;
	using size_type = std::size_t;
	using value_compare = Compare;

	explicit TopKSum(size_type k = 0, Compare compare = Compare{})
		: order(compare), selected(compare), rest(compare), limit(k) {}
	TopKSum(size_type k, std::vector<T> values, Compare compare = Compare{})
		: TopKSum(k, compare) { assign(std::move(values)); }
	TopKSum(const TopKSum &) = default;
	TopKSum(TopKSum &&other) noexcept(std::is_nothrow_copy_constructible_v<Compare> &&
		std::is_nothrow_constructible_v<Sum, int> && std::is_nothrow_swappable_v<Compare> && std::is_nothrow_swappable_v<Sum>)
		: TopKSum(other.limit, other.order) { swap(other); }
	TopKSum &operator=(TopKSum other) noexcept(std::is_nothrow_swappable_v<Compare> && std::is_nothrow_swappable_v<Sum>) {
		swap(other);
		return *this;
	}

	void assign(std::vector<T> values) {
		std::sort(values.begin(), values.end(), order);
		TopKSum result(limit, order);
		const auto count = (std::min)(limit, values.size());
		for (size_type i = 0; i < count; ++i) {
			auto it = result.selected.insert(result.selected.end(), std::move(values[i]));
			result.selected_sum += static_cast<Sum>(*it);
		}
		for (size_type i = count; i < values.size(); ++i)
			result.rest.insert(result.rest.end(), std::move(values[i]));
		swap(result);
	}

	void insert(const T &value) {
		if (choose(value)) {
			selected.insert(value);
			selected_sum += static_cast<Sum>(value);
		} else rest.insert(value);
		balance();
	}

	bool erase(const T &value) {
		auto it = selected.find(value);
		if (it != selected.end()) {
			selected_sum -= static_cast<Sum>(*it);
			selected.erase(it);
			balance();
			return true;
		}
		it = rest.find(value);
		if (it == rest.end()) return false;
		rest.erase(it);
		return true;
	}

	bool replace(const T &old_value, const T &new_value) {
		auto it = selected.find(old_value);
		auto *source = &selected;
		if (it == selected.end()) {
			source = &rest;
			it = rest.find(old_value);
			if (it == rest.end()) return false;
		} else selected_sum -= static_cast<Sum>(*it);
		auto node = source->extract(it);
		node.value() = new_value;
		if (choose(new_value)) {
			selected.insert(std::move(node));
			selected_sum += static_cast<Sum>(new_value);
		} else rest.insert(std::move(node));
		balance();
		return true;
	}

	void set_k(size_type k) { limit = k; balance(); }
	size_type k() const { return limit; }
	size_type size() const { return selected.size() + rest.size(); }
	size_type selected_size() const { return selected.size(); }
	bool empty() const { return selected.empty() && rest.empty(); }
	Sum sum() const { return selected_sum; }
	bool contains(const T &value) const { return selected.find(value) != selected.end() || rest.find(value) != rest.end(); }
	size_type count(const T &value) const { return selected.count(value) + rest.count(value); }
	void clear() { selected.clear(); rest.clear(); selected_sum = Sum(0); }
	void swap(TopKSum &other) noexcept(std::is_nothrow_swappable_v<Compare> && std::is_nothrow_swappable_v<Sum>) {
		using std::swap;
		swap(order, other.order);
		selected.swap(other.selected);
		rest.swap(other.rest);
		swap(limit, other.limit);
		swap(selected_sum, other.selected_sum);
	}
	friend void swap(TopKSum &a, TopKSum &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using LargestKSum = TopKSum<T, Sum, std::greater<T>>;

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using SmallestKSum = TopKSum<T, Sum, std::less<T>>;

#endif
