#include "components/radix_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

void require(bool value) { if (!value) std::abort(); }

struct Item {
	std::uint64_t key = 0;
	int id = 0;
	bool operator==(const Item &) const = default;
};

template <class T>
void adaptive_integer_sort() {
	std::mt19937_64 rng(sizeof(T));
	for (int n : {0, 1, 255, 256, 1024}) {
		std::vector<T> values(n);
		for (auto &value : values) value = T(rng());
		if (n > 1) values[0] = std::numeric_limits<T>::min(), values[1] = std::numeric_limits<T>::max();
		auto expected = values;
		std::sort(expected.begin(), expected.end());
		radix_sort_detail::sort_by_key(values, [](const T &value) -> const T & { return value; });
		require(values == expected);
	}
}

int main() {
	adaptive_integer_sort<std::int8_t>(); adaptive_integer_sort<std::uint8_t>();
	adaptive_integer_sort<std::int64_t>(); adaptive_integer_sort<std::uint64_t>();
	std::vector<std::string> words{"c", "a", "b", "a"};
	radix_sort_detail::sort_by_key(words, [](const auto &word) -> const auto & { return word; });
	require((words == std::vector<std::string>{"a", "a", "b", "c"}));
	struct Nondefault { int value; explicit Nondefault(int value) : value(value) {} };
	std::vector<Nondefault> nondefault{Nondefault(3), Nondefault(1), Nondefault(2)};
	radix_sort_detail::sort_by_key(nondefault, [](const auto &item) { return item.value; });
	for (int i = 0; i < 3; ++i) require(nondefault[i].value == i + 1);
	std::mt19937_64 rng(20261020);
	for (int n : {0, 1, 2, 3, 31, 255, 256, 257, 1024, 10000}) {
		for (int mode = 0; mode < 8; ++mode) {
			std::vector<Item> values(n);
			for (int i = 0; i < n; ++i) {
				std::uint64_t key = rng();
				if (mode == 1) key %= 7;
				if (mode == 2) key = (key % 13) << 56;
				if (mode == 3) key = (key % 19) << 24;
				if (mode == 4) key = 0;
				if (mode == 5) key = std::numeric_limits<std::uint64_t>::max();
				if (mode == 6) key = i / 3;
				if (mode == 7) key = (n - i) / 3;
				values[i] = {key, i};
			}
			auto expected = values;
			std::stable_sort(expected.begin(), expected.end(), [](const Item &a, const Item &b) { return a.key < b.key; });
			for (int bits : {1, 8, 11, 16}) {
				auto copy = values;
				if (bits == 1) stable_radix_sort<1>(copy, [](const Item &item) { return item.key; });
				if (bits == 8) stable_radix_sort<Item>(copy, [](const Item &item) { return item.key; });
				if (bits == 11) stable_radix_sort<11>(copy, [](const Item &item) { return item.key; });
				if (bits == 16) stable_radix_sort<16>(copy, [](const Item &item) { return item.key; });
				require(copy == expected);
			}
			stable_radix_sort(values, [](const Item &item) { return item.key; });
			require(values == expected);
			stable_radix_sort(values, [](const Item &item) { return item.key; });
			require(values == expected);
		}
	}
	struct CopyOnlyKey {
		CopyOnlyKey() = default;
		CopyOnlyKey(const CopyOnlyKey &) = default;
		CopyOnlyKey(CopyOnlyKey &&) = delete;
		std::uint64_t operator()(const Item &item) const { return item.key; }
	} key;
	std::vector<Item> explicit_type(300);
	for (int i = 0; i < 300; ++i) explicit_type[i] = {std::uint64_t(300 - i), i};
	stable_radix_sort<Item, CopyOnlyKey>(explicit_type, key);
	for (int i = 0; i < 300; ++i) require(explicit_type[i].key == std::uint64_t(i + 1));
	std::vector<std::uint32_t> small(1000);
	for (auto &x : small) x = rng();
	auto expected = small;
	std::sort(expected.begin(), expected.end());
	stable_radix_sort(small, [](std::uint32_t x) { return x; });
	require(small == expected);
	std::vector<bool> bits(1000);
	for (std::size_t i = 0; i < bits.size(); ++i) bits[i] = rng() & 1;
	auto sorted_bits = bits;
	std::sort(sorted_bits.begin(), sorted_bits.end());
	stable_radix_sort(bits, [](bool value) { return static_cast<unsigned>(value); });
	require(bits == sorted_bits);
	std::vector<std::unique_ptr<Item>> movable;
	std::vector<Item> stable;
	for (int i = 0; i < 1000; ++i) {
		auto item = std::make_unique<Item>(Item{rng() % 17, i});
		stable.push_back(*item), movable.push_back(std::move(item));
	}
	std::stable_sort(stable.begin(), stable.end(), [](const Item &a, const Item &b) { return a.key < b.key; });
	stable_radix_sort(movable, [](const auto &item) { return item->key; });
	for (std::size_t i = 0; i < stable.size(); ++i) require(movable[i] && *movable[i] == stable[i]);
}
