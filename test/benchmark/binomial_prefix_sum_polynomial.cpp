#include "binomial_prefix_sum_comparison.hpp"
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdio>
#include <string_view>

int main(int argc, char** argv) {
	if (argc != 2) return 2;
	std::string_view mode(argv[1]);
	if (mode != "baseline" && mode != "polynomial") return 2;
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	std::vector<std::pair<int, int>> queries(q);
	for (auto& [n, k] : queries) input.read(n), input.read(k);
	const auto begin = std::chrono::steady_clock::now();
	std::vector<mint> answers;
	if (mode == "baseline") answers = binomial_prefix_sum_baseline(queries);
	else answers = binomial_prefix_sum(queries);
	const auto end = std::chrono::steady_clock::now();
	std::fprintf(stderr, "%lld\n", static_cast<long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()));
	for (auto x : answers) output.println(x.val());
}
