#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include "data_structure/double_ended_priority_queue.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> values(n);
	for (int& value : values) input.read(value);
	DoubleEndedPriorityQueue<int> queue(std::move(values));
	queue.reserve(std::size_t(n) + q);
	while (q--) {
		int type = 0;
		input.read(type);
		if (type == 0) {
			int value = 0;
			input.read(value);
			queue.push(value);
		} else output.println(type == 1 ? queue.pop_min() : queue.pop_max());
	}
}
