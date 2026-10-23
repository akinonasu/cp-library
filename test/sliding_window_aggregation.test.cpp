#define PROBLEM "https://judge.yosupo.jp/problem/queue_operate_all_composite"
#include "data_structure/sliding_window_aggregation.hpp"
#include "components/fast_io.hpp"
#include "math/mint.hpp"
#include <utility>

using S = std::pair<mint, mint>;
S op(S f, S g) { return {g.first * f.first, g.first * f.second + g.second}; }
S e() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	SlidingWindowAggregation<S, op, e> queue;
	queue.reserve(q);
	while (q--) {
		int type = 0;
		input.read(type);
		if (type == 0) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			queue.push_back({mint::raw(a), mint::raw(b)});
		} else if (type == 1) queue.pop_front();
		else {
			unsigned x = 0;
			input.read(x);
			auto [a, b] = queue.prod();
			output.println((a * mint::raw(x) + b).val());
		}
	}
}
