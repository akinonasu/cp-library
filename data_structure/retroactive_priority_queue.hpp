#ifndef RETROACTIVE_PRIORITY_QUEUE_HPP
#define RETROACTIVE_PRIORITY_QUEUE_HPP

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

template <class T, class Compare = std::less<T>, class Time = std::int64_t, int BlockSize = 256>
class RetroactivePriorityQueue {
	static_assert(BlockSize >= 8 && BlockSize <= std::numeric_limits<int>::max() / 2);
	static_assert(std::is_integral_v<Time> && !std::is_same_v<Time, bool> && sizeof(Time) <= 8);
	struct Operation {
		Time time;
		std::optional<T> value;
		int left = 0, right = 0;
		std::uint32_t priority;
		Operation(Time time, std::optional<T> value, std::uint32_t priority) : time(time), value(std::move(value)), priority(priority) {}
	};
	struct Summary {
		std::vector<int> alive, dead;
		int pops = 0;
	};
	struct Block : Summary {
		std::vector<int> operations, prefix, suffix;
	};
	std::vector<Operation> operations;
	std::vector<Block> blocks;
	std::vector<int> prefix{0}, suffix{0};
	[[no_unique_address]] Compare compare;
	SplitMix64 rng;
	int root = 0, free = 0, events = 0;
	mutable int cached = 0;
	mutable bool dirty = false;
	bool less(int a, int b) const {
		const auto &x = operations[a], &y = operations[b];
		if (compare(*x.value, *y.value)) return true;
		if (compare(*y.value, *x.value)) return false;
		return x.time < y.time;
	}
	int make(Time time, std::optional<T> value) {
		if (free) {
			int x = free;
			free = operations[x].left;
			operations[x] = Operation(time, std::move(value), std::uint32_t(rng()));
			return x;
		}
		assert(operations.size() < std::size_t(std::numeric_limits<int>::max()));
		operations.emplace_back(time, std::move(value), std::uint32_t(rng()));
		return int(operations.size()) - 1;
	}
	std::pair<int, int> split(int x, int key) {
		if (!x) return {0, 0};
		if (less(x, key)) {
			auto [a, b] = split(operations[x].right, key);
			operations[x].right = a;
			return {x, b};
		}
		auto [a, b] = split(operations[x].left, key);
		operations[x].left = b;
		return {a, x};
	}
	int merge(int a, int b) {
		if (!a || !b) return a | b;
		if (operations[a].priority > operations[b].priority) {
			operations[a].right = merge(operations[a].right, b);
			return a;
		}
		operations[b].left = merge(a, operations[b].left);
		return b;
	}
	int insert_key(int x, int key) {
		if (!x) return key;
		if (operations[key].priority >= operations[x].priority) {
			auto [a, b] = split(x, key);
			operations[key].left = a;
			operations[key].right = b;
			return key;
		}
		if (less(key, x)) operations[x].left = insert_key(operations[x].left, key);
		else operations[x].right = insert_key(operations[x].right, key);
		return x;
	}
	int erase_key(int x, int key) {
		if (x == key) return merge(operations[x].left, operations[x].right);
		if (less(key, x)) operations[x].left = erase_key(operations[x].left, key);
		else operations[x].right = erase_key(operations[x].right, key);
		return x;
	}
	std::pair<int, int> locate(Time time) const {
		int b = std::lower_bound(blocks.begin(), blocks.end(), time, [&](const Block &block, Time t) {
			return operations[block.operations.back()].time < t;
		}) - blocks.begin();
		if (b == int(blocks.size())) return {b, 0};
		const auto &v = blocks[b].operations;
		int p = std::lower_bound(v.begin(), v.end(), time, [&](int x, Time t) { return operations[x].time < t; }) - v.begin();
		return {b, p};
	}
	bool exists(int b, int p, Time time) const {
		return b < int(blocks.size()) && p < int(blocks[b].operations.size()) && operations[blocks[b].operations[p]].time == time;
	}
	void summarize(Summary &summary, const std::vector<int> &v, int n) const {
		summary.alive.clear(); summary.dead.clear(); summary.pops = 0;
		summary.alive.reserve(n);
		auto heap_compare = [&](int a, int b) { return less(b, a); };
		for (int i = 0; i < n; ++i) {
			int x = v[i];
			if (operations[x].value) {
				summary.alive.push_back(x);
				std::push_heap(summary.alive.begin(), summary.alive.end(), heap_compare);
			} else {
				++summary.pops;
				if (!summary.alive.empty()) {
					std::pop_heap(summary.alive.begin(), summary.alive.end(), heap_compare);
					summary.dead.push_back(summary.alive.back());
					summary.alive.pop_back();
				}
			}
		}
		auto order = [&](int a, int b) { return less(a, b); };
		std::sort(summary.alive.begin(), summary.alive.end(), order);
		std::sort(summary.dead.begin(), summary.dead.end(), order);
	}
	void rebuild(int b) {
		auto &v = blocks[b];
		int n = v.operations.size();
		summarize(v, v.operations, n);
		v.prefix.resize(n + 1); v.suffix.resize(n + 1);
		v.prefix[0] = 0;
		for (int i = 0; i < n; ++i) v.prefix[i + 1] = v.prefix[i] + (operations[v.operations[i]].value ? 1 : -1);
		v.suffix[n] = v.prefix[n];
		for (int i = n; i--;) v.suffix[i] = std::min(v.prefix[i], v.suffix[i + 1]);
	}
	void refresh() {
		int n = blocks.size();
		prefix.resize(n + 1); suffix.resize(n + 1);
		prefix[0] = 0;
		for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + blocks[i].prefix.back();
		suffix[n] = prefix[n];
		for (int i = n; i--;) suffix[i] = std::min(prefix[i] + blocks[i].suffix[0], suffix[i + 1]);
		dirty = true;
	}
	void split_block(int b) {
		Block right;
		auto &v = blocks[b].operations;
		int middle = v.size() / 2;
		right.operations.assign(v.begin() + middle, v.end());
		v.resize(middle);
		blocks.insert(blocks.begin() + b + 1, std::move(right));
		rebuild(b); rebuild(b + 1);
	}
	int minimum_suffix(int b, int p) const {
		if (b == int(blocks.size())) return prefix.back();
		return std::min(prefix[b] + blocks[b].suffix[p], suffix[b + 1]);
	}
	void add(int b, int p, int x) {
		if (blocks.empty()) blocks.emplace_back();
		else if (b == int(blocks.size())) { --b; p = blocks[b].operations.size(); }
		auto &v = blocks[b].operations;
		v.insert(v.begin() + p, x);
		if (v.size() > 2 * BlockSize) split_block(b);
		else rebuild(b);
		++events;
		refresh();
	}
	void remove(int b, int p) {
		auto &v = blocks[b].operations;
		v.erase(v.begin() + p);
		if (blocks.size() == 1 && v.empty()) blocks.clear();
		else if (v.size() < BlockSize / 2 && blocks.size() > 1) {
			int left = b ? b - 1 : 0;
			auto &a = blocks[left].operations;
			const auto &c = blocks[left + 1].operations;
			a.insert(a.end(), c.begin(), c.end());
			blocks.erase(blocks.begin() + left + 1);
			if (a.size() > 2 * BlockSize) split_block(left);
			else rebuild(left);
		} else rebuild(b);
		--events;
		refresh();
	}
	int rank(const std::vector<int> &v, int key) const {
		if (v.empty() || less(key, v.front())) return 0;
		if (!less(key, v.back())) return v.size();
		return std::upper_bound(v.begin(), v.end(), key, [&](int a, int b) { return less(a, b); }) - v.begin();
	}
	bool present(int key, int full, const Summary *partial) const {
		int needed = 0;
		if (partial) {
			int alive = rank(partial->alive, key);
			if (alive) return true;
			needed = partial->pops - rank(partial->dead, key);
			if (needed >= prefix[full]) return false;
		}
		while (full) {
			const auto &v = blocks[--full];
			int alive = rank(v.alive, key);
			if (alive > needed) return true;
			needed += v.pops - rank(v.dead, key) - alive;
			if (needed >= prefix[full]) return false;
		}
		return false;
	}
	int query(int full, int p) const {
		Summary partial;
		const Summary *tail = nullptr;
		if (p) {
			if (p == int(blocks[full].operations.size())) ++full;
			else { partial.dead.reserve((p - blocks[full].prefix[p]) / 2); summarize(partial, blocks[full].operations, p); tail = &partial; }
		}
		if (!full) return partial.alive.empty() ? 0 : partial.alive.front();
		if (full == 1 && !tail) return blocks[0].alive.empty() ? 0 : blocks[0].alive.front();
		int x = root, answer = 0;
		while (x) {
			if (present(x, full, tail)) { answer = x; x = operations[x].left; }
			else x = operations[x].right;
		}
		return answer;
	}

  public:
	using value_type = T;
	using time_type = Time;
	using update_type = std::pair<Time, std::optional<T>>;
	explicit RetroactivePriorityQueue(Compare compare = Compare{}, std::uint64_t seed = 0) : compare(std::move(compare)), rng(seed) { operations.emplace_back(Time(0), std::nullopt, 0); }
	explicit RetroactivePriorityQueue(const std::vector<update_type> &updates, Compare compare = Compare{}, std::uint64_t seed = 0) : RetroactivePriorityQueue(std::move(compare), seed) { bool valid = assign(updates); assert(valid); (void)valid; }
	RetroactivePriorityQueue(const RetroactivePriorityQueue &) = default;
	RetroactivePriorityQueue &operator=(const RetroactivePriorityQueue &) = default;
	RetroactivePriorityQueue(RetroactivePriorityQueue &&other) : RetroactivePriorityQueue(other.compare) { swap(other); }
	RetroactivePriorityQueue &operator=(RetroactivePriorityQueue &&other) {
		if (this != &other) { RetroactivePriorityQueue moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(RetroactivePriorityQueue &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
		operations.swap(other.operations); blocks.swap(other.blocks); prefix.swap(other.prefix); suffix.swap(other.suffix);
		using std::swap;
		swap(compare, other.compare); swap(rng, other.rng); swap(root, other.root); swap(free, other.free); swap(events, other.events); swap(cached, other.cached); swap(dirty, other.dirty);
	}
	void reserve(std::size_t n) { assert(n < std::size_t(std::numeric_limits<int>::max())); operations.reserve(n + 1); blocks.reserve(2 * n / BlockSize + 1); }
	bool assign(const std::vector<update_type> &updates) {
		assert(updates.size() < std::size_t(std::numeric_limits<int>::max()));
		int n = updates.size(), balance = 0;
		std::vector<int> order(n);
		for (int i = 0; i < n; ++i) order[i] = i;
		std::sort(order.begin(), order.end(), [&](int a, int b) { return updates[a].first < updates[b].first; });
		for (int i = 0; i < n; ++i) {
			if (i && updates[order[i - 1]].first == updates[order[i]].first) return false;
			balance += updates[order[i]].second ? 1 : -1;
			if (balance < 0) return false;
		}
		RetroactivePriorityQueue next(compare);
		next.rng = rng;
		next.reserve(n);
		std::vector<int> keys, path;
		keys.reserve(n);
		for (int i : order) {
			int x = next.make(updates[i].first, updates[i].second);
			if (next.operations[x].value) keys.push_back(x);
			if ((x - 1) % BlockSize == 0) next.blocks.emplace_back();
			next.blocks.back().operations.push_back(x);
		}
		for (int b = 0; b < int(next.blocks.size()); ++b) next.rebuild(b);
		std::sort(keys.begin(), keys.end(), [&](int a, int b) { return next.less(a, b); });
		for (int x : keys) {
			int last = 0;
			while (!path.empty() && next.operations[path.back()].priority <= next.operations[x].priority) { last = path.back(); path.pop_back(); }
			next.operations[x].left = last;
			if (!path.empty()) next.operations[path.back()].right = x;
			path.push_back(x);
		}
		next.root = path.empty() ? 0 : path[0];
		next.events = n;
		next.refresh();
		swap(next);
		return true;
	}
	int operation_count() const { return events; }
	int size() const { return prefix.back(); }
	bool empty() const { return size() == 0; }
	int size(Time time) const {
		auto [b, p] = locate(time);
		if (exists(b, p, time)) ++p;
		return b == int(blocks.size()) ? prefix.back() : prefix[b] + blocks[b].prefix[p];
	}
	bool empty(Time time) const { return size(time) == 0; }
	bool contains_time(Time time) const { auto [b, p] = locate(time); return exists(b, p, time); }
	bool insert(Time time, T value) {
		auto [b, p] = locate(time);
		if (exists(b, p, time)) return false;
		int x = make(time, std::move(value));
		root = insert_key(root, x);
		add(b, p, x);
		return true;
	}
	bool insert_pop(Time time) {
		auto [b, p] = locate(time);
		if (exists(b, p, time) || minimum_suffix(b, p) < 1) return false;
		add(b, p, make(time, std::nullopt));
		return true;
	}
	bool set(Time time, T value) {
		auto [b, p] = locate(time);
		if (!exists(b, p, time)) return false;
		int x = blocks[b].operations[p];
		if (!operations[x].value) return false;
		root = erase_key(root, x);
		operations[x].value = std::move(value);
		operations[x].left = operations[x].right = 0;
		root = insert_key(root, x);
		rebuild(b); dirty = true;
		return true;
	}
	bool erase(Time time) {
		auto [b, p] = locate(time);
		if (!exists(b, p, time)) return false;
		int x = blocks[b].operations[p];
		if (operations[x].value) {
			if (minimum_suffix(b, p + 1) < 1) return false;
			root = erase_key(root, x);
			operations[x].value.reset();
		}
		remove(b, p);
		operations[x].left = free; free = x;
		return true;
	}
	std::optional<T> top() const {
		if (empty()) return std::nullopt;
		if (dirty) { cached = query(blocks.size(), 0); dirty = false; }
		return operations[cached].value;
	}
	std::optional<T> top(Time time) const {
		auto [b, p] = locate(time);
		if (b == int(blocks.size())) return top();
		if (exists(b, p, time)) ++p;
		if (prefix[b] + blocks[b].prefix[p] == 0) return std::nullopt;
		return operations[query(b, p)].value;
	}
	void clear() {
		operations.erase(operations.begin() + 1, operations.end()); blocks.clear();
		prefix.assign(1, 0); suffix.assign(1, 0); root = free = events = cached = 0; dirty = false;
	}
	std::size_t memory_usage() const {
		std::size_t bytes = operations.capacity() * sizeof(Operation) + blocks.capacity() * sizeof(Block) + (prefix.capacity() + suffix.capacity()) * sizeof(int);
		for (const auto &b : blocks) bytes += (b.operations.capacity() + b.alive.capacity() + b.dead.capacity() + b.prefix.capacity() + b.suffix.capacity()) * sizeof(int);
		return bytes;
	}
};

#endif
