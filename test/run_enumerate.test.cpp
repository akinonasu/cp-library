#define PROBLEM "https://judge.yosupo.jp/problem/runenumerate"
#include "../string/run_enumerate.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	auto runs = run_enumerate(s);
	out.println(runs.size());
	for (auto [period, l, r] : runs) {
		out.write(period);
		out.write(' ');
		out.write(l);
		out.write(' ');
		out.println(r);
	}
}
