#include "graph/warshall_floyd.hpp"
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>

using Distance = std::int64_t;
using Clock = std::chrono::steady_clock;

struct Generator {
	std::uint64_t state = 20261020;
	std::uint64_t next() {
		state += 0x9e3779b97f4a7c15ULL;
		auto x = state;
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
		x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
		return x ^ (x >> 31);
	}
};

template<class Emit> void generate(int n, const std::string& shape, Emit emit) {
	Generator rng;
	std::vector<Distance> potential(n);
	bool negative = shape == "negative32" || shape == "negative64";
	bool wide = shape == "positive64" || shape == "negative64";
	for (auto& x : potential) x = negative ? Distance(rng.next() % 20001) - 10000 : 0;
	Distance scale = wide ? 10000000 : 1;
	int range = negative ? 1000 : 1000000;
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) if (i != j) {
		Distance weight = Distance(1 + rng.next() % range) + potential[j] - potential[i];
		emit(i, j, scale * weight);
	}
}

int main(int argc, char** argv) {
	if (argc < 2) return 1;
	std::string mode = argv[1];
	int n = argc > 2 ? std::atoi(argv[2]) : 0;
	std::string shape = argc > 3 ? argv[3] : "positive32";
	if (mode == "generate") {
		FastOutput output;
		output.write(n); output.write(' '); output.println(Distance(n) * (n - 1));
		generate(n, shape, [&](int i, int j, Distance w) {
			output.write(i); output.write(' '); output.write(j); output.write(' '); output.println(w);
		});
		return 0;
	}
	if (mode == "io") {
		FastInput input;
		FastOutput output;
		Distance m = 0;
		input.read(n); input.read(m);
		WarshallFloyd<> graph(n);
		for (Distance k = 0; k < m; ++k) {
			int i = 0, j = 0;
			Distance w = 0;
			input.read(i); input.read(j); input.read(w); graph.add_edge(i, j, w);
		}
		auto start = Clock::now();
		bool ok = graph.solve();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
		std::fprintf(stderr, "%lld\n", static_cast<long long>(elapsed));
		if (!ok) return 2;
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
			if (j) output.write(' ');
			if (graph(i, j) == graph.infinity()) output.write("INF");
			else output.write(graph(i, j));
			if (j + 1 == n) output.write('\n');
		}
		return 0;
	}
	const Distance inf = std::numeric_limits<Distance>::max();
	std::vector<std::vector<Distance>> d;
	WarshallFloyd<> graph(mode == "baseline" ? 0 : n);
	if (mode == "baseline") {
		d.assign(n, std::vector<Distance>(n, inf));
		for (int i = 0; i < n; ++i) d[i][i] = 0;
		generate(n, shape, [&](int i, int j, Distance w) { d[i][j] = w; });
	} else generate(n, shape, [&](int i, int j, Distance w) { graph.add_edge(i, j, w); });
	auto start = Clock::now();
	if (mode == "baseline") {
		for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) {
			if (d[i][k] == inf) continue;
			for (int j = 0; j < n; ++j) {
				if (d[k][j] == inf) continue;
				d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
			}
		}
	} else if (!graph.solve()) return 2;
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
	std::uint64_t hash = 1469598103934665603ULL;
	for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
		hash = (hash ^ std::uint64_t(mode == "baseline" ? d[i][j] : graph(i, j))) * 1099511628211ULL;
	std::printf("%lld %llu\n", static_cast<long long>(elapsed), static_cast<unsigned long long>(hash));
}
