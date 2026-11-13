#ifndef TOP_K_SUM_HPP
#define TOP_K_SUM_HPP

#include "../components/radix_sort.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>, class Compare = std::greater<T>>
class TopKSum {
public:
	using value_type = T;
	using sum_type = Sum;
	using size_type = std::int64_t;
	using value_compare = Compare;

private:
	static constexpr bool descending = std::is_same_v<Compare, std::greater<T>> || std::is_same_v<Compare, std::greater<void>>;
	static constexpr bool widen = std::is_integral_v<Sum> && sizeof(Sum) <= 8;
	using Aggregate = std::conditional_t<widen, std::conditional_t<std::is_signed_v<Sum>, __int128_t, __uint128_t>, Sum>;
	static Aggregate contribution(const T &value, size_type copies) {
		auto converted = static_cast<Aggregate>(static_cast<Sum>(value));
		return copies == 1 ? converted : converted * copies;
	}
	struct Node;
	using Pointer = std::unique_ptr<Node>;
	struct Node {
		T value;
		Aggregate total;
		Pointer left, right;
		size_type copies, size;
		int height = 1;
		Node(const T &value, size_type copies) : value(value), total(contribution(value, copies)), copies(copies), size(copies) {}
	};
	[[no_unique_address]] Compare order;
	Pointer root;

	static size_type length(const Pointer &p) { return p ? p->size : 0; }
	static int height(const Pointer &p) { return p ? p->height : 0; }
	static void pull(Node &node) {
		Aggregate total = contribution(node.value, node.copies);
		if (node.left) total += node.left->total;
		if (node.right) total += node.right->total;
		node.total = std::move(total);
		node.size = length(node.left) + node.copies + length(node.right);
		node.height = (std::max)(height(node.left), height(node.right)) + 1;
	}
	static Pointer rotate_left(Pointer p) {
		auto next = std::move(p->right);
		p->right = std::move(next->left);
		pull(*p);
		next->left = std::move(p);
		pull(*next);
		return next;
	}
	static Pointer rotate_right(Pointer p) {
		auto next = std::move(p->left);
		p->left = std::move(next->right);
		pull(*p);
		next->right = std::move(p);
		pull(*next);
		return next;
	}
	static Pointer balance(Pointer p) {
		pull(*p);
		if (height(p->left) > height(p->right) + 1) {
			if (height(p->left->left) < height(p->left->right)) p->left = rotate_left(std::move(p->left));
			return rotate_right(std::move(p));
		}
		if (height(p->right) > height(p->left) + 1) {
			if (height(p->right->right) < height(p->right->left)) p->right = rotate_right(std::move(p->right));
			return rotate_left(std::move(p));
		}
		return p;
	}
	void insert_node(Pointer &p, const T &value, size_type copies) {
		if (!p) { p = std::make_unique<Node>(value, copies); return; }
		if (order(value, p->value)) insert_node(p->left, value, copies);
		else if (order(p->value, value)) insert_node(p->right, value, copies);
		else p->copies += copies;
		p = balance(std::move(p));
	}
	static Pointer detach_first(Pointer &p) {
		if (!p->left) {
			auto first = std::move(p);
			p = std::move(first->right);
			return first;
		}
		auto first = detach_first(p->left);
		p = balance(std::move(p));
		return first;
	}
	void erase_node(Pointer &p, const T &value, size_type copies, size_type &removed) {
		if (!p) return;
		if (order(value, p->value)) erase_node(p->left, value, copies, removed);
		else if (order(p->value, value)) erase_node(p->right, value, copies, removed);
		else {
			removed = (std::min)(copies, p->copies);
			if (removed < p->copies) p->copies -= removed;
			else {
				if (!p->left || !p->right) {
					auto next = p->left ? std::move(p->left) : std::move(p->right);
					p = std::move(next);
					return;
				}
				auto next = detach_first(p->right);
				next->left = std::move(p->left); next->right = std::move(p->right);
				p = std::move(next);
			}
		}
		if (removed) p = balance(std::move(p));
	}
	static Pointer build(const std::vector<T> &values, const std::vector<size_type> &counts, std::size_t first, std::size_t last) {
		if (first == last) return nullptr;
		std::size_t middle = first + (last - first) / 2;
		auto p = std::make_unique<Node>(values[middle], counts[middle]);
		p->left = build(values, counts, first, middle); p->right = build(values, counts, middle + 1, last);
		pull(*p);
		return p;
	}
	static Pointer clone(const Pointer &p) {
		if (!p) return nullptr;
		auto next = std::make_unique<Node>(p->value, p->copies);
		next->left = clone(p->left); next->right = clone(p->right);
		next->total = p->total; next->size = p->size; next->height = p->height;
		return next;
	}
	static Sum result(const Aggregate &value) {
		if constexpr (widen) assert(static_cast<Aggregate>((std::numeric_limits<Sum>::min)()) <= value && value <= static_cast<Aggregate>((std::numeric_limits<Sum>::max)()));
		return static_cast<Sum>(value);
	}
	template<bool Reverse> T kth_impl(size_type k) const {
		assert(0 <= k && k < size());
		auto p = root.get();
		for (;;) {
			const auto &first = Reverse ? p->right : p->left;
			size_type before = length(first);
			if (k < before) p = first.get();
			else if (k < before + p->copies) return p->value;
			else { k -= before + p->copies; p = (Reverse ? p->left : p->right).get(); }
		}
	}
	template<bool Reverse> Sum sum_impl(size_type k) const {
		assert(k >= 0);
		Aggregate answer(0);
		auto p = root.get();
		while (p && k) {
			if (k >= p->size) { answer += p->total; break; }
			const auto &first = Reverse ? p->right : p->left;
			size_type before = length(first);
			if (k <= before) p = first.get();
			else {
				if (first) answer += first->total;
				size_type take = (std::min)(k - before, p->copies);
				answer += contribution(p->value, take);
				k -= before + take;
				p = (Reverse ? p->left : p->right).get();
			}
		}
		return result(answer);
	}
	template<bool Strict> std::optional<T> bound(const T &value) const {
		auto p = root.get();
		const Node *candidate = nullptr;
		while (p) {
			bool found;
			if constexpr (Strict) found = descending ? order(p->value, value) : order(value, p->value);
			else found = descending ? !order(value, p->value) : !order(p->value, value);
			if (found) { candidate = p; p = (descending ? p->right : p->left).get(); }
			else p = (descending ? p->left : p->right).get();
		}
		return candidate ? std::optional<T>(candidate->value) : std::nullopt;
	}

public:
	explicit TopKSum(Compare compare = Compare{}) : order(std::move(compare)) {}
	explicit TopKSum(std::vector<T> values, Compare compare = Compare{}) : TopKSum(std::move(compare)) { assign(std::move(values)); }
	TopKSum(std::initializer_list<T> values, Compare compare = Compare{}) : TopKSum(std::vector<T>(values), std::move(compare)) {}
	TopKSum(const TopKSum &other) : order(other.order), root(clone(other.root)) {}
	TopKSum(TopKSum &&other) noexcept(std::is_nothrow_copy_constructible_v<Compare>) : order(other.order), root(std::move(other.root)) {}
	TopKSum &operator=(TopKSum other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		swap(other);
		return *this;
	}
	void assign(std::vector<T> values) {
		assert(values.size() <= std::size_t((std::numeric_limits<size_type>::max)()));
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8
			&& (descending || std::is_same_v<Compare, std::less<T>> || std::is_same_v<Compare, std::less<void>>)) {
			radix_sort_detail::sort_by_key(values, [](T value) { return value; });
			if constexpr (descending) std::reverse(values.begin(), values.end());
		} else std::sort(values.begin(), values.end(), order);
		std::vector<size_type> counts;
		std::size_t distinct = 0;
		for (std::size_t i = 0; i < values.size(); ++i) {
			if (!distinct || order(values[distinct - 1], values[i])) {
				if (distinct != i) values[distinct] = std::move(values[i]);
				++distinct;
				counts.push_back(1);
			} else ++counts.back();
		}
		TopKSum next(order);
		next.root = build(values, counts, 0, distinct);
		swap(next);
	}
	void reserve([[maybe_unused]] std::size_t distinct) {
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
	}
	void insert(const T &value, size_type copies = 1) {
		assert(copies >= 0 && copies <= (std::numeric_limits<size_type>::max)() - size());
		if (copies) insert_node(root, value, copies);
	}
	bool erase(const T &value) { return erase(value, 1) != 0; }
	size_type erase(const T &value, size_type copies) {
		assert(copies >= 0);
		size_type removed = 0;
		if (copies) erase_node(root, value, copies, removed);
		return removed;
	}
	size_type erase_all(const T &value) { return erase(value, (std::numeric_limits<size_type>::max)()); }
	bool replace(const T &old_value, const T &new_value) {
		if (!erase(old_value)) return false;
		insert(new_value);
		return true;
	}
	size_type size() const { return length(root); }
	bool empty() const { return !root; }
	T kth(size_type k) const { return kth_impl<false>(k); }
	T kth_smallest(size_type k) const { return kth_impl<descending>(k); }
	T kth_largest(size_type k) const { return kth_impl<!descending>(k); }
	Sum sum(size_type k) const { return sum_impl<false>(k); }
	Sum sum_smallest(size_type k) const { return sum_impl<descending>(k); }
	Sum sum_largest(size_type k) const { return sum_impl<!descending>(k); }
	std::optional<T> lower_bound(const T &value) const { return bound<false>(value); }
	std::optional<T> upper_bound(const T &value) const { return bound<true>(value); }
	Sum sum_all() const { return root ? result(root->total) : Sum(0); }
	size_type order_of_key(const T &value) const {
		size_type answer = 0;
		auto p = root.get();
		while (p) {
			if (order(p->value, value)) { answer += length(p->left) + p->copies; p = p->right.get(); }
			else p = p->left.get();
		}
		return answer;
	}
	size_type count(const T &value) const {
		auto p = root.get();
		while (p) {
			if (order(value, p->value)) p = p->left.get();
			else if (order(p->value, value)) p = p->right.get();
			else return p->copies;
		}
		return 0;
	}
	bool contains(const T &value) const { return count(value) != 0; }
	void clear() { root.reset(); }
	void swap(TopKSum &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		using std::swap;
		swap(order, other.order); root.swap(other.root);
	}
	friend void swap(TopKSum &a, TopKSum &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using LargestKSum = TopKSum<T, Sum, std::greater<T>>;

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using SmallestKSum = TopKSum<T, Sum, std::less<T>>;

#endif
