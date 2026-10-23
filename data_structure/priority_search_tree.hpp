#ifndef PRIORITY_SEARCH_TREE_HPP
#define PRIORITY_SEARCH_TREE_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

template <class X = std::int64_t, class Y = X>
class PrioritySearchTree {
	std::vector<X> xs;
	std::vector<std::optional<Y>> ys;
	std::vector<int> order, ranks, heap;
	std::vector<std::size_t> positions;
	std::size_t base = 1;
	int active = 0;
	bool less(int a, int b) const {
		if (a < 0) return false;
		if (b < 0) return true;
		if (*ys[a] < *ys[b]) return true;
		if (*ys[b] < *ys[a]) return false;
		return a < b;
	}
	void push(int id) {
		std::size_t v = 1, left = 0, right = base;
		while (heap[v] >= 0) {
			if (less(id, heap[v])) {
				std::swap(id, heap[v]);
				positions[heap[v]] = v;
			}
			std::size_t middle = left + (right - left) / 2;
			if (std::size_t(ranks[id]) < middle) v *= 2, right = middle;
			else v = v * 2 + 1, left = middle;
		}
		heap[v] = id;
		positions[id] = v;
	}
	void fill_hole(std::size_t v) {
		while (v < base) {
			std::size_t child = v * 2;
			if (less(heap[child + 1], heap[child])) ++child;
			heap[v] = heap[child];
			if (heap[v] < 0) return;
			positions[heap[v]] = v;
			v = child;
		}
		heap[v] = -1;
	}
	std::pair<std::size_t, std::size_t> interval(const X &left, const X &right) const {
		if (!(left < right)) return {0, 0};
		auto lower = [&](const X &x) {
			return std::size_t(std::lower_bound(order.begin(), order.end(), x,
				[&](int id, const X &value) { return xs[id] < value; }) - order.begin());
		};
		return {lower(left), lower(right)};
	}
	template <class F>
	void enumerate_all(std::size_t v, const Y &upper, F &visit) const {
		int id = heap[v];
		if (id < 0 || !(*ys[id] < upper)) return;
		visit(id);
		if (v < base) {
			enumerate_all(v * 2, upper, visit);
			enumerate_all(v * 2 + 1, upper, visit);
		}
	}
	template <class F>
	void enumerate_node(std::size_t v, std::size_t left, std::size_t right,
		std::size_t ql, std::size_t qr, const Y &upper, F &visit) const {
		int id = heap[v];
		if (id < 0 || !(*ys[id] < upper)) return;
		if (ql <= left && right <= qr) {
			enumerate_all(v, upper, visit);
			return;
		}
		if (ql <= std::size_t(ranks[id]) && std::size_t(ranks[id]) < qr) visit(id);
		std::size_t middle = left + (right - left) / 2;
		if (ql < middle) enumerate_node(v * 2, left, middle, ql, qr, upper, visit);
		if (middle < qr) enumerate_node(v * 2 + 1, middle, right, ql, qr, upper, visit);
	}
	int min_node(std::size_t v, std::size_t left, std::size_t right,
		std::size_t ql, std::size_t qr) const {
		int id = heap[v];
		if (id < 0 || (ql <= left && right <= qr)) return id;
		int best = ql <= std::size_t(ranks[id]) && std::size_t(ranks[id]) < qr ? id : -1;
		std::size_t middle = left + (right - left) / 2;
		if (ql < middle) {
			int candidate = min_node(v * 2, left, middle, ql, qr);
			if (less(candidate, best)) best = candidate;
		}
		if (middle < qr) {
			int candidate = min_node(v * 2 + 1, middle, right, ql, qr);
			if (less(candidate, best)) best = candidate;
		}
		return best;
	}
	void check_id([[maybe_unused]] int id) const { assert(0 <= id && id < point_count()); }

public:
	PrioritySearchTree() = default;
	explicit PrioritySearchTree(const std::vector<std::pair<X, Y>> &points) { build(points); }
	explicit PrioritySearchTree(const std::vector<X> &coordinates) { build_coordinates(coordinates); }
	PrioritySearchTree(const PrioritySearchTree &) = default;
	PrioritySearchTree &operator=(const PrioritySearchTree &) = default;
	PrioritySearchTree(PrioritySearchTree &&other) noexcept : PrioritySearchTree() { swap(other); }
	PrioritySearchTree &operator=(PrioritySearchTree &&other) noexcept {
		if (this != &other) {
			PrioritySearchTree replacement(std::move(other));
			swap(replacement);
		}
		return *this;
	}
	void swap(PrioritySearchTree &other) noexcept {
		xs.swap(other.xs);
		ys.swap(other.ys);
		order.swap(other.order);
		ranks.swap(other.ranks);
		heap.swap(other.heap);
		positions.swap(other.positions);
		std::swap(base, other.base);
		std::swap(active, other.active);
	}
	int size() const { return active; }
	bool empty() const { return active == 0; }
	int point_count() const { return int(xs.size()); }
	typename std::vector<X>::const_reference x(int id) const { check_id(id); return xs[id]; }
	const std::optional<Y> &get(int id) const { check_id(id); return ys[id]; }
	bool contains(int id) const { check_id(id); return positions[id] != 0; }
	void build_coordinates(const std::vector<X> &coordinates) {
		assert(coordinates.size() <= std::size_t(std::numeric_limits<int>::max()));
		xs = coordinates;
		ys.clear();
		ys.resize(xs.size());
		positions.assign(xs.size(), 0);
		order.resize(xs.size());
		ranks.resize(xs.size());
		std::iota(order.begin(), order.end(), 0);
		std::sort(order.begin(), order.end(), [&](int a, int b) {
			if (xs[a] < xs[b]) return true;
			if (xs[b] < xs[a]) return false;
			return a < b;
		});
		for (int i = 0; i < point_count(); ++i) ranks[order[i]] = i;
		base = 1;
		while (base < xs.size()) base *= 2;
		heap.assign(base * 2, -1);
		active = 0;
	}
	void build(const std::vector<std::pair<X, Y>> &points) {
		std::vector<X> coordinates;
		coordinates.reserve(points.size());
		for (const auto &point : points) coordinates.push_back(point.first);
		build_coordinates(coordinates);
		for (int id = 0; id < point_count(); ++id) {
			ys[id] = points[id].second;
			heap[base + ranks[id]] = id;
			positions[id] = base + ranks[id];
		}
		for (std::size_t v = base; --v;) fill_hole(v);
		active = point_count();
	}
	void clear() {
		std::fill(heap.begin(), heap.end(), -1);
		std::fill(positions.begin(), positions.end(), 0);
		for (auto &y : ys) y.reset();
		active = 0;
	}
	bool insert(int id, Y y) {
		check_id(id);
		if (contains(id)) return false;
		ys[id] = std::move(y);
		push(id);
		++active;
		return true;
	}
	bool erase(int id) {
		check_id(id);
		if (!contains(id)) return false;
		fill_hole(positions[id]);
		positions[id] = 0;
		ys[id].reset();
		--active;
		return true;
	}
	void set(int id, Y y) {
		erase(id);
		insert(id, std::move(y));
	}
	std::optional<int> min_y(const X &left, const X &right) const {
		auto [l, r] = interval(left, right);
		if (l == r) return std::nullopt;
		int id = min_node(1, 0, base, l, r);
		return id < 0 ? std::nullopt : std::optional<int>(id);
	}
	template <class F>
	void enumerate(const X &left, const X &right, const Y &upper, F &&visit) const {
		auto [l, r] = interval(left, right);
		if (l < r) enumerate_node(1, 0, base, l, r, upper, visit);
	}
	std::vector<int> report(const X &left, const X &right, const Y &upper) const {
		std::vector<int> result;
		enumerate(left, right, upper, [&](int id) { result.push_back(id); });
		return result;
	}
};

#endif
