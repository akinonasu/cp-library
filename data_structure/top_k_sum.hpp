#ifndef TOP_K_SUM_HPP
#define TOP_K_SUM_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>, class Compare = std::greater<T>>
class TopKSum {
	static constexpr bool widen = std::is_integral_v<Sum> && sizeof(Sum) <= 8;
	using Aggregate = std::conditional_t<widen, std::conditional_t<std::is_signed_v<Sum>, __int128_t, __uint128_t>, Sum>;
	struct Node;
	using Pointer = std::unique_ptr<Node>;
	struct Node {
		T value;
		Aggregate total;
		Pointer left, right;
		std::size_t size = 1;
		int height = 1;
		explicit Node(const T &x) : value(x), total(static_cast<Aggregate>(static_cast<Sum>(x))) {}
	};
	[[no_unique_address]] Compare order;
	Pointer root;
	std::size_t limit;
	Sum selected_sum = Sum(0);

	static std::size_t length(const Pointer &p) { return p ? p->size : 0; }
	static int height(const Pointer &p) { return p ? p->height : 0; }
	static void pull(Node &p) {
		Aggregate total(0);
		if (p.left) total += p.left->total;
		total += static_cast<Aggregate>(static_cast<Sum>(p.value));
		if (p.right) total += p.right->total;
		p.total = std::move(total);
		p.size = length(p.left) + 1 + length(p.right);
		p.height = (std::max)(height(p.left), height(p.right)) + 1;
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
	static Pointer join(Pointer left, Pointer middle, Pointer right) {
		if (height(left) > height(right) + 1) {
			left->right = join(std::move(left->right), std::move(middle), std::move(right));
			return balance(std::move(left));
		}
		if (height(right) > height(left) + 1) {
			right->left = join(std::move(left), std::move(middle), std::move(right->left));
			return balance(std::move(right));
		}
		middle->left = std::move(left); middle->right = std::move(right);
		pull(*middle);
		return middle;
	}
	static std::pair<Pointer, Pointer> split(Pointer p, std::size_t k) {
		if (!k) return {nullptr, std::move(p)};
		if (k == length(p)) return {std::move(p), nullptr};
		auto left = std::move(p->left), right = std::move(p->right);
		if (k <= length(left)) {
			auto [a, b] = split(std::move(left), k);
			return {std::move(a), join(std::move(b), std::move(p), std::move(right))};
		}
		auto count = k - length(left) - 1;
		auto [a, b] = split(std::move(right), count);
		return {join(std::move(left), std::move(p), std::move(a)), std::move(b)};
	}
	static Pointer detach_first(Pointer &p) {
		if (!p->left) {
			auto result = std::move(p);
			p = std::move(result->right);
			return result;
		}
		auto result = detach_first(p->left);
		p = balance(std::move(p));
		return result;
	}
	static Pointer concat(Pointer left, Pointer right) {
		if (!left || !right) return left ? std::move(left) : std::move(right);
		auto middle = detach_first(right);
		return join(std::move(left), std::move(middle), std::move(right));
	}
	Pointer insert_node(Pointer p, Pointer node) {
		if (!p) { pull(*node); return node; }
		if (order(node->value, p->value)) p->left = insert_node(std::move(p->left), std::move(node));
		else p->right = insert_node(std::move(p->right), std::move(node));
		return balance(std::move(p));
	}
	std::size_t bound(const T &value, bool upper) const {
		std::size_t result = 0;
		auto p = root.get();
		while (p) {
			if (upper ? !order(value, p->value) : order(p->value, value)) {
				result += length(p->left) + 1;
				p = p->right.get();
			} else p = p->left.get();
		}
		return result;
	}
	static Pointer clone(const Pointer &p) {
		if (!p) return nullptr;
		auto next = std::make_unique<Node>(p->value);
		next->left = clone(p->left); next->right = clone(p->right);
		next->total = p->total; next->size = p->size; next->height = p->height;
		return next;
	}
	template<class Value> static Pointer build(std::size_t first, std::size_t last, const Value &value) {
		if (first == last) return nullptr;
		auto middle = first + (last - first) / 2;
		auto node = std::make_unique<Node>(value(middle));
		node->left = build(first, middle, value); node->right = build(middle + 1, last, value);
		pull(*node);
		return node;
	}
	void refresh() { selected_sum = sum(limit); }

public:
	using value_type = T;
	using sum_type = Sum;
	using size_type = std::size_t;
	using value_compare = Compare;

	explicit TopKSum(size_type k = 0, Compare compare = Compare{}) : order(compare), limit(k) {}
	TopKSum(size_type k, std::vector<T> values, Compare compare = Compare{})
		: TopKSum(k, compare) { assign(std::move(values)); }
	explicit TopKSum(std::vector<T> values, Compare compare = Compare{}) : TopKSum(0, std::move(values), compare) {}
	TopKSum(const TopKSum &other) : order(other.order), root(clone(other.root)), limit(other.limit), selected_sum(other.selected_sum) {}
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
		result.root = build(0, values.size(), [&](size_type i) -> const T & { return values[i]; });
		result.refresh();
		swap(result);
	}
	void insert(const T &value) {
		assert(size() < (std::numeric_limits<size_type>::max)());
		auto node = std::make_unique<Node>(value);
		root = insert_node(std::move(root), std::move(node));
		refresh();
	}
	void insert(const T &value, size_type copies) {
		if (!copies) return;
		assert(copies <= (std::numeric_limits<size_type>::max)() - size());
		auto middle = build(0, copies, [&](size_type) -> const T & { return value; });
		auto index = bound(value, true);
		auto [left, right] = split(std::move(root), index);
		root = concat(concat(std::move(left), std::move(middle)), std::move(right));
		refresh();
	}
	bool erase(const T &value) { return erase(value, 1) != 0; }
	size_type erase(const T &value, size_type copies) {
		if (!copies) return 0;
		auto first = bound(value, false), last = bound(value, true);
		auto removed = (std::min)(copies, last - first);
		if (!removed) return 0;
		auto [left, tail] = split(std::move(root), first);
		auto [middle, right] = split(std::move(tail), removed);
		root = concat(std::move(left), std::move(right));
		refresh();
		return removed;
	}
	bool replace(const T &old_value, const T &new_value) {
		auto first = bound(old_value, false);
		if (first == bound(old_value, true)) return false;
		auto [left, tail] = split(std::move(root), first);
		auto [node, right] = split(std::move(tail), 1);
		root = concat(std::move(left), std::move(right));
		node->value = new_value;
		root = insert_node(std::move(root), std::move(node));
		refresh();
		return true;
	}
	void set_k(size_type k) { limit = k; refresh(); }
	size_type k() const { return limit; }
	size_type size() const { return length(root); }
	size_type selected_size() const { return (std::min)(limit, size()); }
	bool empty() const { return !root; }
	T kth(size_type k) const {
		assert(k < size());
		auto p = root.get();
		for (;;) {
			auto left = length(p->left);
			if (k == left) return p->value;
			if (k < left) p = p->left.get();
			else k -= left + 1, p = p->right.get();
		}
	}
	Sum sum() const { return selected_sum; }
	Sum sum(size_type k) const {
		Aggregate result(0);
		auto p = root.get();
		while (p && k) {
			if (k >= p->size) { result += p->total; break; }
			auto left = length(p->left);
			if (k <= left) p = p->left.get();
			else {
				if (p->left) result += p->left->total;
				result += static_cast<Aggregate>(static_cast<Sum>(p->value));
				k -= left + 1;
				p = p->right.get();
			}
		}
		if constexpr (widen) assert(static_cast<Aggregate>((std::numeric_limits<Sum>::min)()) <= result && result <= static_cast<Aggregate>((std::numeric_limits<Sum>::max)()));
		return static_cast<Sum>(result);
	}
	bool contains(const T &value) const {
		auto p = root.get();
		while (p) {
			if (order(value, p->value)) p = p->left.get();
			else if (order(p->value, value)) p = p->right.get();
			else return true;
		}
		return false;
	}
	size_type count(const T &value) const { return bound(value, true) - bound(value, false); }
	void clear() { root.reset(); selected_sum = Sum(0); }
	void swap(TopKSum &other) noexcept(std::is_nothrow_swappable_v<Compare> && std::is_nothrow_swappable_v<Sum>) {
		using std::swap;
		swap(order, other.order); root.swap(other.root);
		swap(limit, other.limit); swap(selected_sum, other.selected_sum);
	}
	friend void swap(TopKSum &a, TopKSum &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using LargestKSum = TopKSum<T, Sum, std::greater<T>>;

template<class T = std::int64_t, class Sum = std::common_type_t<T, std::int64_t>>
using SmallestKSum = TopKSum<T, Sum, std::less<T>>;

#endif
