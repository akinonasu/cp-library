#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "../data_structure/range_query/offline_range_kth_smallest.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	uint32_t n = 0, q = 0;
	input.read(n); input.read(q);
	vector<uint32_t> a(n);
	for (auto &value : a) input.read(value);
	vector<array<int, 3>> queries(q);
	for (auto &[l, r, k] : queries) input.read(l), input.read(r), input.read(k);
	for (auto answer : offline_range_kth_smallest(a, queries)) output.println(answer);
}
