#include "data_structure/advanced/partially_retroactive_priority_queue.hpp"
#include <algorithm>
#include <cstdlib>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <tuple>

void require(bool ok) { if (!ok) std::abort(); }

template <class Compare, class Time> void random_test(Compare compare) {
	using Queue = PartiallyRetroactivePriorityQueue<long long, Compare, Time>;
	Queue queue(compare);
	std::map<Time, std::optional<long long>> history;
	std::map<Time, long long> actual;
	std::vector<std::tuple<Time, long long, bool>> events;
	long long sum = 0;
	auto callback = [&, calls = std::make_unique<int>(0)](Time time, const long long &value, bool added) {
		++*calls;
		events.emplace_back(time, value, added);
		if (added) require(actual.emplace(time, value).second);
		else {
			auto it = actual.find(time);
			require(it != actual.end() && it->second == value);
			actual.erase(it);
		}
		sum += added ? value : -value;
	};
	auto valid = [&] {
		int count = 0;
		for (const auto &[time, value] : history) { (void)time; count += value ? 1 : -1; if (count < 0) return false; }
		return true;
	};
	auto check = [&] {
		auto less = [&](const auto &a, const auto &b) {
			if (compare(a.first, b.first)) return true;
			if (compare(b.first, a.first)) return false;
			return a.second < b.second;
		};
		std::set<std::pair<long long, Time>, decltype(less)> oracle(less);
		for (const auto &[time, value] : history) {
			if (value) oracle.emplace(*value, time);
			else { require(!oracle.empty()); oracle.erase(oracle.begin()); }
		}
		std::map<Time, long long> expected, visited;
		long long expected_sum = 0;
		for (auto [value, time] : oracle) expected.emplace(time, value), expected_sum += value;
		require(actual == expected && sum == expected_sum && queue.size() == int(expected.size()));
		const auto &constant = queue;
		constant.for_each([&](Time time, const long long &value) { require(visited.emplace(time, value).second); });
		require(visited == expected);
		require(queue.top() == (oracle.empty() ? std::nullopt : std::optional<long long>(oracle.begin()->first)));
	};
	std::mt19937_64 rng(36310496);
	for (int iteration = 0; iteration < 12000; ++iteration) {
		Time time = Time(rng() % 101);
		long long value = static_cast<long long>(rng() % 11) - 5;
		int command = rng() % 7;
		if (command == 6 && iteration % 257) command = rng() % 4;
		auto before = history;
		events.clear();
		bool expected = true, accepted = false;
		if (command <= 1) {
			expected = !history.count(time);
			if (expected) history[time] = command ? std::nullopt : std::optional<long long>(value);
			expected = expected && valid();
			accepted = command ? queue.insert_pop(time, callback) : queue.insert(time, value, callback);
		} else if (command == 2) {
			auto it = history.find(time);
			expected = it != history.end() && it->second.has_value();
			if (expected) it->second = value;
			accepted = queue.set(time, value, callback);
		} else if (command == 3) {
			expected = history.erase(time) && valid();
			accepted = queue.erase(time, callback);
		} else if (command <= 5) {
			std::vector<typename Queue::update_type> updates(history.begin(), history.end());
			if (command == 5) {
				updates.emplace_back(Time(0), std::nullopt);
				expected = false;
			}
			std::shuffle(updates.begin(), updates.end(), rng);
			accepted = queue.assign(updates, callback);
		} else {
			queue.clear(callback); history.clear(); accepted = true;
		}
		if (!expected) { history = std::move(before); require(events.empty()); }
		require(accepted == expected);
		if (command <= 3) {
			require(events.size() <= (command == 2 ? 2u : 1u));
			if (events.size() == 2) require(!std::get<2>(events[0]) && std::get<2>(events[1]));
		}
		check();
	}
	queue.clear(callback); history.clear(); check();
	queue.clear(callback); check();
}

struct Item {
	int priority;
	std::string text;
	Item() = delete;
	Item(int priority, std::string text) : priority(priority), text(std::move(text)) {}
	bool operator==(const Item &) const = default;
};
struct CompareItem {
	int sign;
	CompareItem() = delete;
	explicit CompareItem(int sign) : sign(sign) {}
	bool operator()(const Item &a, const Item &b) const { return sign * a.priority < sign * b.priority; }
};

int main() {
	random_test<std::less<long long>, int>({});
	random_test<std::greater<long long>, int>({});
	random_test<std::less<long long>, std::uint64_t>({});
	PartiallyRetroactivePriorityQueue<Item, CompareItem> queue(CompareItem(1));
	std::map<std::int64_t, Item> contents;
	auto callback = [&](std::int64_t time, const Item &value, bool added) {
		if (added) require(contents.emplace(time, value).second);
		else { auto it = contents.find(time); require(it != contents.end() && it->second == value); contents.erase(it); }
	};
	require(queue.insert(INT64_MIN, Item(1, "early"), callback));
	require(queue.insert(INT64_MAX - 1, Item(1, "late"), callback));
	require(queue.insert_pop(INT64_MAX, callback) && contents.size() == 1 && contents.begin()->second.text == "late");
	require(queue.set(INT64_MIN, Item(2, "changed"), callback) && contents.begin()->second.text == "changed");
	require(queue.set(INT64_MIN, Item(0, "dead"), callback) && contents.begin()->second.text == "late");
	require(queue.erase(INT64_MAX, callback) && contents.size() == 2);
	queue.clear(callback); require(contents.empty());
}
