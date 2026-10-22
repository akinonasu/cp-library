#include "sequence/run_length_encoding.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <forward_list>
#include <list>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

template<class Range>
void check(const Range &a) {
	using T = std::ranges::range_value_t<const Range>;
	auto runs = run_length_encode(a);
	std::vector<T> decoded;
	for (std::size_t i = 0; i < runs.size(); ++i) {
		const auto &[value, count] = runs[i];
		assert(count > 0);
		if (i) assert(runs[i - 1].first != value);
		decoded.insert(decoded.end(), count, value);
	}
	assert(std::ranges::equal(a, decoded));
}

struct Key {
	int id, payload;
	Key(int id, int payload) : id(id), payload(payload) {}
};

int main() {
	static_assert(std::is_same_v<decltype(run_length_encode(std::string{})),
		std::vector<std::pair<char, std::int64_t>>>);
	for (int n = 0; n <= 8; ++n) {
		std::vector<int> a(n);
		while (true) {
			const auto saved = a;
			check(a);
			assert(a == saved);
			int i = 0;
			while (i < n && ++a[i] == 3) a[i++] = 0;
			if (i == n) break;
		}
	}
	check(std::array<int, 0>{});
	check(std::deque<int>{1, 1, 2, 3, 3, 1});
	check(std::list<int>{2, 2, 2, 1, 1});
	check(std::forward_list<int>{4, 4, 3});
	check(std::vector<bool>{true, true, false, false, false, true});
	check(std::vector<std::string>{"a", "a", "bb", "bb", "a"});
	check(std::string("a\0\0b", 4));
	check(std::string_view("aaabbcaaa"));
	check(std::views::iota(0, 100));
	const int a[]{2, 2, 5, 5, 5, 2};
	check(a);
	check(std::span<const int>(a));
	const char raw[]{'a', 'a', '\0', '\0', 'b'};
	check(raw);
	auto sentinel = std::views::counted(a, 6) | std::views::take_while([](int x) { return x != 5; });
	check(sentinel);
	std::vector<Key> keys;
	for (int i = 0; i < 5; ++i) keys.emplace_back(i / 2, i);
	auto grouped = run_length_encode(keys, [](const Key &a, const Key &b) { return a.id == b.id; });
	assert(grouped.size() == 3);
	for (int i = 0; i < 3; ++i) {
		assert(grouped[i].first.id == i && grouped[i].first.payload == 2 * i);
		assert(grouped[i].second == std::int64_t(i == 2 ? 1 : 2));
	}
	auto long_run = run_length_encode(std::vector<int>(1000000, 42));
	assert(long_run.size() == 1 && long_run[0].first == 42 && long_run[0].second == 1000000);
}
