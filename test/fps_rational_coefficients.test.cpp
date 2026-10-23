#define PROBLEM "https://judge.yosupo.jp/problem/consecutive_terms_of_linear_recurrent_sequence"
#include "fps/rational_fps_coefficients.hpp"
#include "fixtures/fps_bulk_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0, count = 0;
	long long k = 0;
	input.read(d), input.read(k), input.read(count);
	fps a = read_fps(input, d), c = read_fps(input, d), q(d + 1);
	q[0] = 1;
	for (int i = 0; i < d; ++i) q[i + 1] = -c[i];
	fps p = fps::multiply(a, q).pre(d);
	write_fps(output, rational_fps_coefficients(std::move(p), std::move(q), k, k + count));
}
