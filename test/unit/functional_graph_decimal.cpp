#include "graph/functional_graph.hpp"
#include <cassert>
#include <numeric>
#include <random>
#include <string>
#include <vector>

std::vector<int> reference(const std::vector<int> &to, std::string_view decimal) {
	std::vector<int> result(to.size()), next(to.size());
	std::iota(result.begin(), result.end(), 0);
	for (char c : decimal) {
		for (int v = 0; v < int(to.size()); ++v) {
			int u = v;
			for (int repeat = 0; repeat < 10; ++repeat) u = result[u];
			for (int repeat = 0; repeat < c - '0'; ++repeat) u = to[u];
			next[v] = u;
		}
		result.swap(next);
	}
	return result;
}

void check(const std::vector<int> &to, std::string_view decimal) {
	FunctionalGraph graph(to);
	auto expected = reference(to, decimal);
	for (int v = 0; v < graph.size(); ++v) assert(graph.jump(v, decimal) == expected[v]);
	assert(graph.to == to);
}

int main() {
	std::mt19937_64 random(102443);
	const std::vector<std::string> examples{"0", "00", "0000001", "1", "2", "13", "18446744073709551614", "18446744073709551615", "18446744073709551616", "340282366920938463463374607431768211455"};
	for (int trial = 0; trial < 100; ++trial) {
		int n = 1 + int(random() % 35);
		std::vector<int> to(n);
		for (int &u : to) u = int(random() % n);
		for (const auto &decimal : examples) check(to, decimal);
		std::string huge(100 + random() % 200, '0');
		for (char &c : huge) c += random() % 10;
		check(to, huge);
		FunctionalGraph graph(to);
		for (std::uint64_t k : {std::uint64_t(0), std::uint64_t(n), UINT64_MAX - 1, UINT64_MAX})
			for (int v = 0; v < n; ++v) assert(graph.jump(v, std::to_string(k)) == graph.jump(v, k));
	}
	check({0}, std::string(100'000, '9'));
	check({1, 2, 3, 4, 2, 5, 6, 7}, std::string(10'000, '0') + "3");
	FunctionalGraph self({0});
	assert(self.jump(0, 0) == 0);
	for (std::string_view invalid : {"", "-1", "+1", " 1", "1 ", "1a", "1.0", "-0"}) {
		bool thrown = false;
		try { self.jump(0, invalid); } catch (const std::invalid_argument &) { thrown = true; }
		assert(thrown);
	}
	bool thrown = false;
	try { self.jump(0, std::string_view("1\0", 2)); } catch (const std::invalid_argument &) { thrown = true; }
	assert(thrown);
}
