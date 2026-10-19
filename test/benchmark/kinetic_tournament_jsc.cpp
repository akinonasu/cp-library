#include "data_structure/segment_tree/kinetic_tournament.hpp"
#include "components/fast_io.hpp"

std::int64_t solve(const std::vector<std::int64_t> &a) {
	if (a.empty()) return 0;
	auto slopes = a;
	std::sort(slopes.begin(), slopes.end());
	slopes.erase(std::unique(slopes.begin(), slopes.end()), slopes.end());
	slopes.push_back(slopes.back() + 1);
	int n = static_cast<int>(slopes.size());
	KineticTournament<> tree(slopes, std::nullopt);
	tree.set(n - 1, 0);
	std::int64_t prefix = slopes.back();
	for (auto x : a) {
		if (x < prefix) {
			tree.add(0, n, x);
			int i = std::lower_bound(slopes.begin(), slopes.end(), prefix) - slopes.begin();
			tree.set(i, tree.all_prod()->value);
			prefix = x;
		} else {
			int k = std::upper_bound(slopes.begin(), slopes.end(), x) - slopes.begin();
			auto next = tree.prod(k, n);
			tree.advance(0, k);
			tree.add(k, n, prefix);
			if (next) {
				int i = k - 1;
				auto previous = tree.get(i);
				tree.set(i, previous ? std::max(*previous, next->value + x) : next->value + x);
			}
		}
	}
	return tree.all_prod()->value;
}

#ifndef KINETIC_TOURNAMENT_JSC_TEST
int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<std::int64_t> a(n);
	for (auto &x : a) in.read(x);
	out.println(solve(a));
}
#endif
