#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "math/mint.hpp"
#include "variants.hpp"

using Affine = std::array<mint, 2>;
long long sum_op(long long a, long long b) { return a + b; }
long long sum_e() { return 0; }
Affine affine_op(Affine a, Affine b) { return {a[0] * b[0], a[1] * b[0] + b[1]}; }
Affine affine_e() { return {1, 0}; }
struct Operation { int kind, u, v, a, b; };
std::vector<int> parents;
std::vector<Operation> operations;
int n;
std::string name, monoid, shape, mode, output;

template <template <class, auto, auto, bool> class Tree, bool affine>
int measure() {
	using S = std::conditional_t<affine, Affine, long long>;
	using LCT = std::conditional_t<affine, Tree<Affine, affine_op, affine_e, false>,
		Tree<long long, sum_op, sum_e, true>>;
	std::vector<S> values;
	values.reserve(n);
	for (int i = 0; i < n; ++i) {
		if constexpr (affine) values.push_back({i + 1, i * 3 + 1});
		else values.push_back(i + 1);
	}
	std::vector<std::uint64_t> answers;
	answers.reserve(operations.size() * 2);
	auto begin = std::chrono::steady_clock::now();
	LCT tree(values);
	for (int i = 1; i < n; ++i) {
		if (!tree.link(i, parents[i])) return 2;
	}
	auto built = std::chrono::steady_clock::now();
	for (auto [kind, u, v, a, b] : operations) {
		if (kind == 0) {
			auto result = tree.prod(u, v);
			if constexpr (affine) {
				answers.push_back(result[0].val());
				answers.push_back(result[1].val());
			} else answers.push_back(result);
		} else if (kind == 1) {
			if constexpr (affine) tree.set(u, {a, b});
			else tree.set(u, a);
		} else {
			if (!tree.cut(u, v) || !tree.link(u, a)) return 3;
		}
	}
	auto end = std::chrono::steady_clock::now();
	std::uint64_t digest = 0;
	for (auto value : answers) digest = digest * 1000000007 + value;
	if (!output.empty()) {
		std::ofstream file(output, std::ios::binary);
		file.write(reinterpret_cast<const char *>(answers.data()), answers.size() * sizeof(answers[0]));
	}
	auto seconds = [](auto a, auto b) { return std::chrono::duration<double>(b - a).count(); };
	std::cout << "{\"variant\":\"" << name << "\",\"monoid\":\"" << monoid
		<< "\",\"shape\":\"" << shape << "\",\"mode\":\"" << mode
		<< "\",\"n\":" << n << ",\"q\":" << operations.size()
		<< ",\"build_s\":" << seconds(begin, built) << ",\"operations_s\":" << seconds(built, end)
		<< ",\"answers\":" << answers.size() << ",\"digest\":\"" << digest << "\"}\n";
	return 0;
}

template <template <class, auto, auto, bool> class Tree>
int run() {
	if (monoid == "affine") return measure<Tree, true>();
	return measure<Tree, false>();
}

int main(int argc, char **argv) {
	if (argc != 9) return 1;
	name = argv[1]; monoid = argv[2]; shape = argv[3]; mode = argv[4];
	n = std::stoi(argv[5]); int q = std::stoi(argv[6]);
	std::mt19937 random(std::stoul(argv[7]));
	output = argv[8];
	parents.assign(n, -1);
	std::vector<int> children(n);
	for (int i = 1; i < n; ++i) {
		int p;
		if (shape == "line") p = i - 1;
		else if (shape == "star") p = 0;
		else if (shape == "binary") p = (i - 1) / 2;
		else if (shape == "broom") p = i < n / 2 ? i - 1 : n / 2 - 1;
		else p = random() % i;
		parents[i] = p;
		++children[p];
	}
	std::vector<int> leaves, inner;
	for (int i = 0; i < n; ++i) (children[i] ? inner : leaves).push_back(i);
	auto current = parents;
	operations.reserve(q);
	for (int i = 0; i < q; ++i) {
		int kind = random() % 100, u = random() % n, v = random() % n;
		if (mode == "query" || (mode == "mixed" && kind < 50) || (mode == "update" && kind < 10))
			operations.push_back({0, u, v, 0, 0});
		else if (mode == "update" || kind < 80)
			operations.push_back({1, u, 0, int(random() % 998244353), int(random() % 998244353)});
		else {
			u = leaves[random() % leaves.size()];
			int p = inner[random() % inner.size()];
			operations.push_back({2, u, current[u], p, 0});
			current[u] = p;
		}
	}
#include "dispatch.inc"
	return 1;
}
