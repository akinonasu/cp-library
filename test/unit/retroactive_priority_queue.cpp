#include "data_structure/priority_queue/retroactive_priority_queue.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

template <class T, class Compare, class Time = std::int64_t>
struct Oracle {
	std::map<Time, std::optional<T>> operations;
	Compare compare;
	struct Less {
		Compare compare;
		bool operator()(const std::pair<T, Time> &a, const std::pair<T, Time> &b) const {
			if (compare(a.first, b.first)) return true;
			if (compare(b.first, a.first)) return false;
			return a.second < b.second;
		}
	};
	bool valid() const {
		int n = 0;
		for (const auto &[t, value] : operations) { (void)t; n += value ? 1 : -1; if (n < 0) return false; }
		return true;
	}
	std::pair<int, std::optional<T>> query(Time time) const {
		std::set<std::pair<T, Time>, Less> queue(Less{compare});
		for (const auto &[t, value] : operations) {
			if (time < t) break;
			if (value) queue.emplace(*value, t);
			else { assert(!queue.empty()); queue.erase(queue.begin()); }
		}
		return {int(queue.size()), queue.empty() ? std::nullopt : std::optional<T>(queue.begin()->first)};
	}
};

template <class Compare>
void random_test(Compare compare, std::mt19937_64 &rng) {
	using Queue = RetroactivePriorityQueue<long long, Compare, std::int64_t, 8>;
	Queue queue(compare);
	Oracle<long long, Compare> oracle{{}, compare};
	queue.reserve(300);
	for (int iteration = 0; iteration < 16000; ++iteration) {
		std::int64_t time = std::int64_t(rng() % 601) - 300;
		long long value = std::int64_t(rng() % 31) - 15;
		int command = rng() % 5;
		auto before = oracle.operations;
		bool expected, actual;
		if (command < 2) {
			expected = !oracle.operations.count(time);
			if (expected) oracle.operations[time] = command ? std::nullopt : std::optional<long long>(value);
			expected = expected && oracle.valid();
			actual = command ? queue.insert_pop(time) : queue.insert(time, value);
		} else if (command == 2) {
			expected = oracle.operations.erase(time) != 0 && oracle.valid();
			actual = queue.erase(time);
		} else {
			auto it = oracle.operations.find(time);
			expected = it != oracle.operations.end() && it->second.has_value();
			if (expected) it->second = value;
			actual = queue.set(time, value);
		}
		if (!expected) oracle.operations = std::move(before);
		assert(actual == expected);
		assert(queue.operation_count() == int(oracle.operations.size()));
		auto [size, top] = oracle.query(INT64_MAX);
		assert(queue.size() == size && queue.top() == top && queue.top() == top && queue.empty() == (size == 0));
		assert(queue.contains_time(time) == bool(oracle.operations.count(time)));
		for (int j = 0; j < 3; ++j) {
			std::int64_t t = j ? std::int64_t(rng() % 701) - 350 : time;
			auto [n, minimum] = oracle.query(t);
			assert(queue.size(t) == n && queue.top(t) == minimum && queue.empty(t) == (n == 0));
		}
		if (iteration % 701 == 0) {
			auto copy = queue;
			assert(copy.top() == top);
			Queue moved(std::move(copy));
			assert(copy.empty() && !copy.operation_count() && moved.top() == top);
			assert(copy.insert(0, 5) && copy.top() == std::optional<long long>(5));
			copy = std::move(moved);
			assert(moved.empty() && copy.top() == top);
			moved = copy;
			assert(moved.top(time) == queue.top(time));
		}
	}
	std::vector<std::int64_t> times;
	for (const auto &[time, value] : oracle.operations) if (!value) times.push_back(time);
	for (auto time : times) assert(queue.erase(time));
	times.clear();
	for (const auto &[time, value] : oracle.operations) if (value) times.push_back(time);
	std::shuffle(times.begin(), times.end(), rng);
	for (auto time : times) assert(queue.erase(time));
	assert(queue.empty() && !queue.operation_count() && !queue.top());
	assert(queue.insert(7, 42) && queue.top(7) == std::optional<long long>(42));
	queue.clear();
	assert(queue.empty() && !queue.operation_count() && !queue.top(100));
}

struct Item {
	int priority;
	std::string payload;
	Item() = delete;
	Item(int priority, std::string payload) : priority(priority), payload(std::move(payload)) {}
	bool operator==(const Item &) const = default;
};
struct CompareItem {
	int sign;
	CompareItem() = delete;
	explicit CompareItem(int sign) : sign(sign) {}
	bool operator()(const Item &a, const Item &b) const { return sign * a.priority < sign * b.priority; }
};

int main() {
	std::mt19937_64 rng(49656811);
	random_test(std::less<long long>{}, rng);
	random_test(std::greater<long long>{}, rng);
	for (int code = 0; code < 16384; ++code) {
		RetroactivePriorityQueue<int, std::less<int>, int, 8> queue;
		Oracle<int, std::less<int>, int> oracle{{}, {}};
		int state = code;
		for (int time = 0; time < 7; ++time, state /= 4) {
			int kind = state % 4;
			if (kind < 2) { assert(queue.insert(time, kind)); oracle.operations[time] = kind; }
			else if (kind == 2 && queue.size()) { assert(queue.insert_pop(time)); oracle.operations[time] = std::nullopt; }
		}
		for (int time = -1; time <= 7; ++time) {
			auto [n, top] = oracle.query(time);
			assert(queue.size(time) == n && queue.top(time) == top);
		}
	}
	RetroactivePriorityQueue<Item, CompareItem, std::int64_t, 8> items(CompareItem(1));
	assert(items.insert(20, Item(1, "late")));
	assert(items.insert(-5, Item(1, "early")));
	assert(items.insert_pop(30));
	assert(items.top()->payload == "late" && items.top(0)->payload == "early");
	assert(!items.insert_pop(-6) && !items.set(30, Item(3, "invalid")) && items.top()->payload == "late");
	assert(items.set(-5, Item(2, "changed")) && items.top()->payload == "changed");
	assert(items.erase(30) && items.top()->payload == "late");
	RetroactivePriorityQueue<long long, std::less<long long>, std::uint64_t, 8> wide;
	assert(wide.insert(0, LLONG_MAX) && wide.insert(UINT64_MAX - 1, LLONG_MIN));
	assert(wide.insert_pop(UINT64_MAX));
	assert(wide.top(UINT64_MAX - 1) == LLONG_MIN && wide.top(UINT64_MAX) == LLONG_MAX);
	using Bulk = RetroactivePriorityQueue<int, std::less<int>, int, 8>;
	std::vector<Bulk::update_type> history;
	for (int i = 0; i < 200; ++i) history.emplace_back(2 * i, i);
	for (int i = 0; i < 199; ++i) history.emplace_back(2 * i + 1, std::nullopt);
	std::shuffle(history.begin(), history.end(), rng);
	Bulk bulk(history);
	assert(bulk.top() == 199 && bulk.top(200) == 100 && bulk.size(200) == 1);
	auto bad = history;
	bad.emplace_back(-1, std::nullopt);
	assert(!bulk.assign(bad) && bulk.top() == 199);
	bad = history; bad.push_back(history[0]);
	assert(!bulk.assign(bad) && bulk.size() == 1);
	assert(bulk.assign({}) && bulk.empty() && !bulk.operation_count());
	RetroactivePriorityQueue<int> reuse;
	std::size_t bytes = 0;
	for (int i = 0; i < 5000; ++i) {
		assert(reuse.insert(INT64_MIN, i));
		if (!i) bytes = reuse.memory_usage();
		assert(reuse.memory_usage() == bytes);
		assert(reuse.erase(INT64_MIN) && reuse.memory_usage() <= bytes);
	}
	RetroactivePriorityQueue<int, std::less<int>, int, 8> cascade;
	for (int i = 0; i < 300; ++i) assert(cascade.insert(2 * i, 300 - i));
	for (int i = 0; i < 299; ++i) assert(cascade.insert_pop(2 * i + 1));
	assert(cascade.top() == 1);
	assert(cascade.set(0, -1000) && cascade.top() == 1);
	assert(!cascade.insert_pop(-1) && !cascade.erase(0));
	assert(cascade.erase(1) && cascade.erase(0));
	assert(cascade.top() == 1);
}
