#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "fps/bostan_mori.hpp"
#include "fixtures/fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int d = 0; long long k = 0; input.read(d); input.read(k);
	auto a = read_fps(input, d), q = read_fps(input, d);
	for (auto &x : q) x = -x;
	q.insert(q.begin(), mint(1));
	fps p = (a * q).pre(d);
	output.println(bostan_mori(std::move(p), std::move(q), k).val());
}
