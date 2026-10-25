#ifndef PARTIALLY_RETROACTIVE_PRIORITY_QUEUE_HPP
#define PARTIALLY_RETROACTIVE_PRIORITY_QUEUE_HPP

#include "../components/splitmix64.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, class Compare = std::less<T>, class Time = std::int64_t>
class PartiallyRetroactivePriorityQueue {
	static_assert(std::is_integral_v<Time> && !std::is_same_v<Time, bool> && sizeof(Time) <= 8);
	struct Ignore { void operator()(Time, const T &, bool) const {} };
	struct Node {
		Time time{};
		std::optional<T> value;
		int left = 0, right = 0, size = 0, sum = 0, low = 0, count = 0, minimum_count = 0, alive = 0, dead = 0;
		std::uint32_t priority = 0;
		int state = 0;
	};
	std::vector<Node> nodes{1};
	[[no_unique_address]] Compare compare;
	SplitMix64 rng;
	int root = 0, free = 0;
	bool less(int a, int b) const {
		if (compare(*nodes[a].value, *nodes[b].value)) return true;
		if (compare(*nodes[b].value, *nodes[a].value)) return false;
		return nodes[a].time < nodes[b].time;
	}
	int smaller(int a, int b) const { return !a ? b : !b || less(a, b) ? a : b; }
	int larger(int a, int b) const { return !a ? b : !b || less(b, a) ? a : b; }
	void pull(int x) {
		auto &n = nodes[x];
		const auto &a = nodes[n.left], &b = nodes[n.right];
		int c = n.value ? 1 : -1;
		n.size = a.size + 1 + b.size;
		n.sum = a.sum + n.state + b.sum;
		n.low = std::min(a.low, a.sum + n.state + b.low);
		n.count = a.count + c + b.count;
		n.minimum_count = std::min(a.minimum_count, a.count + c + b.minimum_count);
		n.alive = smaller(smaller(a.alive, n.value && n.state == 0 ? x : 0), b.alive);
		n.dead = larger(larger(a.dead, n.state == 1 ? x : 0), b.dead);
	}
	int make(Time time, std::optional<T> value) {
		int x;
		if (free) { x = free; free = nodes[x].left; nodes[x] = Node{}; }
		else {
			assert(nodes.size() < std::size_t(std::numeric_limits<int>::max()));
			x = nodes.size(); nodes.emplace_back();
		}
		nodes[x].time = time;
		nodes[x].value = std::move(value);
		nodes[x].priority = std::uint32_t(rng());
		nodes[x].state = nodes[x].value ? 0 : -1;
		pull(x);
		return x;
	}
	int merge(int a, int b) {
		if (!a || !b) return a | b;
		if (nodes[a].priority > nodes[b].priority) {
			nodes[a].right = merge(nodes[a].right, b); pull(a); return a;
		}
		nodes[b].left = merge(a, nodes[b].left); pull(b); return b;
	}
	std::pair<int, int> split(int x, int k) {
		if (!x) return {0, 0};
		int n = nodes[nodes[x].left].size;
		if (k <= n) {
			auto [a, b] = split(nodes[x].left, k);
			nodes[x].left = b; pull(x); return {a, x};
		}
		auto [a, b] = split(nodes[x].right, k - n - 1);
		nodes[x].right = a; pull(x); return {x, b};
	}
	std::pair<int, int> locate(Time time) const {
		int x = root, rank = 0;
		while (x) {
			if (time < nodes[x].time) x = nodes[x].left;
			else {
				int n = nodes[nodes[x].left].size;
				if (time == nodes[x].time) return {rank + n, x};
				rank += n + 1; x = nodes[x].right;
			}
		}
		return {rank, 0};
	}
	int bridge_dead(int x, int offset) const {
		if (!x) return 0;
		int a = nodes[x].left, b = nodes[x].right;
		int after = offset + nodes[a].sum + nodes[x].state;
		if (after + nodes[b].low == 0) return bridge_dead(b, after);
		return larger(larger(bridge_dead(a, offset), nodes[x].state == 1 ? x : 0), nodes[b].dead);
	}
	int bridge_alive(int x, int offset) const {
		if (offset == 0) return 0;
		assert(x);
		int a = nodes[x].left;
		if (offset + nodes[a].low == 0) return bridge_alive(a, offset);
		int after = offset + nodes[a].sum + nodes[x].state;
		return smaller(smaller(nodes[a].alive, nodes[x].value && nodes[x].state == 0 ? x : 0), bridge_alive(nodes[x].right, after));
	}
	void change(int x, int key, int state) {
		assert(x);
		if (x == key) nodes[x].state = state;
		else change(nodes[key].time < nodes[x].time ? nodes[x].left : nodes[x].right, key, state);
		pull(x);
	}
	struct Selection { int tree, key; };
	Selection promote(int a, int b) const {
		int key = bridge_dead(a, 0);
		return larger(key, nodes[b].dead) == key ? Selection{a, key} : Selection{b, nodes[b].dead};
	}
	Selection demote(int a, int b, int offset) const {
		int key = bridge_alive(b, offset);
		return smaller(nodes[a].alive, key) == key ? Selection{b, key} : Selection{a, nodes[a].alive};
	}
	void change(Selection selected, int state) { change(selected.tree, selected.key, state); }
	int insert_push(int a, int b, int x) {
		auto selected = promote(a, b);
		int added = x;
		if (selected.key && less(x, selected.key)) {
			change(selected, 0); nodes[x].state = 1; pull(x);
			added = selected.key;
		}
		root = merge(merge(a, x), b);
		return added;
	}
	template <class F> bool erase_at(int rank, F &&on_change) {
		auto [a, tail] = split(root, rank);
		auto [x, b] = split(tail, 1);
		if (nodes[x].value && nodes[a].count + nodes[b].minimum_count < 0) {
			root = merge(merge(a, x), b); return false;
		}
		int changed = x;
		bool added = nodes[x].state == -1;
		if (added) {
			auto selected = promote(a, b);
			changed = selected.key; change(selected, 0);
		} else if (nodes[x].state == 1) {
			auto selected = demote(a, b, nodes[a].sum + 1);
			changed = selected.key; change(selected, 1);
		}
		root = merge(a, b);
		on_change(changed, added);
		nodes[x].value.reset(); nodes[x].left = free; free = x;
		return true;
	}
	template <class F> void visit(int x, F &f) const {
		if (!nodes[x].alive) return;
		visit(nodes[x].left, f);
		if (nodes[x].value && nodes[x].state == 0) f(nodes[x].time, *nodes[x].value);
		visit(nodes[x].right, f);
	}
	template <class F> void notify(int x, bool added, F &f) const { f(nodes[x].time, *nodes[x].value, added); }
public:
	using update_type = std::pair<Time, std::optional<T>>;
	explicit PartiallyRetroactivePriorityQueue(Compare compare = Compare{}, std::uint64_t seed = 0) : compare(std::move(compare)), rng(seed) {}
	PartiallyRetroactivePriorityQueue(const std::vector<update_type> &updates, Compare compare = Compare{}, std::uint64_t seed = 0)
		: PartiallyRetroactivePriorityQueue(std::move(compare), seed) {
		[[maybe_unused]] bool ok = assign(updates);
		assert(ok);
	}
	PartiallyRetroactivePriorityQueue(const PartiallyRetroactivePriorityQueue &) = default;
	PartiallyRetroactivePriorityQueue &operator=(const PartiallyRetroactivePriorityQueue &) = default;
	PartiallyRetroactivePriorityQueue(PartiallyRetroactivePriorityQueue &&other) : PartiallyRetroactivePriorityQueue(other.compare) { swap(other); }
	PartiallyRetroactivePriorityQueue &operator=(PartiallyRetroactivePriorityQueue &&other) {
		if (this != &other) { PartiallyRetroactivePriorityQueue next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(PartiallyRetroactivePriorityQueue &other) {
		using std::swap;
		nodes.swap(other.nodes); swap(compare, other.compare); swap(rng, other.rng); swap(root, other.root); swap(free, other.free);
	}
	bool insert(Time time, T value) { return insert(time, std::move(value), Ignore{}); }
	template <class F> bool insert(Time time, T value, F &&on_change) {
		auto [rank, found] = locate(time);
		if (found) return false;
		int x = make(time, std::move(value));
		auto [a, b] = split(root, rank);
		int added = insert_push(a, b, x);
		notify(added, true, on_change); return true;
	}
	bool insert_pop(Time time) { return insert_pop(time, Ignore{}); }
	template <class F> bool insert_pop(Time time, F &&on_change) {
		auto [rank, found] = locate(time);
		if (found) return false;
		auto [a, b] = split(root, rank);
		if (nodes[a].count + nodes[b].minimum_count < 1) { root = merge(a, b); return false; }
		auto selected = demote(a, b, nodes[a].sum);
		change(selected, 1);
		int x = make(time, std::nullopt);
		root = merge(merge(a, x), b);
		notify(selected.key, false, on_change); return true;
	}
	bool set(Time time, T value) { return set(time, std::move(value), Ignore{}); }
	template <class F> bool set(Time time, T value, F &&on_change) {
		auto [rank, found] = locate(time);
		if (!found || !nodes[found].value) return false;
		int x = make(time, std::move(value));
		auto [a, b] = split(root, rank);
		int added = insert_push(a, b, x);
		bool canceled = false;
		[[maybe_unused]] bool ok = erase_at(rank + 1, [&](int key, bool inserted) {
			assert(!inserted); (void)inserted;
			if (key == added) canceled = true;
			else notify(key, false, on_change);
		});
		assert(ok);
		if (!canceled) notify(added, true, on_change);
		return true;
	}
	bool erase(Time time) { return erase(time, Ignore{}); }
	template <class F> bool erase(Time time, F &&on_change) {
		auto [rank, found] = locate(time);
		return found && erase_at(rank, [&](int key, bool added) { notify(key, added, on_change); });
	}
	bool contains_time(Time time) const { return locate(time).second != 0; }
	const T *top_ptr() const { int x = nodes[root].alive; return x ? &*nodes[x].value : nullptr; }
	std::optional<T> top() const { auto p = top_ptr(); return p ? std::optional<T>(*p) : std::nullopt; }
	int size() const { return nodes[root].count; }
	bool empty() const { return size() == 0; }
	int operation_count() const { return nodes[root].size; }
	void reserve(std::size_t capacity) { nodes.reserve(capacity + 1); }
	void clear() { nodes.resize(1); root = free = 0; }
	template <class F> void clear(F &&on_change) {
		for_each([&](Time time, const T &value) { on_change(time, value, false); });
		clear();
	}
	template <class F> void for_each(F &&f) const { visit(root, f); }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	bool assign(const std::vector<update_type> &updates) { return assign(updates, Ignore{}); }
	template <class F> bool assign(const std::vector<update_type> &updates, F &&on_change) {
		auto sorted = updates;
		std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
		int count = 0;
		for (std::size_t i = 0; i < sorted.size(); ++i) {
			if (i && sorted[i - 1].first == sorted[i].first) return false;
			count += sorted[i].second ? 1 : -1;
			if (count < 0) return false;
		}
		PartiallyRetroactivePriorityQueue next(compare, rng());
		next.reserve(sorted.size());
		std::vector<int> heap, stack;
		heap.reserve(count); stack.reserve(sorted.size());
		auto order = [&](int a, int b) { return next.less(b, a); };
		for (auto &[time, value] : sorted) {
			int x = next.make(time, std::move(value));
			if (next.nodes[x].value) {
				heap.push_back(x); std::push_heap(heap.begin(), heap.end(), order);
			} else {
				std::pop_heap(heap.begin(), heap.end(), order);
				next.nodes[heap.back()].state = 1; heap.pop_back();
			}
		}
		for (int x = 1; x < int(next.nodes.size()); ++x) {
			int left = 0;
			while (!stack.empty() && next.nodes[stack.back()].priority < next.nodes[x].priority) {
				left = stack.back(); stack.pop_back(); next.pull(left);
			}
			next.nodes[x].left = left;
			if (!stack.empty()) next.nodes[stack.back()].right = x;
			stack.push_back(x);
		}
		while (!stack.empty()) { next.root = stack.back(); next.pull(next.root); stack.pop_back(); }
		if constexpr (!std::is_same_v<std::remove_cvref_t<F>, Ignore>) {
			for_each([&](Time time, const T &value) { on_change(time, value, false); });
			next.for_each([&](Time time, const T &value) { on_change(time, value, true); });
		}
		swap(next); return true;
	}
};

#endif
