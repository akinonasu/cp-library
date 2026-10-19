#include "../../components/fast_io_bulk.hpp"
#include <cstdlib>
#include <random>
#include <string>
#include <thread>
#include <vector>
#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static void require(bool value) { if (!value) std::abort(); }

static void check(FastInput &input, const std::vector<std::uint32_t> &expected, bool blocks) {
	std::vector<std::uint32_t> actual(expected.size(), 37);
	if (blocks) {
		std::size_t at = 0;
		read_unsigned_blocks(input, actual.size(), [&](const std::uint32_t *values, std::size_t n) {
			require(n > 0 && n <= actual.size() - at);
			std::copy_n(values, n, actual.data() + at);
			at += n;
		});
		require(at == actual.size());
	} else read_unsigned(input, actual);
	require(actual == expected);
	std::uint32_t last = 0;
	require(input.read(last) && last == 1234567 && input.token().empty());
}

template <std::size_t Fields>
static void records(std::size_t count, bool irregular) {
	std::vector<std::uint32_t> expected(Fields * count);
	std::string text;
	for (std::size_t i = 0; i < expected.size(); ++i) {
		expected[i] = std::uint32_t(i * 2654435761u);
		text += std::to_string(expected[i]) + (irregular ? "\r\n\t  " : " ");
	}
	text += "1234567";
	FastInput input{std::string_view(text)};
	std::size_t at = 0, calls = 0;
	read_unsigned_records<Fields>(input, count, [&](auto... x) {
		const std::uint32_t values[] = {x...};
		require(at + Fields <= expected.size());
		for (auto value : values) require(value == expected[at++]);
		++calls;
	});
	require(at == expected.size() && calls == count);
	std::uint32_t last = 0;
	require(input.read(last) && last == 1234567 && input.token().empty());
}

int main() {
	std::mt19937 random(20261019);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 32767, 32768, 32769, 65537}) {
		std::vector<std::uint32_t> values(n);
		for (auto &value : values) value = random();
		const std::uint32_t edge[] = {0, 1, 9, 10, 99, 100, 9999, 10000, 99999999, 100000000, 4294967295u};
		for (std::size_t i = 0; i < std::size(edge) && i < values.size(); ++i) values[i] = edge[i];
		for (bool irregular : {false, true}) for (bool blocks : {false, true}) {
			std::string text = " \t\r\n";
			for (auto value : values) text += std::to_string(value) + (irregular ? "\r\n \t " : " ");
			text += "1234567";
			{
				FastInput input{std::string_view(text)};
				check(input, values, blocks);
			}
			FILE *file = std::tmpfile();
			require(file != nullptr);
			require(std::fwrite(text.data(), 1, text.size(), file) == text.size());
			std::rewind(file);
			{
				FastInput input(file);
				check(input, values, blocks);
			}
			std::fclose(file);
#if defined(__linux__)
			file = fmemopen(text.data(), text.size(), "r");
			require(file != nullptr);
			{
				FastInput input(file);
				check(input, values, blocks);
			}
			std::fclose(file);
			const auto page = std::size_t(sysconf(_SC_PAGESIZE));
			const auto bytes = (text.size() + page - 1) / page * page;
			void *mapping = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			require(mapping != MAP_FAILED);
			require(mprotect(static_cast<char *>(mapping) + bytes, page, PROT_NONE) == 0);
			char *first = static_cast<char *>(mapping) + bytes - text.size();
			std::memcpy(first, text.data(), text.size());
			{
				FastInput input{std::string_view(first, text.size())};
				check(input, values, blocks);
			}
			require(munmap(mapping, bytes + page) == 0);
#endif
		}
	}
	for (std::size_t n : {0, 1, 2, 3, 63, 64, 65, 1023, 1024, 1025, 20001}) for (bool irregular : {false, true}) {
		records<1>(n, irregular);
		records<2>(n, irregular);
		records<3>(n, irregular);
		records<5>(n, irregular);
		records<7>(n, irregular);
	}
	{
		FastInput input("13");
		bool failed = false;
		try { read_unsigned_records<3>(input, std::numeric_limits<std::size_t>::max(), [](auto...) {}); }
		catch (const std::length_error &) { failed = true; }
		require(failed);
		unsigned value = 0;
		require(input.read(value) && value == 13);
	}
	{
		std::string text;
		for (int i = 0; i < 2048; ++i) text += "00000000000000000000000000000000001 ";
		FastInput input{std::string_view(text)};
		std::vector<std::uint32_t> values(2048);
		read_unsigned(input, values);
		for (auto value : values) require(value == 1);
		require(input.token().empty());
	}
#if defined(__unix__) || defined(__APPLE__)
	{
		std::vector<std::uint32_t> values(20000);
		std::string text;
		for (auto &value : values) value = random(), text += std::to_string(value) + " ";
		text += "1234567";
		int fd[2];
		require(pipe(fd) == 0);
		std::thread writer([&] {
			FILE *file = fdopen(fd[1], "w");
			require(file != nullptr);
			require(std::fwrite(text.data(), 1, text.size(), file) == text.size());
			std::fclose(file);
		});
		FILE *file = fdopen(fd[0], "r");
		require(file != nullptr);
		{
			FastInput input(file);
			check(input, values, false);
		}
		std::fclose(file);
		writer.join();
	}
#endif
}
