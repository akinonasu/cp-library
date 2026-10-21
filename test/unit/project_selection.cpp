#include "graph/flow/project_selection.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <utility>
#include <vector>

using Wide = __int128_t;

struct Model {
	struct Pair { int u, v; std::array<Wide, 4> cost; };
	struct Group { std::vector<int> vertices; bool value; Wide profit; };
	int n;
	Wide constant = 0;
	std::vector<std::array<Wide, 2>> unary;
	std::vector<Pair> pairs;
	std::vector<Group> groups;
	std::vector<std::pair<int, bool>> forced;
	std::vector<std::pair<int, int>> implications;
	explicit Model(int n) : n(n), unary(n) {}
	std::optional<Wide> evaluate(unsigned mask) const {
		for (auto [v, value] : forced) if (bool(mask >> v & 1) != value) return std::nullopt;
		for (auto [u, v] : implications) if ((mask >> u & 1) && !(mask >> v & 1)) return std::nullopt;
		Wide cost = constant;
		for (int v = 0; v < n; ++v) cost += unary[v][mask >> v & 1];
		for (const auto &pair : pairs) cost += pair.cost[2 * (mask >> pair.u & 1) + (mask >> pair.v & 1)];
		for (const auto &group : groups) {
			bool all = true;
			for (int v : group.vertices) all &= bool(mask >> v & 1) == group.value;
			if (all) cost -= group.profit;
		}
		return cost;
	}
};

template<class Cost>
void check(const ProjectSelection<Cost> &problem, const Model &model) {
	assert(problem.size() == model.n);
	std::optional<Wide> expected;
	for (unsigned mask = 0; mask < (1U << model.n); ++mask) {
		auto cost = model.evaluate(mask);
		if (cost && (!expected || *cost < *expected)) expected = cost;
	}
	auto result = problem.solve();
	assert(bool(result) == bool(expected));
	if (!expected) return;
	assert(result->cost == *expected && result->profit() == -*expected);
	assert(result->selected.size() == std::size_t(model.n));
	unsigned mask = 0;
	for (int v = 0; v < model.n; ++v) mask |= unsigned(result->selected[v]) << v;
	assert(model.evaluate(mask) == expected);
}

template<class Cost>
void random_cases() {
	std::mt19937 random(20261021);
	auto number = [&] { return Cost(int(random() % 41) - 20); };
	for (int trial = 0; trial < 1200; ++trial) {
		int n = random() % 9;
		ProjectSelection<Cost> problem(n);
		Model model(n);
		for (int step = 0; step < 24; ++step) {
			int op = n ? random() % 12 : random() % 2 ? 6 : 10;
			int u = n ? random() % n : 0, v = n ? random() % n : 0;
			Cost a = number(), b = number(), c = Cost(random() % 21);
			switch (op) {
			case 0:
				problem.add_cost(u, a, b);
				model.unary[u][0] += a, model.unary[u][1] += b;
				break;
			case 1:
				problem.add_profit(u, a);
				model.unary[u][1] -= a;
				break;
			case 2:
				problem.add_profit(u, a, b);
				model.unary[u][0] -= a, model.unary[u][1] -= b;
				break;
			case 3:
				problem.add_penalty(u, v, c);
				model.pairs.push_back({u, v, {0, 0, c, 0}});
				break;
			case 4:
				problem.add_difference_cost(u, v, c);
				model.pairs.push_back({u, v, {0, c, c, 0}});
				break;
			case 5: {
				Cost d = number();
				Cost c00 = a, c01 = a + b, c10 = a + d + c, c11 = a + b + d;
				problem.add_pair_cost(u, v, c00, c01, c10, c11);
				model.pairs.push_back({u, v, {c00, c01, c10, c11}});
				break;
			}
			case 6: {
				std::vector<int> vertices;
				if (n) for (int i = random() % 6; i; --i) vertices.push_back(random() % n);
				bool value = random() & 1;
				problem.add_all_profit(vertices, value, c);
				model.groups.push_back({vertices, value, c});
				break;
			}
			case 7:
				problem.force(u, bool(c & 1));
				model.forced.emplace_back(u, bool(c & 1));
				break;
			case 8:
				problem.imply(u, v);
				model.implications.emplace_back(u, v);
				break;
			case 9:
				problem.same(u, v);
				model.implications.emplace_back(u, v);
				model.implications.emplace_back(v, u);
				break;
			case 10:
				problem.add_constant(a);
				model.constant += a;
				break;
			default:
				problem.add_cost(u, a);
				model.unary[u][1] += a;
			}
			if (step % 8 == 7) check(problem, model);
		}
		check(problem, model);
		auto copy = problem;
		copy.add_constant(Cost(7));
		model.constant += 7;
		check(copy, model);
	}
}

int main() {
	random_cases<int>();
	random_cases<std::int64_t>();
	for (int c00 = -2; c00 <= 2; ++c00) for (int c01 = -2; c01 <= 2; ++c01)
		for (int c10 = -2; c10 <= 2; ++c10) for (int c11 = -2; c11 <= 2; ++c11) {
			if (c00 + c11 > c01 + c10) continue;
			for (int state = 0; state < 4; ++state) {
				ProjectSelection<> problem(2);
				problem.add_pair_cost(0, 1, c00, c01, c10, c11);
				problem.force(0, state >> 1), problem.force(1, state & 1);
				assert(problem.solve()->cost == (std::array<int, 4>{c00, c01, c10, c11}[state]));
			}
		}
	ProjectSelection<> empty;
	assert(empty.solve()->cost == 0 && empty.solve()->selected.empty());
	ProjectSelection<> example(2);
	example.add_profit(0, 10);
	example.add_cost(1, 4);
	example.imply(0, 1);
	assert(example.solve()->profit() == 6);
	assert((example.solve()->selected == std::vector<bool>{true, true}));
	example.force(1, false);
	assert(example.solve()->cost == 0);
	example.force(0, true);
	assert(!example.solve());
	ProjectSelection<> extreme(2);
	const auto low = std::numeric_limits<std::int64_t>::min();
	const auto high = std::numeric_limits<std::int64_t>::max();
	extreme.add_cost(0, low);
	extreme.add_profit(1, low);
	extreme.add_profit(1, high);
	auto result = extreme.solve();
	assert(result && result->cost == Wide(low) && result->profit() == -Wide(low));
	extreme.force(0, false);
	assert(extreme.solve()->cost == 0);
	ProjectSelection<> pair(2);
	pair.add_pair_cost(0, 1, high - 1, -1, high - 1, -1);
	assert(pair.solve()->cost == -1);
	ProjectSelection<> wide_constant;
	wide_constant.add_constant(high);
	wide_constant.add_constant(high);
	assert(wide_constant.solve()->cost == Wide(high) * 2);
	ProjectSelection<> wide_capacity(1);
	wide_capacity.add_cost(0, low, high - 1);
	wide_capacity.force(0, true);
	assert(wide_capacity.solve()->cost == Wide(high) - 1);
	ProjectSelection<> contradictions(10);
	contradictions.add_profit(0, high);
	for (int v = 0; v < 10; ++v) {
		contradictions.force(v, true);
		contradictions.force(v, false);
	}
	assert(!contradictions.solve());
	ProjectSelection<> diagonal(1);
	diagonal.add_pair_cost(0, 0, 10, low, low, -7);
	assert(diagonal.solve()->cost == -7);
	ProjectSelection<> chain(200000);
	chain.add_profit(0, 17);
	chain.add_cost(199999, 3);
	for (int v = 1; v < chain.size(); ++v) chain.imply(v - 1, v);
	assert(chain.solve()->profit() == 14);
	chain.force(199999, false);
	assert(chain.solve()->cost == 0);
	chain.force(0, true);
	assert(!chain.solve());
}
