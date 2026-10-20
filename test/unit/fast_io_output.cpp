#include "components/fast_io_bulk.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <vector>
#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static void require(bool condition) { if (!condition) std::abort(); }

static std::string contents(FILE *file) {
	std::rewind(file);
	std::string result;
	char buffer[4096];
	for (;;) {
		auto n = std::fread(buffer, 1, sizeof(buffer), file);
		result.append(buffer, n);
		if (n != sizeof(buffer)) break;
	}
	require(!std::ferror(file));
	return result;
}

static std::vector<std::uint32_t> boundaries() {
	std::vector<std::uint32_t> result = {0, 1, 2, std::numeric_limits<std::uint32_t>::max() - 1, std::numeric_limits<std::uint32_t>::max()};
	for (std::uint64_t power = 10; power <= 1000000000; power *= 10)
		for (int delta = -1; delta <= 1; ++delta) result.push_back(std::uint32_t(power + delta));
	return result;
}

template<std::size_t Fields>
static void records(std::size_t count, std::mt19937 &random, bool flush) {
	std::vector<std::array<std::uint32_t, Fields>> rows(count);
	const auto edge = boundaries();
	std::size_t at = 0;
	std::string expected = "prefix\n";
	for (auto &row : rows) {
		for (auto &value : row) {
			value = at < edge.size() ? edge[at] : std::uint32_t(random());
			++at;
		}
		for (std::size_t j = 0; j < Fields; ++j) {
			expected += std::to_string(row[j]);
			expected += j + 1 == Fields ? '\n' : ' ';
		}
	}
	expected += "suffix\n";
	FILE *file = std::tmpfile();
	require(file != nullptr);
	{
		FastOutput out(file);
		out.write("prefix\n");
		std::size_t calls = 0;
		if (flush) {
			for (std::size_t i = 0; i < count; ++i) {
				write_unsigned_records<Fields>(out, 1, [&](std::size_t j) {
					require(j == 0);
					++calls;
					return std::span<const std::uint32_t, Fields>(rows[i]);
				});
				require(out.flush() && out.flush());
			}
		} else write_unsigned_records<Fields>(out, count, [&](std::size_t i) {
			require(i == calls++);
			return rows[i];
		});
		require(calls == count);
		write_unsigned_records<Fields>(out, 0, [](std::size_t) {
			std::abort();
			return std::array<std::uint32_t, Fields>{};
		});
		out.write("suffix\n");
		require(out.flush());
	}
	require(contents(file) == expected);
	std::fclose(file);
}

static void guard_boundary() {
	const auto edge = boundaries();
	for (auto value : edge) {
		std::array<char, 32> guarded;
		guarded.fill('!');
		auto end = fast_io_detail::BulkOutput::number(guarded.data() + 7, value);
		require(std::string(guarded.data() + 7, end) == std::to_string(value));
		for (int i = 0; i < 7; ++i) require(guarded[i] == '!');
		for (std::size_t i = 18; i < guarded.size(); ++i) require(guarded[i] == '!');
	}
#if defined(__linux__)
	const auto page = std::size_t(sysconf(_SC_PAGESIZE));
	void *mapping = mmap(nullptr, 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	require(mapping != MAP_FAILED);
	require(mprotect(static_cast<char *>(mapping) + page, page, PROT_NONE) == 0);
	for (auto value : edge) {
		char *begin = static_cast<char *>(mapping) + page - 11;
		auto end = fast_io_detail::BulkOutput::number(begin, value);
		require(std::string(begin, end) == std::to_string(value));
	}
	require(munmap(mapping, 2 * page) == 0);
#endif
}

int main() {
	std::mt19937 random(20261019);
	guard_boundary();
	for (std::size_t count : {0, 1, 2, 3, 31, 32, 33, 1023, 1024, 1025, 2977, 2978, 2979, 5956, 5957, 5958, 20001}) {
		records<1>(count, random, false);
		records<2>(count, random, false);
		records<3>(count, random, false);
		records<7>(count, random, false);
	}
	records<1>(257, random, true);
	records<2>(257, random, true);
	records<3>(257, random, true);
	records<7>(257, random, true);
	records<7000>(3, random, false);
	records<7000>(3, random, true);
	records<3>(500001, random, false);
	{
		FILE *file = std::tmpfile();
		require(file != nullptr);
		{
			FastOutput out(file);
			out.write(std::string(65535, 'x'));
			write_unsigned_records<2>(out, 1, [](std::size_t) { return std::array<std::uint32_t, 2>{0, 4294967295u}; });
			out.write('y');
		}
		require(contents(file) == std::string(65535, 'x') + "0 4294967295\ny");
		std::fclose(file);
	}
#if defined(__linux__)
	{
		FILE *file = std::fopen("/dev/full", "w");
		require(file != nullptr);
		require(std::setvbuf(file, nullptr, _IONBF, 0) == 0);
		{
			FastOutput out(file);
			write_unsigned_records<1>(out, 1, [](std::size_t) { return std::array<std::uint32_t, 1>{7}; });
			require(!out.flush());
			bool failed = false;
			try {
				write_unsigned_records<2>(out, 10000, [](std::size_t) { return std::array<std::uint32_t, 2>{0, 4294967295u}; });
			} catch (const std::runtime_error &) { failed = true; }
			require(failed);
		}
		std::fclose(file);
	}
#endif
}
