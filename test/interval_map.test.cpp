#define PROBLEM "https://judge.yosupo.jp/problem/associative_array"
#include "data_structure/interval_map.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	IntervalMap<std::uint64_t, std::uint64_t> values;
	while (q--) {
		int type = 0;
		std::uint64_t key = 0;
		input.read(type), input.read(key);
		if (type == 0) {
			std::uint64_t value = 0;
			input.read(value);
			values.set(key, value);
		} else output.println(values.get(key).value_or(0));
	}
}
