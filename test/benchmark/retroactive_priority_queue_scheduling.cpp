#include "../../components/fast_io.hpp"
#include "../fixtures/retroactive_scheduling.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<int> deadline(n);
	std::vector<long long> reward(n);
	for (auto &x : deadline) in.read(x);
	for (auto &x : reward) in.read(x);
	RetroactiveScheduling scheduling(std::move(deadline), std::move(reward));
	while (q--) {
		int c = 0, x = 0;
		long long y = 0;
		in.read(c), in.read(x), in.read(y);
		out.println(scheduling.set(c - 1, x, y));
	}
}
