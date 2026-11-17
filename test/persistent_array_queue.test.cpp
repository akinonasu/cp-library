#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include "data_structure/persistent_array.hpp"
#include "components/fast_io.hpp"
#include <bit>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	PersistentArray<int> array(std::vector<int>(q, 0));
	array.reserve(std::size_t(q) * (std::bit_width(unsigned(q)) / 3 + 3));
	struct State { decltype(array)::root_type root; int first, last; };
	std::vector<State> versions(q + 1, {array.initial_root(), 0, 0});
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		auto state = versions[k + 1];
		if (type == 0) {
			int value = 0;
			input.read(value);
			state.root = array.set(state.root, state.last++, value);
		} else output.println(array.get(state.root, state.first++));
		versions[i] = state;
	}
}
