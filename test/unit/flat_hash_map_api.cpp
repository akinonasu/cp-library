#include "data_structure/flat_hash_map.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

struct ApiConstantHash {
	std::size_t operator()(int) const { return 0; }
};

struct ApiPayload {
	std::array<std::uint64_t, 4> data{};
	friend bool operator==(const ApiPayload &, const ApiPayload &) = default;
};

template <class V> V api_value(std::uint64_t x) {
	if constexpr (std::is_same_v<V, ApiPayload>) return { {x, x + 1, x + 2, x + 3} };
	else return static_cast<V>(x);
}

template <class Map, class Oracle> void check_api_map(const Map &map, const Oracle &expected) {
	assert(map.size() == expected.size() && map.empty() == expected.empty());
	std::size_t count = 0;
	for (const auto &[key, value] : map) {
		auto it = expected.find(key);
		assert(it != expected.end() && value == it->second);
		++count;
	}
	assert(count == expected.size());
	for (const auto &[key, value] : expected) {
		auto it = map.find(key);
		assert(it != map.end() && it->second == value);
	}
}

template <class K, class V, class Hash = std::hash<K>> void random_api() {
	using Map = FlatHashMap<K, V, Hash>;
	using Result = std::pair<typename Map::iterator, bool>;
	using ConstIterator = typename Map::const_iterator;
	static_assert(std::is_same_v<decltype(std::declval<Map &>().insert(std::declval<const typename Map::value_type &>())), Result>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().insert(std::declval<typename Map::value_type &&>())), Result>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().try_emplace(K{}, V{})), Result>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().insert_or_assign(K{}, V{})), Result>);
	static_assert(std::is_same_v<decltype(std::declval<Map &>().erase(std::declval<ConstIterator>())), typename Map::iterator>);
	Map map;
	std::unordered_map<K, V, Hash> expected;
	std::mt19937_64 rng(483691);
	for (int step = 0; step < 9000; ++step) {
		K key = static_cast<K>(rng() % 257);
		V value = api_value<V>(rng() % 100000);
		switch (rng() % 9) {
		case 0: {
			const typename Map::value_type entry{key, value};
			auto a = map.insert(entry);
			auto b = expected.insert(entry);
			assert(a.second == b.second && a.first->first == b.first->first && a.first->second == b.first->second);
			break;
		}
		case 1: {
			auto a = map.insert({key, value});
			auto b = expected.insert({key, value});
			assert(a.second == b.second && a.first->first == key && a.first->second == b.first->second);
			break;
		}
		case 2: {
			auto a = map.try_emplace(key, value);
			auto b = expected.try_emplace(key, value);
			assert(a.second == b.second && a.first->first == key && a.first->second == b.first->second);
			break;
		}
		case 3: {
			auto a = map.insert_or_assign(key, value);
			auto b = expected.insert_or_assign(key, value);
			assert(a.second == b.second && a.first->first == key && a.first->second == value);
			break;
		}
		case 4: {
			auto a = map.emplace(key, value);
			auto b = expected.emplace(key, value);
			assert(a.second == b.second && a.first->first == key && a.first->second == b.first->second);
			break;
		}
		case 5: {
			auto it = map.find(key);
			if (it == map.end()) break;
			auto next = it;
			++next;
			auto other = map.begin();
			if (other == it) ++other;
			auto *saved = other == map.end() ? nullptr : std::addressof(other->second);
			auto result = (step & 1) ? map.erase(it) : map.erase(ConstIterator(it));
			assert(result == next);
			if (saved) assert(std::addressof(other->second) == saved && other->second == expected.at(other->first));
			assert(expected.erase(key) == 1);
			break;
		}
		case 6: assert(map.erase(key) == bool(expected.erase(key))); break;
		case 7:
			if (step % 173 == 0) map.clear(), expected.clear();
			break;
		case 8:
			if (step % 127 == 0) map.reserve(rng() % 1000);
			break;
		}
		if (step % 131 == 0) check_api_map(map, expected);
	}
	check_api_map(map, expected);
	for (auto it = map.cbegin(); it != map.cend();) {
		auto key = it->first;
		auto next = it;
		++next;
		it = map.erase(it);
		assert(it == next && expected.erase(key) == 1);
	}
	assert(map.empty() && map.begin() == map.end() && expected.empty());
	assert(map.try_emplace(K{}, V{}).second);
	assert(map.erase(map.begin()) == map.end());
}

struct ApiConstructed {
	static inline int constructions = 0;
	int value = 0;
	ApiConstructed() noexcept = default;
	explicit ApiConstructed(int x) : value(x) {
		++constructions;
		if (x < 0) throw std::runtime_error("value");
	}
};

struct ApiNonDefault {
	int value;
	std::string text;
	ApiNonDefault() = delete;
	ApiNonDefault(int value, std::string text) : value(value), text(std::move(text)) {}
};

struct ApiAssignable {
	static inline int constructions = 0, assignments = 0;
	int value = 0;
	ApiAssignable() noexcept = default;
	ApiAssignable(int x) : value(x) { ++constructions; }
	ApiAssignable &operator=(int x) { ++assignments; value = x * 2; return *this; }
};

struct ApiKey {
	std::unique_ptr<int> value;
	explicit ApiKey(int x) : value(std::make_unique<int>(x)) {}
	ApiKey(ApiKey &&) = default;
	ApiKey &operator=(ApiKey &&) = default;
	friend bool operator==(const ApiKey &a, const ApiKey &b) { return *a.value == *b.value; }
};

struct ApiKeyHash {
	std::size_t operator()(const ApiKey &key) const { return std::hash<int>{}(*key.value); }
};

void construction_and_exceptions() {
	static_assert(flat_hash_map_detail::packed<int, ApiConstructed, std::hash<int>, std::equal_to<int>>);
	FlatHashMap<int, ApiConstructed> map;
	ApiConstructed::constructions = 0;
	auto a = map.try_emplace(1, 7);
	assert(a.second && a.first->second.value == 7 && ApiConstructed::constructions == 1);
	auto b = map.try_emplace(1, -1);
	assert(!b.second && b.first == a.first && ApiConstructed::constructions == 1);
	for (int i = 0; i < 3; ++i) {
		bool caught = false;
		try { map.try_emplace(2, -1); } catch (const std::runtime_error &) { caught = true; }
		assert(caught && map.size() == 1 && map.find(2) == map.end() && map.at(1).value == 7);
	}
	assert(map.try_emplace(2, 9).second && map.at(2).value == 9);
	FlatHashMap<std::string, ApiNonDefault> words;
	auto [first, fresh] = words.try_emplace("first", 3, "abc");
	assert(fresh && first->second.value == 3 && first->second.text == "abc");
	auto [second, added] = words.emplace(std::piecewise_construct, std::forward_as_tuple("second"), std::forward_as_tuple(4, "def"));
	assert(added && second->second.value == 4 && second->second.text == "def");
	auto [replaced, inserted] = words.insert_or_assign("first", ApiNonDefault(5, "ghi"));
	assert(!inserted && replaced->second.value == 5 && replaced->second.text == "ghi");
	FlatHashMap<std::string, ApiConstructed> fallback;
	fallback.try_emplace("one", 1);
	auto before = ApiConstructed::constructions;
	assert(!fallback.try_emplace("one", -1).second && ApiConstructed::constructions == before);
	bool caught = false;
	try { fallback.try_emplace("two", -1); } catch (const std::runtime_error &) { caught = true; }
	assert(caught && fallback.size() == 1 && !fallback.contains("two"));
	static_assert(flat_hash_map_detail::packed<int, ApiAssignable, std::hash<int>, std::equal_to<int>>);
	FlatHashMap<int, ApiAssignable> assigned;
	std::unordered_map<int, ApiAssignable> expected;
	assigned.try_emplace(1, 3);
	expected.try_emplace(1, 3);
	ApiAssignable::constructions = ApiAssignable::assignments = 0;
	auto actual = assigned.insert_or_assign(1, 4);
	assert(!actual.second && ApiAssignable::constructions == 0 && ApiAssignable::assignments == 1);
	auto wanted = expected.insert_or_assign(1, 4);
	assert(actual.first->second.value == wanted.first->second.value);
	auto absent = assigned.insert_or_assign(2, 5);
	auto new_value = expected.insert_or_assign(2, 5);
	assert(absent.second && absent.first->second.value == new_value.first->second.value);
}

void move_only_values() {
	using Map = FlatHashMap<int, std::unique_ptr<int>>;
	Map map;
	auto a = map.try_emplace(1, std::make_unique<int>(10));
	assert(a.second && *a.first->second == 10);
	auto pointer = std::make_unique<int>(20);
	auto b = map.try_emplace(1, std::move(pointer));
	assert(!b.second && pointer && *pointer == 20 && *b.first->second == 10);
	auto c = map.insert_or_assign(1, std::move(pointer));
	assert(!c.second && !pointer && *c.first->second == 20);
	Map::value_type entry{2, std::make_unique<int>(30)};
	auto d = map.insert(std::move(entry));
	assert(d.second && !entry.second && *d.first->second == 30);
	auto e = map.emplace(3, std::make_unique<int>(40));
	assert(e.second && *e.first->second == 40);
	const auto &view = map;
	auto next = view.find(2);
	++next;
	assert(map.erase(view.find(2)) == next && !map.contains(2));
	FlatHashMap<ApiKey, std::unique_ptr<int>, ApiKeyHash> keys;
	auto [one, inserted] = keys.try_emplace(ApiKey(1), std::make_unique<int>(3));
	assert(inserted && *one->first.value == 1 && *one->second == 3);
	ApiKey duplicate(1);
	assert(!keys.try_emplace(std::move(duplicate), std::make_unique<int>(4)).second && duplicate.value);
	auto assigned = keys.insert_or_assign(ApiKey(1), std::make_unique<int>(5));
	assert(!assigned.second && *assigned.first->second == 5);
	auto fresh = keys.emplace(std::piecewise_construct, std::forward_as_tuple(2), std::forward_as_tuple(std::make_unique<int>(6)));
	assert(fresh.second && *fresh.first->first.value == 2 && *fresh.first->second == 6);
}

void aliased_values() {
	for (int cleanup = 0; cleanup < 2; ++cleanup) for (int operation = 0; operation < 4; ++operation) {
		FlatHashMap<int, std::uint64_t> map;
		for (int key = 0; key < 8; ++key) map.set(key, key + 10);
		if (cleanup) for (int key = 0; key < 8; ++key) if (key != 3 && key != 7) map.erase(key);
		auto &source = map.at(3);
		if (operation == 0) assert(map.try_emplace(100, source).second);
		if (operation == 1) assert(map.insert_or_assign(100, source).second);
		if (operation == 2) assert(map.emplace(100, source).second);
		if (operation == 3) assert(map.insert({100, source}).second);
		assert(map.at(100) == 13 && map.at(3) == 13 && map.size() == (cleanup ? 3u : 9u));
		assert(!map.insert_or_assign(3, map.at(3)).second && map.at(3) == 13);
	}
	FlatHashMap<std::string, std::string> words;
	words.try_emplace("source", "destination");
	auto a = words.try_emplace(words.at("source"), "value");
	assert(a.second && a.first->first == "destination" && a.first->second == "value");
	assert(!words.insert_or_assign(words.at("source"), words.at("source")).second);
	assert(words.at("destination") == "destination");
	auto position = words.find(words.at("source")), following = position;
	++following;
	assert(words.erase(position) == following);
	assert(words.size() == 1 && words.at("source") == "destination");
}

int main() {
	random_api<std::uint64_t, std::uint64_t>();
	random_api<std::uint32_t, std::uint64_t>();
	random_api<std::int16_t, ApiPayload>();
	random_api<int, int, ApiConstantHash>();
	construction_and_exceptions();
	move_only_values();
	aliased_values();
}
