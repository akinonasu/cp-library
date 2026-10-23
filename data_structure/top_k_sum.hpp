#ifndef TOP_K_SUM_HPP
#define TOP_K_SUM_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include "../components/ordered_multiset_btree.hpp"

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
	static constexpr bool blocked = std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8
		&& (std::is_integral_v<Sum> || std::is_same_v<Sum, __int128_t> || std::is_same_v<Sum, __uint128_t>)
		&& (descending || std::is_same_v<Compare, std::less<T>> || std::is_same_v<Compare, std::less<void>>);
	struct EmptyBackend {};
	using Backend = std::conditional_t<blocked, ordered_multiset_detail::BTree<T, Sum, Aggregate>, EmptyBackend>;
	static Aggregate contribution(const T &value, size_type copies) {
		auto converted = static_cast<Aggregate>(static_cast<Sum>(value));
		return copies == 1 ? converted : converted * copies;
	}
	struct Node {
		Aggregate total;
		std::optional<T> value;
		size_type copies, size;
		int left = -1, right = -1, height = 1;
		Node(const T &value, size_type copies) : total(contribution(value, copies)), value(value), copies(copies), size(copies) {}
	};
	[[no_unique_address]] Compare order;
	[[no_unique_address]] Backend backend;
	std::vector<Node> nodes;
	int root = -1, free = -1;

	size_type length(int p) const { return p < 0 ? 0 : nodes[p].size; }
	int height(int p) const { return p < 0 ? 0 : nodes[p].height; }
	void pull(int p) {
		auto &node = nodes[p];
		Aggregate total = contribution(*node.value, node.copies);
		if (node.left >= 0) total += nodes[node.left].total;
		if (node.right >= 0) total += nodes[node.right].total;
		node.total = std::move(total);
		node.size = length(node.left) + node.copies + length(node.right);
		node.height = (std::max)(height(node.left), height(node.right)) + 1;
	}
	int rotate_left(int p) {
		int next = nodes[p].right;
		nodes[p].right = nodes[next].left;
		nodes[next].left = p;
		pull(p); pull(next);
		return next;
	}
	int rotate_right(int p) {
		int next = nodes[p].left;
		nodes[p].left = nodes[next].right;
		nodes[next].right = p;
		pull(p); pull(next);
		return next;
	}
	int balance(int p) {
		pull(p);
		int left = nodes[p].left, right = nodes[p].right;
		if (height(left) > height(right) + 1) {
			if (height(nodes[left].left) < height(nodes[left].right)) nodes[p].left = rotate_left(left);
			return rotate_right(p);
		}
		if (height(right) > height(left) + 1) {
			if (height(nodes[right].right) < height(nodes[right].left)) nodes[p].right = rotate_right(right);
			return rotate_left(p);
		}
		return p;
	}
	int make_node(const T &value, size_type copies) {
		if (free >= 0) {
			int p = free, next = nodes[p].left;
			nodes[p] = Node(value, copies);
			free = next;
			return p;
		}
		assert(nodes.size() < std::size_t((std::numeric_limits<int>::max)()));
		nodes.emplace_back(value, copies);
		return static_cast<int>(nodes.size()) - 1;
	}
	void release(int p) {
		nodes[p].value.reset();
		nodes[p].total = Aggregate(0);
		nodes[p].left = free;
		free = p;
	}
	int insert_node(int p, const T &value, size_type copies) {
		if (p < 0) return make_node(value, copies);
		if (order(value, *nodes[p].value)) {
			int next = insert_node(nodes[p].left, value, copies);
			nodes[p].left = next;
		} else if (order(*nodes[p].value, value)) {
			int next = insert_node(nodes[p].right, value, copies);
			nodes[p].right = next;
		} else nodes[p].copies += copies;
		return balance(p);
	}
	int detach_first(int p, int &first) {
		if (nodes[p].left < 0) {
			first = p;
			return nodes[p].right;
		}
		nodes[p].left = detach_first(nodes[p].left, first);
		return balance(p);
	}
	int erase_node(int p, const T &value, size_type copies, size_type &removed) {
		if (p < 0) return -1;
		if (order(value, *nodes[p].value)) nodes[p].left = erase_node(nodes[p].left, value, copies, removed);
		else if (order(*nodes[p].value, value)) nodes[p].right = erase_node(nodes[p].right, value, copies, removed);
		else {
			removed = (std::min)(copies, nodes[p].copies);
			if (removed < nodes[p].copies) nodes[p].copies -= removed;
			else {
				int left = nodes[p].left, right = nodes[p].right;
				if (left < 0 || right < 0) {
					release(p);
					return left < 0 ? right : left;
				}
				int next;
				right = detach_first(right, next);
				nodes[next].left = left; nodes[next].right = right;
				release(p);
				return balance(next);
			}
		}
		return removed ? balance(p) : p;
	}
	int build(const std::vector<T> &values, const std::vector<size_type> &counts, int first, int last) {
		if (first == last) return -1;
		int middle = first + (last - first) / 2;
		int p = make_node(values[middle], counts[middle]);
		int left = build(values, counts, first, middle), right = build(values, counts, middle + 1, last);
		nodes[p].left = left; nodes[p].right = right;
		pull(p);
		return p;
	}
	static Sum result(const Aggregate &value) {
		if constexpr (widen) assert(static_cast<Aggregate>((std::numeric_limits<Sum>::min)()) <= value && value <= static_cast<Aggregate>((std::numeric_limits<Sum>::max)()));
		return static_cast<Sum>(value);
	}
	template<bool Reverse> T kth_impl(size_type k) const {
		assert(0 <= k && k < size());
		if constexpr (blocked) return backend.kth(Reverse != descending ? size() - 1 - k : k);
		int p = root;
		for (;;) {
			const auto &node = nodes[p];
			int first = Reverse ? node.right : node.left;
			size_type before = length(first);
			if (k < before) p = first;
			else if (k < before + node.copies) return *node.value;
			else { k -= before + node.copies; p = Reverse ? node.left : node.right; }
		}
	}
	template<bool Reverse> Sum sum_impl(size_type k) const {
		assert(k >= 0);
		if constexpr (blocked) {
			k = (std::min)(k, size());
			if constexpr (Reverse != descending) return result(backend.sum_all() - backend.sum(size() - k));
			else return result(backend.sum(k));
		}
		Aggregate answer(0);
		int p = root;
		while (p >= 0 && k) {
			const auto &node = nodes[p];
			if (k >= node.size) { answer += node.total; break; }
			int first = Reverse ? node.right : node.left;
			size_type before = length(first);
			if (k <= before) p = first;
			else {
				if (first >= 0) answer += nodes[first].total;
				size_type take = (std::min)(k - before, node.copies);
				answer += contribution(*node.value, take);
				k -= before + take;
				p = Reverse ? node.left : node.right;
			}
		}
		return result(answer);
	}
	template<bool Strict> std::optional<T> bound(const T &value) const {
		if constexpr (blocked) return backend.template bound<Strict>(value);
		int p = root, candidate = -1;
		while (p >= 0) {
			const auto &node = nodes[p];
			bool found;
			if constexpr (Strict) found = descending ? order(*node.value, value) : order(value, *node.value);
			else found = descending ? !order(value, *node.value) : !order(*node.value, value);
			if (found) { candidate = p; p = descending ? node.right : node.left; }
			else p = descending ? node.left : node.right;
		}
		return candidate < 0 ? std::nullopt : nodes[candidate].value;
	}

public:
	explicit TopKSum(Compare compare = Compare{}) : order(std::move(compare)) {}
	explicit TopKSum(std::vector<T> values, Compare compare = Compare{}) : TopKSum(std::move(compare)) { assign(std::move(values)); }
	TopKSum(std::initializer_list<T> values, Compare compare = Compare{}) : TopKSum(std::vector<T>(values), std::move(compare)) {}
	TopKSum(const TopKSum &) = default;
	TopKSum(TopKSum &&other) noexcept(std::is_nothrow_copy_constructible_v<Compare>)
		: order(other.order), backend(std::move(other.backend)), nodes(std::move(other.nodes)), root(std::exchange(other.root, -1)), free(std::exchange(other.free, -1)) { other.nodes.clear(); }
	TopKSum &operator=(TopKSum other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		swap(other);
		return *this;
	}
	void assign(std::vector<T> values) {
		assert(values.size() <= std::size_t((std::numeric_limits<size_type>::max)()));
		if constexpr (blocked) { Backend next; next.assign(std::move(values)); backend.swap(next); return; }
		std::sort(values.begin(), values.end(), order);
		std::vector<size_type> counts;
		std::size_t distinct = 0;
		for (std::size_t i = 0; i < values.size(); ++i) {
			if (!distinct || order(values[distinct - 1], values[i])) {
				if (distinct != i) values[distinct] = std::move(values[i]);
				++distinct;
				counts.push_back(1);
			} else ++counts.back();
		}
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		TopKSum next(order);
		next.nodes.reserve(distinct);
		next.root = next.build(values, counts, 0, static_cast<int>(distinct));
		swap(next);
	}
	void reserve(std::size_t distinct) {
		assert(distinct <= std::size_t((std::numeric_limits<int>::max)()));
		if constexpr (blocked) backend.reserve(distinct);
		else nodes.reserve(distinct);
	}
	void insert(const T &value, size_type copies = 1) {
		assert(copies >= 0 && copies <= (std::numeric_limits<size_type>::max)() - size());
		if constexpr (blocked) { if (copies) backend.insert(value, copies); }
		else if (copies) root = insert_node(root, value, copies);
	}
	bool erase(const T &value) { return erase(value, 1) != 0; }
	size_type erase(const T &value, size_type copies) {
		assert(copies >= 0);
		if constexpr (blocked) return copies ? backend.erase(value, copies) : 0;
		size_type removed = 0;
		if (copies) root = erase_node(root, value, copies, removed);
		return removed;
	}
	size_type erase_all(const T &value) { return erase(value, (std::numeric_limits<size_type>::max)()); }
	bool replace(const T &old_value, const T &new_value) {
		if (!erase(old_value)) return false;
		insert(new_value);
		return true;
	}
	size_type size() const { if constexpr (blocked) return backend.size(); else return length(root); }
	bool empty() const { return size() == 0; }
	T kth(size_type k) const { return kth_impl<false>(k); }
	T kth_smallest(size_type k) const { return kth_impl<descending>(k); }
	T kth_largest(size_type k) const { return kth_impl<!descending>(k); }
	Sum sum(size_type k) const { return sum_impl<false>(k); }
	Sum sum_smallest(size_type k) const { return sum_impl<descending>(k); }
	Sum sum_largest(size_type k) const { return sum_impl<!descending>(k); }
	std::optional<T> lower_bound(const T &value) const { return bound<false>(value); }
	std::optional<T> upper_bound(const T &value) const { return bound<true>(value); }
	Sum sum_all() const { if constexpr (blocked) return result(backend.sum_all()); else return root < 0 ? Sum(0) : result(nodes[root].total); }
	size_type order_of_key(const T &value) const {
		if constexpr (blocked) {
			if constexpr (descending) return size() - backend.template order_of_key<true>(value);
			else return backend.order_of_key(value);
		}
		size_type answer = 0;
		int p = root;
		while (p >= 0) {
			const auto &node = nodes[p];
			if (order(*node.value, value)) { answer += length(node.left) + node.copies; p = node.right; }
			else p = node.left;
		}
		return answer;
	}
	size_type count(const T &value) const {
		if constexpr (blocked) return backend.count(value);
		int p = root;
		while (p >= 0) {
			if (order(value, *nodes[p].value)) p = nodes[p].left;
			else if (order(*nodes[p].value, value)) p = nodes[p].right;
			else return nodes[p].copies;
		}
		return 0;
	}
	bool contains(const T &value) const { return count(value) != 0; }
	void clear() {
		if constexpr (blocked) backend.clear();
		else { nodes.clear(); root = free = -1; }
	}
	void swap(TopKSum &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		using std::swap;
		if constexpr (blocked) backend.swap(other.backend);
		swap(order, other.order); nodes.swap(other.nodes);
		swap(root, other.root); swap(free, other.free);
	}
	friend void swap(TopKSum &a, TopKSum &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using LargestKSum = TopKSum<T, Sum, std::greater<T>>;

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using SmallestKSum = TopKSum<T, Sum, std::less<T>>;

#endif
