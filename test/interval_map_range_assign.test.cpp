#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_D"
#include "data_structure/interval_map.hpp"
#include "components/fast_io.hpp"
#include <limits>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	IntervalMap<int, int> values(0, n, std::numeric_limits<int>::max());
	while (q--) {
		int type = 0, l = 0;
		input.read(type), input.read(l);
		if (type == 0) {
			int r = 0, x = 0;
			input.read(r), input.read(x);
			if (x) values.assign(l, r + 1, x);
			else values.erase(l, r + 1);
		} else output.println(values.get(l).value_or(0));
	}
}
