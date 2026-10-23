#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#include "data_structure/flat_hash_map.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	FlatHashMap<std::uint64_t, std::uint64_t> values;
	values.reserve(q);
	while (q--) {
		int type = 0;
		std::uint64_t key = 0;
		input.read(type), input.read(key);
		if (type == 0) {
			std::uint64_t value = 0;
			input.read(value);
			values.set(key, value);
		} else {
			const auto *value = values.find(key);
			output.println(value ? *value : 0);
		}
	}
}
