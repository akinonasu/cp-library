#include "data_structure/flat_hash_map.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <utility>

struct IteratorConstantHash {
	std::size_t operator()(int) const { return 0; }
};

struct CaseHash {
	std::size_t operator()(const std::string &s) const {
		std::size_t h = 0;
		for (unsigned char c : s) h = h * 131 + (c >= 'A' && c <= 'Z' ? c + 32 : c);
		return h;
	}
};
struct CaseEqual {
	bool operator()(const std::string &a, const std::string &b) const {
		if (a.size() != b.size()) return false;
		for (std::size_t i = 0; i < a.size(); ++i) {
			auto lower = [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; };
			if (lower(a[i]) != lower(b[i])) return false;
		}
		return true;
	}
};

struct alignas(64) IteratorPayload {
	std::uint64_t data[8]{};
	friend bool operator==(const IteratorPayload &, const IteratorPayload &) = default;
};

template <class V> V make_value(std::uint64_t x) {
	if constexpr (std::is_same_v<V, IteratorPayload>) {
		V value;
		for (int i = 0; i < 8; ++i) value.data[i] = x + i;
		return value;
	} else return V(x);
}

template <class Map> void check_traits() {
	using I = typename Map::iterator;
	using C = typename Map::const_iterator;
	using K = typename Map::key_type;
	using V = typename Map::mapped_type;
	static_assert(std::is_same_v<decltype(std::declval<Map &>().find(std::declval<const K &>())), I>);
	static_assert(std::is_same_v<decltype(std::declval<const Map &>().find(std::declval<const K &>())), C>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().begin()), I>);
	static_assert(std::is_same_v<decltype(std::declval<const Map &>().begin()), C>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().cbegin()), C>);
	static_assert(std::is_convertible_v<I, C> && !std::is_convertible_v<C, I>);
	static_assert(!std::is_assignable_v<decltype((std::declval<I>()->first)), K>);
	static_assert(std::is_same_v<decltype((std::declval<I>()->second)), V &>);
	static_assert(std::is_same_v<decltype((std::declval<C>()->second)), const V &>);
	static_assert(std::is_same_v<decltype(++std::declval<I &>()), I &>);
	static_assert(std::is_same_v<decltype(std::declval<I &>()++), I>);
	static_assert(std::is_same_v<typename std::iterator_traits<I>::value_type, typename Map::value_type>);
	I a, b;
	C c;
	assert(a == b && a == c && c == a);
}

template <class Map> void compare_all(Map &map, const std::map<typename Map::key_type, typename Map::mapped_type> &expected) {
	const auto &view = map;
	using C = typename Map::const_iterator;
	assert((map.begin() == map.end()) == expected.empty());
	assert(view.begin() == map.cbegin() && map.cend() == view.end());
	assert(map.end() == view.end() && view.end() == map.end());
	assert(std::distance(map.begin(), map.end()) == static_cast<std::ptrdiff_t>(expected.size()));
	std::map<typename Map::key_type, typename Map::mapped_type> actual;
	for (const auto &[key, value] : view) assert(actual.emplace(key, value).second);
	assert(actual == expected);
	std::size_t seen = 0;
	for (auto it = map.begin(); it != map.end();) {
		auto copy = it;
		C const_copy = copy;
		auto key = copy->first;
		auto *value = std::addressof(copy->second);
		assert(copy == const_copy && const_copy == copy && copy == view.find(key));
		assert(std::addressof(const_copy->second) == value);
		auto previous = it++;
		assert(previous == copy && previous->first == key && previous->second == expected.at(key));
		assert(std::addressof(previous->second) == value);
		assert(copy->first == key && std::addressof(copy->second) == value);
		++const_copy;
		assert(it == const_copy && const_copy == it);
		++seen;
	}
	assert(seen == expected.size());
	for (const auto &[key, value] : expected) {
		auto it = map.find(key);
		assert(it != map.end() && it->first == key && it->second == value);
		auto const_it = view.find(key);
		assert(it == const_it && const_it == it);
		auto cursor = it;
		std::size_t tail = 0;
		for (; cursor != map.end(); ++cursor) {
			assert(expected.at(cursor->first) == cursor->second);
			assert(++tail <= expected.size());
		}
	}
}

template <class K, class V, class Hash = std::hash<K>> void random_traversal() {
	using Map = FlatHashMap<K, V, Hash>;
	check_traits<Map>();
	Map map;
	std::map<K, V> expected;
	compare_all(map, expected);
	map.reserve(8192);
	assert(map.begin() == map.end() && map.find(K(3)) == map.end());
	std::mt19937_64 rng(194231);
	for (int step = 0; step < 1600; ++step) {
		K key = K(rng() % 73);
		switch (rng() % 5) {
		case 0: case 1: {
			V value = make_value<V>(rng());
			map.set(key, value);
			expected[key] = value;
			break;
		}
		case 2: assert(map.erase(key) == bool(expected.erase(key))); break;
		case 3:
			map[key] = make_value<V>(step);
			expected[key] = make_value<V>(step);
			break;
		case 4:
			if (step % 37 == 0) map.clear(), expected.clear();
			break;
		}
		if (step % 41 == 0) compare_all(map, expected);
	}
	for (auto &&[key, value] : map) value = expected[key] = make_value<V>(std::uint64_t(key) + 1000);
	compare_all(map, expected);
	if (!expected.empty()) {
		auto it = map.begin();
		auto &&entry = *it;
		auto *mapped_address = std::addressof(entry.second);
		assert(mapped_address == std::addressof(it->second));
		assert(entry.first == it->first && entry.second == it->second);
		{
			auto cursor = it;
			auto &&borrowed = *cursor;
			auto key = borrowed.first;
			auto *value = std::addressof(borrowed.second);
			++cursor;
			assert(borrowed.first == key && std::addressof(borrowed.second) == value);
		}
		typename Map::value_type copied(*it);
		copied.second = make_value<V>(4);
		assert(it->second == expected.at(it->first));
		const auto fixed = map.begin();
		fixed->second = expected[fixed->first] = make_value<V>(100);
		V *saved = nullptr;
		typename Map::key_type saved_key{};
		{
			auto cursor = map.begin();
			auto &&[key, value] = *cursor;
			saved_key = key;
			saved = std::addressof(value);
		}
		assert(saved == std::addressof(map.at(saved_key)) && *saved == expected.at(saved_key));
		auto assigned = map.end();
		{
			auto temporary = it;
			assigned = temporary;
		}
		assert(assigned == it && assigned->first == it->first);
		assigned = it;
		assert(assigned == it && assigned->second == it->second);
		auto moved = std::move(assigned);
		assert(moved == it && moved->second == it->second);
	}
	auto copy = map;
	compare_all(copy, expected);
	auto moved = std::move(copy);
	assert(copy.begin() == copy.end());
	compare_all(moved, expected);
	Map empty;
	map.swap(empty);
	assert(map.begin() == map.end());
	compare_all(empty, expected);
	map.reserve(16384);
	assert(map.begin() == map.end());
}

int main() {
	random_traversal<std::uint64_t, std::uint64_t>();
	random_traversal<std::uint32_t, std::uint64_t>();
	random_traversal<std::uint8_t, IteratorPayload>();
	random_traversal<int, std::uint64_t, IteratorConstantHash>();
	using Words = FlatHashMap<std::string, std::string, CaseHash, CaseEqual>;
	check_traits<Words>();
	Words words;
	words.set("Alpha", "one");
	words.set("ALPHA", "two");
	words.set("Beta", "three");
	assert(words.size() == 2 && words.find("aLpHa")->second == "two");
	std::size_t total = 0;
	for (auto &&[key, value] : words) {
		assert(key == "Alpha" || key == "Beta");
		value += "!";
		++total;
	}
	assert(total == 2 && words.find("ALpha")->second == "two!");
	assert(words.find("Gamma") == words.end());
	FlatHashMap<int, std::unique_ptr<int>> pointers;
	check_traits<decltype(pointers)>();
	pointers[3] = std::make_unique<int>(7);
	pointers[9] = std::make_unique<int>(11);
	for (auto &&[key, value] : pointers) *value += key;
	auto it = pointers.find(3), copied = it;
	assert(copied == it && *copied->second == 10);
	assert(*pointers.find(9)->second == 20);
	const auto &view = pointers;
	assert(view.find(3) == it && view.find(3)->first == 3);
}
