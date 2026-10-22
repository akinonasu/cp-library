#include "graph/difference_constraints.hpp"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

using System = DifferenceConstraints<>;

template<class Integer>
bool validate(const DifferenceConstraints<Integer> &system, const typename DifferenceConstraints<Integer>::Result &result, bool feasible) {
	if (result.feasible() != feasible) return false;
	if (!feasible) {
		__int128_t cost = 0;
		for (auto edge : result.negative_cycle()) cost += edge.weight;
		return cost < 0;
	}
	for (auto edge : system.constraints()) if (__int128_t(result.value(edge.to)) - __int128_t(result.value(edge.from)) > edge.weight) return false;
	return true;
}

int main() {
	std::mt19937_64 random(20261022);
	std::cout << std::fixed << std::setprecision(6);
	for (const std::string name : {"sparse", "reverse_dag", "components", "infeasible"}) {
		int n = name == "reverse_dag" ? 200000 : 20000;
		int source = name == "reverse_dag" ? n - 1 : 0;
		System system(n);
		std::vector<std::int64_t> potential(n);
		for (auto &value : potential) value = std::int64_t(random() % 20001) - 10000;
		if (name == "reverse_dag") {
			for (int v = 1; v < n; ++v) system.add_le(v, v - 1, -1);
			system.fix(source, 0);
		} else {
			for (int v = 0; v < n; ++v) {
				int to = name == "components" ? v / 4 * 4 + (v + 1) % 4 : (v + 1) % n;
				system.add_le(v, to, 1 + potential[to] - potential[v]);
			}
			if (name != "components") for (int i = 0; i < 80000; ++i) {
				int u = random() % n, v = random() % n;
				system.add_le(u, v, std::int64_t(random() % 100) + potential[v] - potential[u]);
			}
			system.fix(0, potential[0]);
		}
		bool feasible = name != "infeasible";
		if (!feasible) system.add_lt(n - 1, n - 1, 0);
		DifferenceConstraints<__int128_t> wide_system(n);
		for (auto edge : system.constraints()) wide_system.add_le(edge.from, edge.to, edge.weight);
		for (int run = -1; run < 7; ++run) {
			for (int order = 0; order < 2; ++order) {
				bool wide = (order + run + 1) % 2;
				auto measure = [&](const auto &chosen) {
					auto start = std::chrono::steady_clock::now();
					auto result = chosen.solve();
					auto stop = std::chrono::steady_clock::now();
					if (!validate(chosen, result, feasible)) std::exit(1);
					if (run >= 0) std::cout << name << '\t' << n << '\t' << system.constraint_count() << '\t' << (wide ? "128" : "64") << "\tsolve\t" << run << '\t'
						<< std::chrono::duration<double, std::milli>(stop - start).count() << '\n';
				};
				if (wide) measure(wide_system);
				else measure(system);
			}
		}
		if (!feasible) continue;
		auto result = system.solve();
		auto wide_result = wide_system.solve();
		auto expected = result.upper_bounds(source);
		if (name == "reverse_dag") for (int v = 0; v < n; ++v) if (expected[v] != v - source) return 1;
		for (int run = -1; run < 7; ++run) {
			for (int order = 0; order < 2; ++order) {
				bool wide = (order + run + 1) % 2;
				auto measure = [&](const auto &chosen) {
					auto start = std::chrono::steady_clock::now();
					auto bounds = chosen.upper_bounds(source);
					auto stop = std::chrono::steady_clock::now();
					for (int v = 0; v < n; ++v) {
						if (bool(bounds[v]) != bool(expected[v])) std::exit(1);
						if (bounds[v] && *bounds[v] != *expected[v]) std::exit(1);
					}
					if (run >= 0) std::cout << name << '\t' << n << '\t' << system.constraint_count() << '\t' << (wide ? "128" : "64") << "\tupper_bounds\t" << run << '\t'
						<< std::chrono::duration<double, std::milli>(stop - start).count() << '\n';
				};
				if (wide) measure(wide_result);
				else measure(result);
			}
		}
	}
}
