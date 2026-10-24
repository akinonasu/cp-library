#ifndef DYNAMIC_CONVEX_HULL_HPP
#define DYNAMIC_CONVEX_HULL_HPP

#include "point.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <type_traits>
#include <vector>

namespace geometry {
template<class T = std::int64_t> class DynamicConvexHull {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using P = Point<T>;
	using W = __int128_t;
	struct Direction { W x, y; };
	struct Node { P point; mutable P next; mutable bool last; };
	struct Compare {
		using is_transparent = void;
		bool operator()(const Node &a, const Node &b) const { return a.point < b.point; }
		bool operator()(const Node &a, P b) const { return a.point < b; }
		bool operator()(P a, const Node &b) const { return a < b.point; }
		bool operator()(const Node &a, Direction d) const {
			return !a.last && (W(a.next.x) - a.point.x) * d.x + (W(a.next.y) - a.point.y) * d.y > 0;
		}
		bool operator()(Direction d, const Node &a) const { return !(*this)(a, d); }
	};
	struct Chain {
		std::set<Node, Compare> nodes;
		W area = 0;
		using Iterator = typename std::set<Node, Compare>::iterator;
		void erase(Iterator i) {
			auto j = std::next(i);
			if (j != nodes.end()) area -= cross(i->point, j->point);
			if (i != nodes.begin()) {
				auto p = std::prev(i);
				area -= cross(p->point, i->point);
				p->last = j == nodes.end();
				if (j != nodes.end()) { p->next = j->point; area += cross(p->point, j->point); }
			}
			nodes.erase(i);
		}
		bool insert(P p, bool upper) {
			auto next = nodes.lower_bound(P{p.x, std::numeric_limits<T>::lowest()});
			if (next != nodes.end() && next->point.x == p.x) {
				if (upper ? p.y <= next->point.y : p.y >= next->point.y) return false;
				auto old = next++;
				erase(old);
			}
			auto turn = [&](P a, P b, P c) { auto value = cross(a, b, c); return upper ? -value : value; };
			if (next != nodes.begin() && next != nodes.end() && turn(std::prev(next)->point, p, next->point) <= 0) return false;
			auto i = nodes.insert(next, Node{p, next == nodes.end() ? p : next->point, next == nodes.end()});
			if (next != nodes.end()) area += cross(p, next->point);
			if (i != nodes.begin()) {
				auto prev = std::prev(i);
				if (next != nodes.end()) area -= cross(prev->point, next->point);
				area += cross(prev->point, p);
				prev->next = p, prev->last = false;
			}
			while (i != nodes.begin()) {
				auto prev = std::prev(i);
				if (prev == nodes.begin() || turn(std::prev(prev)->point, prev->point, p) > 0) break;
				erase(prev);
			}
			while (std::next(i) != nodes.end()) {
				auto j = std::next(i), k = std::next(j);
				if (k == nodes.end() || turn(p, j->point, k->point) > 0) break;
				erase(j);
			}
			return true;
		}
		int side(P p, bool upper) const {
			if (nodes.empty() || p.x < nodes.begin()->point.x || p.x > nodes.rbegin()->point.x) return -1;
			auto i = nodes.lower_bound(P{p.x, std::numeric_limits<T>::lowest()});
			W value;
			if (i != nodes.end() && i->point.x == p.x) value = W(p.y) - i->point.y;
			else value = cross(std::prev(i)->point, i->point, p);
			if (upper) value = -value;
			return (value > 0) - (value < 0);
		}
	};
	Chain lower_, upper_;
	static void check(P p) {
		[[maybe_unused]] constexpr W limit = W(1000000000000000000LL);
		assert(-limit <= W(p.x) && W(p.x) <= limit && -limit <= W(p.y) && W(p.y) <= limit);
	}
	std::optional<P> support_impl(Direction d) const {
		if (empty()) return std::nullopt;
		const auto &nodes = d.y < 0 ? lower_.nodes : upper_.nodes;
		if (!d.y) return d.x > 0 ? nodes.rbegin()->point : nodes.begin()->point;
		return nodes.lower_bound(d)->point;
	}
public:
	using point_type = P;
	using value_type = W;
	DynamicConvexHull() = default;
	explicit DynamicConvexHull(const std::vector<P> &points) { for (auto p : points) insert(p); }
	DynamicConvexHull(const DynamicConvexHull &) = default;
	DynamicConvexHull(DynamicConvexHull &&other) noexcept { swap(other); }
	DynamicConvexHull &operator=(DynamicConvexHull other) noexcept { swap(other); return *this; }
	void swap(DynamicConvexHull &other) noexcept {
		lower_.nodes.swap(other.lower_.nodes), upper_.nodes.swap(other.upper_.nodes);
		std::swap(lower_.area, other.lower_.area), std::swap(upper_.area, other.upper_.area);
	}
	friend void swap(DynamicConvexHull &a, DynamicConvexHull &b) noexcept { a.swap(b); }

	bool empty() const { return lower_.nodes.empty(); }
	std::size_t size() const {
		if (empty()) return 0;
		auto result = lower_.nodes.size() + upper_.nodes.size();
		bool first = lower_.nodes.begin()->point == upper_.nodes.begin()->point;
		bool last = lower_.nodes.rbegin()->point == upper_.nodes.rbegin()->point;
		return result - first - (last && result > 2);
	}
	void clear() { lower_.nodes.clear(); upper_.nodes.clear(); lower_.area = upper_.area = 0; }
	bool insert(P p) { check(p); bool a = lower_.insert(p, false), b = upper_.insert(p, true); return a || b; }
	std::vector<P> vertices() const {
		std::vector<P> result;
		result.reserve(size());
		for (const auto &node : lower_.nodes) result.push_back(node.point);
		for (auto i = upper_.nodes.rbegin(); i != upper_.nodes.rend(); ++i)
			if (result.empty() || result.back() != i->point) result.push_back(i->point);
		if (result.size() > 1 && result.front() == result.back()) result.pop_back();
		return result;
	}
	W doubled_area() const {
		if (empty()) return 0;
		return lower_.area - upper_.area + cross(lower_.nodes.rbegin()->point, upper_.nodes.rbegin()->point)
			+ cross(upper_.nodes.begin()->point, lower_.nodes.begin()->point);
	}
	Location contains(P p) const {
		check(p);
		if (lower_.side(p, false) < 0 || upper_.side(p, true) < 0) return Location::outside;
		if (p.x == lower_.nodes.begin()->point.x || p.x == lower_.nodes.rbegin()->point.x
			|| !lower_.side(p, false) || !upper_.side(p, true) || !doubled_area()) return Location::boundary;
		return Location::inside;
	}
	std::optional<P> support(P direction) const { check(direction); return support_impl(Direction{direction.x, direction.y}); }
	std::optional<W> max_dot(P direction) const {
		auto p = support(direction);
		return p ? std::optional<W>(dot(*p, direction)) : std::nullopt;
	}
	std::optional<W> min_dot(P direction) const {
		check(direction);
		auto p = support_impl(Direction{-W(direction.x), -W(direction.y)});
		return p ? std::optional<W>(dot(*p, direction)) : std::nullopt;
	}
};
}

#endif
