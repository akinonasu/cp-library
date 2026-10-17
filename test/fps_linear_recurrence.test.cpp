#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "fps/linear_recurrence.hpp"
#include "fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0; long long k = 0; input.read(d); input.read(k);
	auto a = read_fps(input, d), c = read_fps(input, d);
	output.println(linear_recurrence<mint>(a, c, k).val());
}
