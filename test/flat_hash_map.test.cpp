#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#include "data_structure/flat_hash_map.hpp"
#include "components/fast_io.hpp"
#include <array>
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	FlatHashMap<std::uint64_t, std::uint64_t> values;
	values.reserve(q);
	struct Query { std::uint64_t key, value; int type; };
	std::array<Query, 128> queries{};
	auto apply = [&](const Query &query) {
		if (query.type == 0) values.set(query.key, query.value);
		else {
			const auto *value = values.find(query.key);
			output.println(value ? *value : 0);
		}
	};
	auto read = [&](Query &query) {
		input.read(query.type), input.read(query.key);
		if (query.type == 0) input.read(query.value);
	};
	while (q) {
		while (q && values.size() < 4096) {
			Query query{};
			read(query);
			apply(query);
			--q;
		}
		if (!q) break;
		int count = std::min<int>(q, queries.size());
		for (int i = 0; i < count; ++i) read(queries[i]);
		for (int i = 0; i < std::min(count, 16); ++i) values.prefetch(queries[i].key);
		for (int i = 0; i < count; ++i) {
			if (i + 16 < count) values.prefetch(queries[i + 16].key);
			apply(queries[i]);
		}
		q -= count;
	}
}
