#include "../../components/static_range_min.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

void require(bool condition) { if (!condition) std::abort(); }

template <class T, bool Cached>
void check_once(const std::vector<T> &values) {
	StaticRangeMin<T, Cached> range(values);
	require(range.size() == int(values.size()) && range.empty() == values.empty());
	for (int l = 0; l < range.size(); ++l) {
		require(!(range[l] < values[l]) && !(values[l] < range[l]));
		T expected = values[l];
		for (int r = l + 1; r <= range.size(); ++r) {
			if (values[r - 1] < expected) expected = values[r - 1];
			T actual = range.prod(l, r);
			require(!(actual < expected) && !(expected < actual));
		}
	}
	if (!values.empty()) {
		T expected = *std::min_element(values.begin(), values.end());
		T actual = range.all_prod();
		require(!(actual < expected) && !(expected < actual));
	}
	std::vector<std::pair<int, int>> queries;
	for (int l = 0; l < range.size(); ++l) {
		queries.emplace_back(l, l + 1);
		queries.emplace_back(l, range.size());
		queries.emplace_back(l, std::min(l + 33, range.size()));
	}
	auto answers = range.prod_many(queries);
	require(answers.size() == queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [l, r] = queries[i];
		T expected = *std::min_element(values.begin() + l, values.begin() + r);
		T actual = answers[i];
		require(!(actual < expected) && !(expected < actual));
	}
	require(range.prod_many({}).empty());
	auto copy = range;
	auto moved = std::move(copy);
	require(moved.size() == range.size());
	copy.build(values);
	require(copy.size() == range.size());
	range.build({});
	require(range.empty() && range.size() == 0);
	range = moved;
	require(range.size() == int(values.size()));
}

template <class T>
void check(const std::vector<T> &values) {
	check_once<T, false>(values);
	check_once<T, true>(values);
}

struct Value {
	int value;
	explicit Value(int value) : value(value) {}
	friend bool operator<(const Value &a, const Value &b) { return a.value < b.value; }
};

struct Payload {
	int key, id;
	friend bool operator<(const Payload &a, const Payload &b) { return a.key < b.key; }
	friend bool operator==(const Payload &, const Payload &) = default;
};

template <class T>
void check_wide_batches(const std::vector<T> &values) {
	StaticRangeMin<T> range(values);
	std::mt19937 rng(7162);
	for (int count : {1, 15, 31, 32, 33, 64, 65, 257}) {
		for (bool mixed : {false, true}) {
			std::vector<std::pair<int, int>> queries;
			for (int i = 0; i < count; ++i) {
				int l = rng() % 400, r = 700 + rng() % 350;
				if (mixed && i % 3 == 0) r = l + 1;
				queries.emplace_back(l, r);
			}
			auto answers = range.prod_many(queries);
			for (int i = 0; i < count; ++i) {
				auto [l, r] = queries[i];
				T expected = *std::min_element(values.begin() + l, values.begin() + r);
				T actual = answers[i];
				require(!(actual < expected) && !(expected < actual));
			}
		}
	}
}

int main() {
	static_assert(std::is_same_v<decltype(StaticRangeMin(std::vector<int>{})), StaticRangeMin<int>>);
	std::mt19937 rng(37192);
	for (int n : {0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129, 1023}) {
		for (int kind = 0; kind < 5; ++kind) {
			std::vector<int> a(n);
			for (int i = 0; i < n; ++i) a[i] = kind == 0 ? i : kind == 1 ? -i : kind == 2 ? 7 : kind == 3 ? int(rng() % 5) : int(rng());
			check(a);
		}
	}
	check<std::uint64_t>({0, 1, std::numeric_limits<std::uint64_t>::max(), 0});
	check<std::string>({"z", "apple", "apple", "banana", ""});
	std::vector<bool> bits(130);
	for (int i = 0; i < int(bits.size()); ++i) bits[i] = rng() & 1;
	check(bits);
	std::vector<Value> values;
	for (int i = 0; i < 100; ++i) values.emplace_back(int(rng() % 7));
	check(values);
	for (int kind = 0; kind < 6; ++kind) {
		std::vector<int> data(1057);
		for (int i = 0; i < int(data.size()); ++i)
			data[i] = kind == 0 ? i : kind == 1 ? -i : kind == 2 ? 7 : kind == 3 ? int(rng() % 5) : kind == 4 ? int(rng()) : (i % 32 == 16 ? -1000 : i);
		check_wide_batches(data);
		std::vector<Value> objects;
		for (int value : data) objects.emplace_back(value);
		check_wide_batches(objects);
	}
	check_wide_batches(std::vector<bool>(1057, true));

	for (int kind = 0; kind < 2; ++kind) {
		std::vector<Payload> payloads;
		for (int i = 0; i < 1057; ++i) payloads.push_back({kind ? i % 5 : 0, i});
		StaticRangeMin range(payloads);
		std::vector<std::pair<int, int>> queries;
		for (int i = 0; i < 257; ++i) queries.emplace_back(i, 700 + i);
		auto answers = range.prod_many(queries);
		for (int i = 0; i < int(queries.size()); ++i)
			require(answers[i] == range.prod(queries[i].first, queries[i].second));
	}

	StaticRangeMin<int> empty;
	require(empty.empty());
}
