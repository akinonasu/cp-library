#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "../data_structure/static_threshold_search.hpp"
#include "../components/fast_io.hpp"
#include "../components/static_range_min.hpp"
int main() {
	FastInput in; FastOutput out;
	int n = 0, q = 0; in.read(n); in.read(q);
	std::vector<int> a(n); for (auto &x : a) in.read(x);
	StaticRangeMin<int> minimum(a);
	StaticThresholdSearch<int, true> search(std::move(a));
	while (q--) { int l = 0, r = 0; in.read(l); in.read(r); int value = minimum.prod(l, r); int p = search.first(l, r, value); out.println(search[p]); }
}
