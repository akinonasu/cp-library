#define PROBLEM "https://atcoder.jp/contests/abc326/tasks/abc326_g"
#include "graph/flow/project_selection.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	ProjectSelection<> problem(4 * n);
	for (int v = 0; v < n; ++v) {
		std::int64_t cost = 0;
		input.read(cost);
		for (int level = 0; level < 4; ++level) {
			problem.add_cost(4 * v + level, cost);
			if (level) problem.imply(4 * v + level, 4 * v + level - 1);
		}
	}
	std::vector<std::int64_t> rewards(m);
	for (auto &reward : rewards) input.read(reward);
	for (auto reward : rewards) {
		std::vector<int> required;
		for (int v = 0; v < n; ++v) {
			int level = 0;
			input.read(level);
			if (level > 1) required.push_back(4 * v + level - 2);
		}
		problem.add_all_profit(required, true, reward);
	}
	output.println(problem.solve()->profit());
}
