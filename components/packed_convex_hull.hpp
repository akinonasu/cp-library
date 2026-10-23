#ifndef PACKED_CONVEX_HULL_HPP
#define PACKED_CONVEX_HULL_HPP

#include "radix_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

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
	stable_radix_sort<11>(order, [](const Value &value) { return key(value); });
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
}

#endif
