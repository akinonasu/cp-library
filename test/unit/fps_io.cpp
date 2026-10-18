#include "test/fps_io.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <thread>
#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static void require(bool value) { if (!value) std::abort(); }

static void check(FastInput &input, const std::vector<unsigned> &values) {
	const fps actual = read_fps(input, values.size());
	require(actual.size() == values.size());
	for (std::size_t i = 0; i < values.size(); ++i) require(actual[i] == mint::raw(values[i]));
	unsigned last = 0;
	require(input.read(last) && last == 1234567);
	FILE *file = std::tmpfile();
	require(file != nullptr);
	{
		FastOutput output(file);
		write_fps(output, actual);
		require(output.flush());
	}
	std::rewind(file);
	{
		FastInput output(file);
		for (auto expected : actual) {
			unsigned x = 0;
			require(output.read(x) && x == expected.val());
		}
		require(output.token().empty());
	}
	std::fclose(file);
}

static void check_parallel() {
	std::vector<unsigned> values(20000);
	std::string text;
	for (std::size_t i = 0; i < values.size(); ++i) {
		values[i] = unsigned(i * 1234567);
		text += std::to_string(values[i]) + " ";
	}
	const auto run = [&] {
		for (int rep = 0; rep < 3; ++rep) {
			FastInput input{std::string_view(text)};
			const fps result = read_fps(input, values.size());
			for (std::size_t i = 0; i < values.size(); ++i) require(result[i] == mint::raw(values[i]));
		}
	};
	std::thread a(run), b(run), c(run);
	a.join();
	b.join();
	c.join();
	FILE *file = std::tmpfile();
	require(file != nullptr);
	fps series;
	for (auto x : values) series.emplace_back(x);
	{
		FastOutput output(file);
		output.println(71);
		write_fps(output, series);
		output.println(72);
		require(output.flush());
	}
	std::rewind(file);
	{
		FastInput input(file);
		unsigned value = 0;
		require(input.read(value) && value == 71);
		for (auto x : series) require(input.read(value) && value == x.val());
		require(input.read(value) && value == 72 && input.token().empty());
	}
	std::fclose(file);
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		for (const char *bad : {"123x", "4294967296", "99999999999", "9999999999999999999999999999999999999999999999999999999999999999999"}) {
			std::string source;
			for (int i = 0; i < 2000; ++i) source += i == 64 ? std::string(bad) + " " : "998244352 ";
			const char *at = source.data(), *const first = at;
			std::vector<unsigned> output(2000), scratch(3 * fast_io_detail::BulkInput::kCapacity);
			require(fast_io_detail::BulkInput::chunk(at, at + source.size(), output.data(), output.size(), scratch.data()) == 0);
			require(at == first);
		}
		std::string leading;
		for (int i = 0; i < 2000; ++i) leading += "00000000000000000000000000000000000000000000000000000000000000001 ";
		FastInput input{std::string_view(leading)};
		const fps parsed = read_fps(input, 2000);
		for (auto x : parsed) require(x == mint(1));
		FILE *full = std::fopen("/dev/full", "w");
		if (full) {
			bool failed = false;
			{
				FastOutput output(full);
				try { write_fps(output, series); }
				catch (const std::runtime_error &) { failed = true; }
			}
			require(failed);
			std::fclose(full);
		}
	}
#endif
}

int main() {
	check_parallel();
	std::mt19937 rng(20261018);
	for (bool single : {false, true}) {
		std::vector<unsigned> values(20000);
		std::string text;
		for (auto &x : values) {
			x = single ? rng() % 10 : 100000000 + rng() % 898244353;
			text += std::to_string(x) + " ";
		}
		text += "1234567";
		FastInput input{std::string_view(text)};
		check(input, values);
	}
	for (int n : {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 4095, 4096, 4097, 20000}) {
		std::vector<unsigned> values(n);
		for (auto &x : values) x = rng();
		const unsigned edge[] = {0, 1, 9, 10, 99, 100, 9999, 10000, 99999999, 100000000, 998244352, 4294967295u};
		for (std::size_t i = 0; i < values.size() && i < std::size(edge); ++i) values[i] = edge[i];
		for (bool irregular : {false, true}) {
			std::string text;
			for (auto x : values) text += std::to_string(x) + (irregular ? "\r\n \t " : " ");
			text += "1234567";
			{
				FastInput input{std::string_view(text)};
				check(input, values);
			}
#if defined(__linux__)
			FILE *memory = fmemopen(text.data(), text.size(), "r");
			require(memory != nullptr);
			{
				FastInput input(memory);
				check(input, values);
			}
			std::fclose(memory);
			const auto page = std::size_t(sysconf(_SC_PAGESIZE));
			const auto bytes = (text.size() + page - 1) / page * page;
			void *mapping = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			require(mapping != MAP_FAILED);
			require(mprotect(static_cast<char *>(mapping) + bytes, page, PROT_NONE) == 0);
			char *first = static_cast<char *>(mapping) + bytes - text.size();
			std::memcpy(first, text.data(), text.size());
			{
				FastInput input{std::string_view(first, text.size())};
				check(input, values);
			}
			munmap(mapping, bytes + page);
#endif
		}
	}
}
