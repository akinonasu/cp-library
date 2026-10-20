#define PROBLEM "https://judge.yosupo.jp/problem/longest_increasing_subsequence"
#include "../string/lis.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<int> a(n);
	for (auto &x : a) in.read(x);
	auto indices = lis(a);
	out.println(indices.size());
	for (int i = 0; i < int(indices.size()); ++i) {
		if (i) out.write(' ');
		out.write(indices[i]);
	}
	out.write('\n');
}
