#define PROBLEM "https://judge.yosupo.jp/problem/factorial"
#include "../math/factorial_mod.hpp"
#include "../components/fast_io.hpp"
int main() {
	FastInput in;
	FastOutput out;
	int t = 0; in.read(t);
	while (t--) { std::uint64_t n = 0; in.read(n); out.println(factorial_mod(n).val()); }
}
