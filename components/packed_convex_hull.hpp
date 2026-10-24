#ifndef PACKED_CONVEX_HULL_HPP
#define PACKED_CONVEX_HULL_HPP

#include "radix_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <utility>

namespace packed_convex_hull_detail {
struct Entry {
	std::uint64_t key;
	int index;
};
inline std::int64_t cross(std::uint64_t a, std::uint64_t b, std::uint64_t c) {
	std::int64_t x = a >> 32, y = std::uint32_t(a);
	return (std::int64_t(b >> 32) - x) * (std::int64_t(std::uint32_t(c)) - y)
		- (std::int64_t(std::uint32_t(b)) - y) * (std::int64_t(c >> 32) - x);
}
inline std::uint64_t key(std::uint64_t value) { return value; }
inline std::uint64_t key(Entry value) { return value.key; }
template <class Value>
std::vector<Value> build(std::vector<Value> order, bool keep_collinear) {
	if (order.size() < 256) std::sort(order.begin(), order.end(), [](const Value &a, const Value &b) { return key(a) < key(b); });
	else stable_radix_sort<11>(order, [](const Value &value) { return key(value); });
	order.erase(std::unique(order.begin(), order.end(), [](const Value &a, const Value &b) { return key(a) == key(b); }), order.end());
	if (order.size() <= 2 || (keep_collinear && std::all_of(order.begin() + 2, order.end(), [&](const Value &value) {
		return !cross(key(order[0]), key(order[1]), key(value));
	}))) {
		return order;
	}
	std::vector<Value> hull;
	hull.reserve(order.size() * 2);
	auto append = [&](Value value, std::size_t base) {
		while (hull.size() >= base) {
			auto turn = cross(key(hull[hull.size() - 2]), key(hull.back()), key(value));
			if (turn > 0 || (keep_collinear && !turn)) break;
			hull.pop_back();
		}
		hull.push_back(value);
	};
	for (auto value : order) append(value, 2);
	std::size_t base = hull.size() + 1;
	for (std::size_t i = order.size() - 1; i-- > 0;) append(order[i], base);
	hull.pop_back();
	return hull;
}
inline std::pair<std::uint64_t, std::uint64_t> diameter(const std::vector<std::uint64_t> &hull) {
	std::size_t n = hull.size();
	std::uint64_t a = hull[0], b = a, best = 0;
	auto distance = [](std::uint64_t p, std::uint64_t q) {
		std::int64_t x = std::int64_t(p >> 32) - std::int64_t(q >> 32);
		std::int64_t y = std::int64_t(std::uint32_t(p)) - std::uint32_t(q);
		return std::uint64_t(x * x + y * y);
	};
	for (std::size_t i = 0, j = n > 1; i < n; ++i) {
		std::size_t next = i + 1 == n ? 0 : i + 1;
		while (true) {
			std::size_t after = j + 1 == n ? 0 : j + 1;
			if (cross(hull[i], hull[next], hull[after]) <= cross(hull[i], hull[next], hull[j])) break;
			j = after;
		}
		for (auto p : {hull[i], hull[next]}) {
			auto d = distance(p, hull[j]);
			if (d > best) a = p, b = hull[j], best = d;
		}
	}
	return {a, b};
}

}

#endif
