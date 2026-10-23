#include "data_structure/flat_hash_map.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

struct ConstantHash {
	std::size_t operator()(int) const { return 0; }
};
struct Value {
	int number;
	Value() = delete;
	explicit Value(int number) : number(number) {}
};

template<class K> void randomized() {
	FlatHashMap<K, std::uint64_t> map;
	std::map<K, std::uint64_t> expected;
	std::mt19937_64 random(929039);
	for (int step = 0; step < 100000; ++step) {
		K key = static_cast<K>(random() % 2048);
		std::uint64_t value = random();
		switch (random() % 8) {
		case 0: case 1: case 2:
			map.set(key, value), expected[key] = value;
			break;
		case 3:
			assert(map.erase(key) == bool(expected.erase(key)));
			break;
		case 4:
			map[key] += value, expected[key] += value;
			break;
		case 5:
			map.reserve(random() % 8192);
			break;
		case 6:
			if (step % 127 == 0) map.clear(), expected.clear();
			break;
		default:
			if (!expected.empty()) {
				auto it = expected.begin();
				map.set(key, map.at(it->first));
				expected[key] = it->second;
			}
		}
		const auto &view = map;
		auto it = expected.find(key);
		assert(map.size() == expected.size() && map.empty() == expected.empty());
		assert(view.contains(key) == expected.contains(key));
		assert(view.count(key) == expected.count(key));
		assert(bool(view.find(key)) == (it != expected.end()));
		assert(view.get(key) == (it == expected.end() ? std::nullopt : std::optional(it->second)));
		if (it != expected.end()) assert(view.at(key) == it->second && *map.find(key) == it->second);
		else {
			try { map.at(key); assert(false); } catch (const std::out_of_range &) {}
		}
		if (step % 701 == 0) {
			auto copy = map;
			auto moved = std::move(copy);
			assert(copy.empty() && moved.size() == expected.size());
			copy = moved;
			auto *self = &moved;
			moved = std::move(*self);
			for (auto [k, v] : expected) assert(copy.at(k) == v && moved.at(k) == v);
			map.swap(moved);
		}
	}
	K lo = (std::numeric_limits<K>::min)(), hi = (std::numeric_limits<K>::max)();
	map.set(lo, 1), map.set(hi, 2);
	assert(map.at(lo) == 1 && map.at(hi) == 2);
	try { map.reserve((std::numeric_limits<std::size_t>::max)()); assert(false); }
	catch (const std::length_error &) {}
	assert(map.at(lo) == 1 && map.at(hi) == 2);
}

int main() {
	randomized<std::int32_t>();
	randomized<std::uint32_t>();
	randomized<std::int64_t>();
	randomized<std::uint64_t>();
	FlatHashMap<std::string, std::string> words;
	words.set("ab", "cd");
	words["ef"] = "gh";
	assert(words.get("ab") == "cd" && words.get("ef") == "gh" && !words.get("missing"));
	auto copy = words;
	words.erase("ab");
	assert(copy.at("ab") == "cd" && !words.contains("ab"));
	auto moved = std::move(copy);
	assert(copy.empty() && moved.at("ab") == "cd");
	FlatHashMap<int, Value> values;
	values.set(1, Value(2));
	assert(values.at(1).number == 2);
	FlatHashMap<int, std::unique_ptr<int>, ConstantHash> pointers;
	pointers[3] = std::make_unique<int>(7);
	assert(*pointers.at(3) == 7 && pointers.erase(3));
	FlatHashMap<int, int, ConstantHash> collisions;
	for (int i = 0; i < 1000; ++i) collisions.set(i, i * i);
	for (int i = 0; i < 1000; i += 2) collisions.erase(i);
	for (int i = 1; i < 1000; i += 2) assert(collisions.at(i) == i * i);
}
