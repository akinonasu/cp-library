#include "string/run_enumerate.hpp"
#include "components/fast_io.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#if CP_RUN_VARIANT == 0
#include "baseline.hpp"
#elif CP_RUN_VARIANT == 1
#include "filtered.hpp"
#elif CP_RUN_VARIANT == 2
#include "cached.hpp"
#elif CP_RUN_VARIANT == 3
#include "reference.hpp"
#elif CP_RUN_VARIANT == 5
#include "guarded.hpp"
#elif CP_RUN_VARIANT == 6
#include "unindexed.hpp"
#endif

std::vector<Run> solve(std::string_view s) {
#if CP_RUN_VARIANT == 0
	return run_enumerate_baseline(s);
#elif CP_RUN_VARIANT == 1
	return run_enumerate_filtered(s);
#elif CP_RUN_VARIANT == 2
	return run_enumerate_cached(s);
#elif CP_RUN_VARIANT == 5
	return run_enumerate_guarded(s);
#elif CP_RUN_VARIANT == 6
	return run_enumerate_unindexed(s);
#elif CP_RUN_VARIANT == 3
	int n = static_cast<int>(s.size());
	std::vector<Run> runs;
	if (n < 2) return runs;
	std::vector<std::uint16_t> text(n + 2);
	for (int i = 0; i < n; ++i) text[i + 1] = std::uint8_t(s[i]) + 1;
	for (auto run : linear_time_runs::compute_all_runs(text.data(), text.size()))
		runs.push_back({int(run.period), int(run.start - 1), int(run.start - 1 + run.length)});
	run_enumerate_detail::sort_runs(runs, n);
	return runs;
#else
	return run_enumerate(s);
#endif
}

std::string make_input(std::string kind, int n) {
	std::mt19937_64 rng(713);
	std::string s(n, 'a');
	if (kind == "random" || kind == "binary") for (char &c : s) c += rng() % (kind == "random" ? 26 : 2);
	else if (kind == "periodic") for (int i = 0; i < n; ++i) s[i] += i % 7;
	else if (kind == "thue") for (int i = 0; i < n; ++i) s[i] += std::popcount(unsigned(i)) % 2;
	else if (kind == "fibonacci") {
		std::string a = "a", b = "ab";
		while (int(b.size()) < n) { auto next = b + a; a = std::move(b); b = std::move(next); }
		s = b.substr(0, n);
	} else if (kind == "blocks") {
		for (int i = 0; i < n; i += 257) s[i] = 'b';
	} else if (kind == "near_periodic" || kind == "long_period" || kind == "period64" || kind == "period4096") {
		std::string block(kind == "period64" ? 64 : kind == "period4096" ? 4096 : 1024, 'a');
		for (char &c : block) c += rng() % 2;
		for (int i = 0; i < n; ++i) s[i] = block[i % block.size()];
		if (kind == "near_periodic") for (int i = 1023; i < n; i += 8191) s[i] = 'c';
	}
	return s;
}

int main(int argc, char **argv) {
	if (argc == 4) {
		int n = std::stoi(argv[2]), repetitions = std::stoi(argv[3]);
		auto s = make_input(argv[1], n);
		std::uint64_t digest = 0;
		std::size_t count = 0;
		auto begin = std::chrono::steady_clock::now();
		for (int repeat = 0; repeat < repetitions; ++repeat) {
			auto runs = solve(s);
			count = runs.size();
			for (auto run : runs) digest = digest * 1000000007 + std::uint64_t(run.period) * 1000003 + std::uint64_t(run.l) * 1009 + run.r;
		}
		double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count() / repetitions;
		std::cout << "{\"count\":" << count << ",\"digest\":\"" << digest << "\",\"ms\":" << ms << "}\n";
		return 0;
	}
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	auto runs = solve(s);
	out.println(runs.size());
	for (auto [period, l, r] : runs) {
		out.write(period); out.write(' ');
		out.write(l); out.write(' ');
		out.println(r);
	}
}
