#define PROBLEM "https://judge.yosupo.jp/problem/deque"
#include "components/fast_io.hpp"
#include <deque>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	std::deque<int> values;
	while (q--) {
		int type = 0, x = 0;
		input.read(type);
		if (type == 0 || type == 1 || type == 4) input.read(x);
		if (type == 0) values.push_front(x);
		else if (type == 1) values.push_back(x);
		else if (type == 2) values.pop_front();
		else if (type == 3) values.pop_back();
		else output.println(values[x]);
	}
}
