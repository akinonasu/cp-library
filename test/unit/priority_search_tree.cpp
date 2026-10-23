#include "data_structure/priority_search_tree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

struct Coordinate {
	int value;
	explicit Coordinate(int value) : value(value) {}
	friend bool operator<(Coordinate a, Coordinate b) { return a.value < b.value; }
};

struct Priority {
	int value;
	static inline int comparisons = 0;
	friend bool operator<(Priority a, Priority b) { ++comparisons; return a.value < b.value; }
};

void randomized() {
	std::mt19937 random(819735);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 100, 257}) {
		std::vector<std::pair<int, int>> points;
		for (int i = 0; i < n; ++i) points.emplace_back(int(random() % 21) - 10, int(random() % 21) - 10);
		PrioritySearchTree<int> tree(points);
		std::vector<std::optional<int>> values;
		for (auto p : points) values.emplace_back(p.second);
		for (int step = 0; step < 1500; ++step) {
			if (n) {
				int id = random() % n, value = int(random() % 21) - 10;
				switch (random() % 5) {
				case 0:
					assert(tree.erase(id) == values[id].has_value());
					values[id].reset();
					break;
				case 1: {
					bool inserted = !values[id];
					assert(tree.insert(id, value) == inserted);
					if (inserted) values[id] = value;
					break;
				}
				case 2:
				case 3:
					tree.set(id, value);
					values[id] = value;
					break;
				default:
					if (values[id]) tree.set(id, *tree.get(id));
				}
			}
			if (step % 197 == 0) {
				tree.clear();
				std::fill(values.begin(), values.end(), std::nullopt);
			}
			if (step % 251 == 0) {
				tree.build(points);
				for (int i = 0; i < n; ++i) values[i] = points[i].second;
			}
			int active = 0;
			for (int i = 0; i < n; ++i) {
				assert(tree.x(i) == points[i].first && tree.get(i) == values[i]);
				assert(tree.contains(i) == values[i].has_value());
				active += values[i].has_value();
			}
			assert(tree.size() == active && tree.empty() == (active == 0) && tree.point_count() == n);
			for (int query = 0; query < 8; ++query) {
				int l = int(random() % 27) - 13, r = int(random() % 27) - 13, up = int(random() % 27) - 13;
				std::vector<int> expected;
				std::optional<int> best;
				for (int i = 0; i < n; ++i) if (values[i] && l <= points[i].first && points[i].first < r) {
					if (*values[i] < up) expected.push_back(i);
					if (!best || *values[i] < *values[*best]) best = i;
				}
				auto actual = tree.report(l, r, up);
				std::sort(actual.begin(), actual.end());
				assert(actual == expected && tree.min_y(l, r) == best);
				std::vector<int> enumerated;
				auto callback = [capture = std::make_unique<int>(0), &enumerated](int id) mutable {
					++*capture;
					enumerated.push_back(id);
				};
				tree.enumerate(l, r, up, std::move(callback));
				std::sort(enumerated.begin(), enumerated.end());
				assert(enumerated == expected);
			}
		}
		auto copy = tree;
		auto result = tree.report(-20, 20, 20);
		tree.clear();
		assert(copy.report(-20, 20, 20) == result);
		PrioritySearchTree<int> moved(std::move(copy));
		assert(copy.empty() && copy.point_count() == 0 && !copy.min_y(-20, 20));
		assert(copy.report(-20, 20, 20).empty() && moved.report(-20, 20, 20) == result);
		tree = std::move(moved);
		assert(moved.empty() && moved.point_count() == 0 && tree.report(-20, 20, 20) == result);
		auto &alias = tree;
		tree = alias;
		assert(tree.report(-20, 20, 20) == result);
		moved.build_coordinates(std::vector<int>{3});
		assert(moved.insert(0, 4) && moved.min_y(0, 5) == 0);
	}
}

void output_sensitive() {
	const int n = 32768, k = 64;
	std::vector<std::pair<int, Priority>> points;
	for (int i = 0; i < n; ++i) points.push_back({i, Priority{i % (n / k) == 0 ? 0 : 1}});
	PrioritySearchTree<int, Priority> tree(points);
	Priority::comparisons = 0;
	auto ids = tree.report(0, n, Priority{1});
	assert(ids.size() == k && Priority::comparisons < 4 * k);
	Priority::comparisons = 0;
	assert(tree.report(0, n, Priority{0}).empty() && Priority::comparisons == 1);
}

void large() {
	const int n = 100003;
	std::vector<std::pair<int, int>> points;
	for (int i = 0; i < n; ++i) points.emplace_back(i, -i);
	PrioritySearchTree<int> tree(points);
	assert(tree.report(0, n, 1).size() == std::size_t(n));
	for (int i = n - 1; i >= 0; --i) {
		assert(tree.min_y(0, n) == i);
		assert(tree.erase(i));
	}
	assert(tree.empty());
	for (int i = n - 1; i >= 0; --i) tree.insert(i, 0);
	for (int i = 0; i < n; ++i) {
		assert(tree.min_y(0, n) == i);
		assert(tree.report(i, i + 1, 1) == std::vector<int>{i});
		tree.erase(i);
	}
	for (int i = 0; i < n; ++i) tree.set(i, i);
	for (int i = 0; i < n; ++i) {
		assert(tree.min_y(i, n) == i);
		assert(tree.erase(i));
	}
}

int main() {
	randomized();
	large();
	output_sensitive();
	PrioritySearchTree<> empty;
	assert(empty.empty() && empty.report(0, 1, 1).empty() && !empty.min_y(0, 1));
	PrioritySearchTree<std::string, Coordinate> generic(std::vector<std::string>{"b", "a", "b"});
	generic.insert(0, Coordinate(7));
	generic.insert(1, Coordinate(-2));
	generic.insert(2, Coordinate(7));
	assert(generic.min_y("b", "c") == 0);
	assert(generic.report("a", "b", Coordinate(-1)) == std::vector<int>{1});
	generic.set(0, Coordinate(10));
	assert(generic.min_y("b", "c") == 2);
	generic.build(std::vector<std::pair<std::string, Coordinate>>{ {"c", Coordinate(2)}, {"a", Coordinate(0)} });
	assert(generic.min_y("a", "z") == 1);
	using I = std::int64_t;
	using U = std::uint64_t;
	const I lo = std::numeric_limits<I>::min(), hi = std::numeric_limits<I>::max();
	const U top = std::numeric_limits<U>::max();
	PrioritySearchTree<I, U> extremes(std::vector<std::pair<I, U>>{ {lo, top}, {hi, 0}, {0, top - 1} });
	assert(extremes.report(lo, hi, top) == std::vector<int>{2});
	assert(extremes.min_y(lo, hi) == 2 && !extremes.min_y(hi, hi));
	extremes.erase(2);
	assert(extremes.min_y(lo, hi) == 0 && extremes.report(lo, hi, top).empty());
	extremes.build_coordinates(std::vector<I>{hi, lo});
	assert(extremes.empty() && extremes.point_count() == 2);
}
