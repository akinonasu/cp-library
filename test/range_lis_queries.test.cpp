#define PROBLEM "https://judge.yosupo.jp/problem/static_range_lis_query"
#include "data_structure/range_lis_query.hpp"
#include "components/fast_io.hpp"
#include <utility>
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<int> values(n);
	for (auto &value : values) input.read(value);
	std::vector<std::pair<int, int>> intervals(q);
	for (auto &[l, r] : intervals) input.read(l), input.read(r);
	for (int answer : range_lis_queries_sqrt(values, intervals)) output.println(answer);
}
