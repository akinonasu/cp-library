#define PROBLEM "https://judge.yosupo.jp/problem/find_linear_recurrence"
#include "../fps/berlekamp_massey.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	std::vector<mint> a(n);
	for (auto &x : a) {
		unsigned value = 0;
		in.read(value);
		x = mint::raw(value);
	}
	auto c = berlekamp_massey(a);
	out.println(c.size());
	for (std::size_t i = 0; i < c.size(); ++i) {
		if (i) out.write(' ');
		out.write(c[i].val());
	}
	out.write('\n');
}
