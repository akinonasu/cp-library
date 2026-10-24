#ifndef OFFLINE_RECTANGLE_SUM_HPP
#define OFFLINE_RECTANGLE_SUM_HPP

#include "../components/fenwick_sum.hpp"
#include "../components/radix_sort.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

template <class X = std::int64_t, class Y = X, class Sum = std::int64_t>
class OfflineRectangleSum {
	using Accum = std::conditional_t<std::is_integral_v<Sum> && sizeof(Sum) <= 8, __int128_t, Sum>;
	struct Point { X x; Y y; Sum weight; int rank = 0; };
	struct Query { X left, right; Y down, up; };
	struct Event { X x; int down, up, query; bool subtract; };
	std::vector<Point> points;
	std::vector<Query> queries;
	template <class A>
	static void sweep(const std::vector<Point> &ordered, const std::vector<Event> &events, int size, std::vector<Accum> &answers) {
		FenwickSum<A> tree(size);
		std::size_t p = 0;
		for (const auto &event : events) {
			while (p < ordered.size() && ordered[p].x < event.x) {
				tree.add(ordered[p].rank, A(ordered[p].weight));
				++p;
			}
			auto value = tree.template prod_as<Accum>(event.down, event.up);
			if (event.subtract) answers[event.query] -= value;
			else answers[event.query] += value;
		}
	}

public:
	explicit OfflineRectangleSum(int point_capacity = 0, int query_capacity = 0) {
		assert(point_capacity >= 0 && query_capacity >= 0);
		points.reserve(point_capacity); queries.reserve(query_capacity);
	}
	void add_point(const X &x, const Y &y, const Sum &weight) {
		assert(points.size() < std::size_t(std::numeric_limits<int>::max()));
		points.push_back({x, y, weight});
	}
	int add_query(const X &left, const Y &down, const X &right, const Y &up) {
		assert(queries.size() < std::size_t(std::numeric_limits<int>::max()));
		queries.push_back({left, right, down, up});
		return int(queries.size() - 1);
	}
	std::vector<Sum> solve() const {
		if (points.empty() || queries.empty()) return std::vector<Sum>(queries.size());
		std::vector<Accum> answers(queries.size());
		auto ordered = points;
		radix_sort_detail::sort_by_key(ordered, [](const Point &p) -> const Y & { return p.y; });
		std::vector<Y> ys;
		ys.reserve(points.size());
		for (auto &p : ordered) {
			if (ys.empty() || ys.back() < p.y) ys.push_back(p.y);
			p.rank = int(ys.size() - 1);
		}
		radix_sort_detail::sort_by_key(ordered, [](const Point &p) -> const X & { return p.x; });
		std::vector<Event> events;
		events.reserve(2 * queries.size());
		for (int i = 0; i < int(queries.size()); ++i) {
			const auto &q = queries[i];
			if (!(q.left < q.right) || !(q.down < q.up)) continue;
			int down = std::lower_bound(ys.begin(), ys.end(), q.down) - ys.begin();
			int up = std::lower_bound(ys.begin(), ys.end(), q.up) - ys.begin();
			if (down == up) continue;
			events.push_back({q.left, down, up, i, true});
			events.push_back({q.right, down, up, i, false});
		}
		radix_sort_detail::sort_by_key(events, [](const Event &e) -> const X & { return e.x; });
		bool narrow = false;
		if constexpr (std::is_integral_v<Sum> && sizeof(Sum) <= 8) {
			Accum positive = 0, negative = 0;
			for (const auto &point : points) {
				Accum value(point.weight);
				if (value > 0) positive += value;
				else negative += value;
			}
			narrow = positive <= INT64_MAX && negative >= INT64_MIN;
		}
		if (narrow) {
			if constexpr (std::is_integral_v<Sum> && sizeof(Sum) <= 8) sweep<std::int64_t>(ordered, events, int(ys.size()), answers);
		} else sweep<Accum>(ordered, events, int(ys.size()), answers);

		if constexpr (std::is_same_v<Accum, Sum>) return answers;
		else return std::vector<Sum>(answers.begin(), answers.end());
	}
};

#endif
