#define PROBLEM "https://judge.yosupo.jp/problem/consecutive_terms_of_linear_recurrent_sequence"
#include "fps/rational_fps_coefficients_light.hpp"
#include "fixtures/fps_light_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0, count = 0;
	long long k = 0;
	input.read(d), input.read(k), input.read(count);
	light_fps a = read_light_fps(input, d), c = read_light_fps(input, d), q(d + 1);
	q[0] = 1;
	for (int i = 0; i < d; ++i) q[i + 1] = -c[i];
	light_fps p = light_fps::multiply(a, q).pre(d);
	write_light_fps(output, light_fps_algorithms::rational_fps_coefficients(std::move(p), std::move(q), k, k + count));
}
