#include "data_structure/double_ended_priority_queue.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <type_traits>

struct Direction {
	bool reverse;
	bool operator()(int a, int b) const { return reverse ? a > b : a < b; }
};

template<class Queue, class Set>
void check(const Queue& queue, const Set& values) {
	assert(queue.size() == values.size() && queue.empty() == values.empty());
	if (!values.empty()) {
		assert(queue.min() == *values.begin());
		assert(queue.max() == *values.rbegin());
	}
}

template<class Compare>
void randomized(Compare compare) {
	std::mt19937 random(20261111);
	for (int n : {0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32, 63, 64, 255, 256, 4095, 4096}) {
		std::vector<int> initial(n);
		for (int& x : initial) x = static_cast<int>(random() % 257) - 128;
		DoubleEndedPriorityQueue<int, Compare> queue(initial, compare);
		std::multiset<int, Compare> values(initial.begin(), initial.end(), compare);
		queue.reserve(n + 1000);
		for (int step = 0; step < 10000; ++step) {
			check(queue, values);
			int operation = random() % 4;
			if (values.empty() || operation < 2) {
				int value = static_cast<int>(random() % 257) - 128;
				queue.push(value);
				values.insert(value);
			} else if (operation == 2) {
				assert(queue.pop_min() == *values.begin());
				values.erase(values.begin());
			} else {
				auto it = std::prev(values.end());
				assert(queue.pop_max() == *it);
				values.erase(it);
			}
			if (step % 1024 == 0) {
				auto copy = queue;
				for (int value : values) assert(copy.pop_min() == value);
				assert(copy.empty());
			}
		}
		check(queue, values);
		while (!values.empty()) {
			assert(queue.pop_min() == *values.begin());
			values.erase(values.begin());
		}
		assert(queue.empty());
		queue.clear();
		queue.push(7);
		assert(queue.pop_max() == 7 && queue.empty());
	}
}

void exhaustive() {
	int states = 1;
	for (int n = 0; n <= 7; ++n, states *= 3) {
		for (int code = 0; code < states; ++code) {
			std::vector<int> input(n);
			for (int i = 0, value = code; i < n; ++i, value /= 3) input[i] = value % 3 - 1;
			auto sorted = input;
			std::sort(sorted.begin(), sorted.end());
			DoubleEndedPriorityQueue<int> built(input), pushed;
			for (int x : input) pushed.push(x);
			for (int mask = 0; mask < (1 << n); ++mask) {
				auto a = built, b = pushed;
				int first = 0, last = n;
				for (int step = 0; step < n; ++step) {
					assert(a.min() == sorted[first] && b.min() == sorted[first]);
					assert(a.max() == sorted[last - 1] && b.max() == sorted[last - 1]);
					if ((mask >> step) & 1) {
						assert(a.pop_min() == sorted[first]);
						assert(b.pop_min() == sorted[first++]);
					} else {
						assert(a.pop_max() == sorted[last - 1]);
						assert(b.pop_max() == sorted[--last]);
					}
				}
				assert(a.empty() && b.empty());
			}
		}
	}
}

struct PointerOrder {
	bool reverse;
	bool operator()(const std::unique_ptr<int>& a, const std::unique_ptr<int>& b) const {
		return reverse ? *a > *b : *a < *b;
	}
};

int main() {
	static_assert(std::is_same_v<DoubleEndedPriorityQueue<int>::value_type, int>);
	static_assert(std::is_same_v<DoubleEndedPriorityQueue<int>::size_type, std::size_t>);
	static_assert(std::is_same_v<decltype(std::declval<const DoubleEndedPriorityQueue<int>&>().min()), const int&>);
	exhaustive();
	randomized(std::less<int>{});
	randomized(std::greater<int>{});
	randomized(Direction{false});
	randomized(Direction{true});
	DoubleEndedPriorityQueue<std::string> words(std::vector<std::string>{"z", "", "a", "z", "long string"});
	words.push(words.min());
	assert(words.pop_min().empty() && words.pop_min().empty());
	assert(words.pop_max() == "z" && words.pop_max() == "z");
	assert(words.pop_min() == "a" && words.pop_max() == "long string");
	assert(words.empty());
	using U = std::int64_t;
	DoubleEndedPriorityQueue<U> extremes(std::vector<U>{0, std::numeric_limits<U>::min(), std::numeric_limits<U>::max()});
	assert(extremes.pop_min() == std::numeric_limits<U>::min());
	assert(extremes.pop_max() == std::numeric_limits<U>::max());
	assert(extremes.pop_min() == 0 && extremes.empty());
	DoubleEndedPriorityQueue<int, Direction> ascending(std::vector<int>{3, 1, 5}, Direction{false});
	DoubleEndedPriorityQueue<int, Direction> descending(std::vector<int>{3, 1, 5}, Direction{true});
	auto copy = ascending;
	copy.push(7);
	assert(ascending.max() == 5 && copy.pop_max() == 7);
	descending = ascending;
	assert(descending.pop_min() == 1 && ascending.size() == 3);
	descending = DoubleEndedPriorityQueue<int, Direction>(std::vector<int>{-1, 2}, Direction{true});
	swap(ascending, descending);
	assert(ascending.pop_min() == 2 && descending.pop_min() == 1);
	auto moved = std::move(ascending);
	assert(ascending.empty() && moved.pop_min() == -1);
	ascending.push(4);
	ascending.push(9);
	assert(ascending.pop_min() == 9);
	std::vector<std::unique_ptr<int>> pointers;
	for (int i : {3, 7, -2, 7, 0}) pointers.push_back(std::make_unique<int>(i));
	DoubleEndedPriorityQueue<std::unique_ptr<int>, PointerOrder> owner(std::move(pointers), PointerOrder{true});
	owner.push(std::make_unique<int>(11));
	assert(*owner.pop_min() == 11 && *owner.pop_max() == -2);
	auto transferred = std::move(owner);
	assert(owner.empty() && *transferred.min() == 7);
	owner.push(std::make_unique<int>(17));
	transferred = std::move(owner);
	assert(owner.empty() && *transferred.pop_max() == 17 && transferred.empty());
	owner.push(std::make_unique<int>(1));
	owner.clear();
	owner.push(std::make_unique<int>(2));
	assert(*owner.pop_min() == 2 && owner.empty());
}
