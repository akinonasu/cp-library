#include "test/fixtures/fps_io.hpp"
#include "components/fps_io.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static fps read_series(FastInput& input, int n) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps result;
		fps_detail::read_coefficients(input, result, n);
		return result;
	}
#endif
	return read_fps(input, n);
}

static void write_series(FastOutput& output, const fps& series) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fps_detail::write_coefficients(output, series.data(), series.size());
		return;
	}
#endif
	write_fps(output, series);
}

static void require(bool value) { if (!value) std::abort(); }

static void check_output_words() {
	std::mt19937 rng(726315);
	const auto p = mint::mod();
	for (int n : {0, 1, 15, 16, 17, 25599, 25600, 25601, 51217})
		for (int offset : {0, 1, 7}) for (int pattern = 0; pattern < 4; ++pattern) {
			fps storage(n + offset);
			for (int i = 0; i < n; ++i) {
				unsigned word = rng() % p + (i & 1 ? p : 0);
				if (pattern == 0 || (pattern == 2 && i / 16 % 2)) word = i & 1 ? p : 0;
				if (pattern == 3) word = (i % 2 ? p : 0) + (1u << (i % 29));
				storage[offset + i] = mint::word(word);
			}
			const auto saved = storage;
			FILE *file = std::tmpfile();
			require(file != nullptr);
			{
				FastOutput output(file);
				output.println(71);
				fps_detail::write_coefficients(output, n ? storage.data() + offset : nullptr, n);
				output.println(72);
				require(output.flush());
			}
			std::rewind(file);
			std::string text;
			char buffer[4096];
			while (auto count = std::fread(buffer, 1, sizeof(buffer), file)) text.append(buffer, count);
			std::fclose(file);
			std::istringstream input(text);
			unsigned value = 0;
			require(bool(input >> value) && value == 71);
			for (int i = 0; i < n; ++i) require(bool(input >> value) && value == saved[offset + i].val());
			require(bool(input >> value) && value == 72);
			input >> std::ws;
			require(input.eof() && !text.empty() && text.back() == '\n');
			for (std::size_t i = 0; i < storage.size(); ++i) require(storage[i].v == saved[i].v);
		}
}

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
static void check_append() {
	for (int n : {0, 1, 63, 64, 65, 1023, 1024, 1025, 32769}) {
		std::vector<unsigned> values(n);
		std::string text;
		for (int i = 0; i < n; ++i) {
			values[i] = 2654435761u * unsigned(i + 1);
			text += std::to_string(values[i]) + " ";
		}
		text += "1234567";
		for (bool spare : {false, true}) {
			fps out = {mint::word(2 * mint::mod() - 1), mint::word(0), mint::word(mint::mod())};
			const auto prefix = out;
			if (spare) out.reserve(out.size() + n);
			FastInput input{std::string_view(text)};
			fps_detail::read_coefficients(input, out, n);
			require(out.size() == prefix.size() + n);
			for (std::size_t i = 0; i < prefix.size(); ++i) require(out[i].v == prefix[i].v);
			for (int i = 0; i < n; ++i) require(out[prefix.size() + i] == mint::raw(values[i]));
			unsigned last = 0;
			require(input.read(last) && last == 1234567 && input.token().empty());
		}
	}
}

static void check_chunks() {
	struct Recorder {
		std::vector<unsigned> values;
		int calls = 0;
		void operator()(const unsigned *data, std::size_t n) {
			++calls;
			values.insert(values.end(), data, data + n);
		}
	};
	using B = fast_io_detail::BulkInput;
	std::vector<unsigned> scratch(4 * B::kCapacity), values(20000);
	unsigned *const streams[4] = {scratch.data(), scratch.data() + B::kCapacity,
			scratch.data() + 2 * B::kCapacity, scratch.data() + 3 * B::kCapacity};
	std::string text;
	for (std::size_t i = 0; i < values.size(); ++i) {
		values[i] = 2654435761u * unsigned(i + 1);
		text += std::to_string(values[i]) + " ";
	}
	const char *p = text.data(), *const limit = p + text.size();
	std::size_t total = 0;
	Recorder recorder;
	while (values.size() - total >= 1024 && limit - p >= 1088) {
		const auto parsed = B::chunk_blocks(p, limit, values.size() - total, streams, recorder);
		require(parsed > 0);
		total += parsed;
		require(recorder.values.size() == total);
	}
	for (std::size_t i = 0; i < total; ++i) require(recorder.values[i] == values[i]);
	require(recorder.calls > 4 && recorder.calls % 4 == 0);
	for (const char *bad : {"123x", "4294967296"}) {
		for (int at : {0, 999, 1000, 1999, 2000, 2999, 3000}) {
			std::string invalid;
			for (int i = 0; i < 20000; ++i) invalid += i == at ? std::string(bad) + " " : "998244352 ";
			const char *first = invalid.data(), *current = first;
			Recorder rejected;
			require(B::chunk_blocks(current, first + invalid.size(), 20000, streams, rejected) == 0);
			require(current == first && rejected.calls == 0 && rejected.values.empty());
		}
	}
}
#endif

static void check(FastInput &input, const std::vector<unsigned> &values) {
	const fps actual = read_series(input, values.size());
	require(actual.size() == values.size());
	for (std::size_t i = 0; i < values.size(); ++i) require(actual[i] == mint::raw(values[i]));
	unsigned last = 0;
	require(input.read(last) && last == 1234567);
	FILE *file = std::tmpfile();
	require(file != nullptr);
	{
		FastOutput output(file);
		write_series(output, actual);
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
			const fps result = read_series(input, values.size());
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
		write_series(output, series);
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
		check_append();
		check_chunks();
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
		const fps parsed = read_series(input, 2000);
		for (auto x : parsed) require(x == mint(1));
		FILE *full = std::fopen("/dev/full", "w");
		if (full) {
			bool failed = false;
			{
				FastOutput output(full);
				try { write_series(output, series); }
				catch (const std::runtime_error &) { failed = true; }
			}
			require(failed);
			std::fclose(full);
		}
	}
#endif
}

int main() {
	check_output_words();
	check_parallel();
	for (int n : {0, 1, 15, 16, 17, 31, 32, 33, 47, 48, 49, 25601, 51201}) for (int pattern = 0; pattern < 3; ++pattern) {
		std::vector<unsigned> values(n);
		std::string text;
		for (int i = 0; i < n; ++i) {
			if ((pattern == 1 && (i / 16) % 2) || (pattern == 2 && i % 17 == 0)) values[i] = 998244352;
			text += std::to_string(values[i]) + " ";
		}
		text += "1234567";
		FastInput input{std::string_view(text)};
		check(input, values);
	}
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
	for (int n : {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 4095, 4096, 4097, 20000, 32767, 32768, 32769, 65535, 65536, 65537}) {
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
			require(mprotect(mapping, bytes, PROT_READ) == 0);
			{
				FastInput input{std::string_view(first, text.size())};
				check(input, values);
			}
			munmap(mapping, bytes + page);
#endif
		}
	}
}
