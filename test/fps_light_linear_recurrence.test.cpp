#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "fps/fps_light_algorithms.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	long long k = 0;
	in.read(n), in.read(k);
	std::vector<mint> initial(n), coefficients(n);
	for (auto &x : initial) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	for (auto &x : coefficients) { unsigned value = 0; in.read(value); x = mint::raw(value); }
	out.println(light_fps_algorithms::linear_recurrence(initial, coefficients, k).val());
}
