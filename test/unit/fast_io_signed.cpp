#include "components/fast_io_bulk.hpp"
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

static void check(FastInput &input, const std::vector<std::int32_t> &expected) {
	std::vector<std::int32_t> actual;
	read_signed_blocks(input, expected.size(), [&](const std::int32_t *values, std::size_t n) {
		require(n > 0 && n <= expected.size() - actual.size());
		actual.insert(actual.end(), values, values + n);
	});
	require(actual == expected);
	int last = 0;
	require(input.read(last) && last == 1234567 && input.token().empty());
}

template <std::size_t Fields>
static void records(std::size_t count, bool irregular) {
	std::vector<std::int32_t> expected(Fields * count);
	std::string text;
	for (std::size_t i = 0; i < expected.size(); ++i) {
		expected[i] = std::int32_t(i * 2654435761u);
		text += std::to_string(expected[i]) + (irregular ? "\r\n\t  " : " ");
	}
	text += "1234567";
	FastInput input{std::string_view(text)};
	std::size_t at = 0, calls = 0;
	read_signed_records<Fields>(input, count, [&](auto... x) {
		const std::int32_t values[] = {x...};
		require(at + Fields <= expected.size());
		for (auto value : values) require(value == expected[at++]);
		++calls;
	});
	require(at == expected.size() && calls == count);
	int last = 0;
	require(input.read(last) && last == 1234567 && input.token().empty());
}

int main() {
	std::mt19937 random(115631);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 63, 64, 65, 1023, 1024, 1025, 32767, 32768, 32769, 65537}) {
		std::vector<std::int32_t> values(n);
		for (auto &value : values) value = std::int32_t(random());
		const std::int32_t edge[] = {0, -1, 1, -9, 9, -10, 10, -9999, 10000, -99999999, 100000000, -2147483647 - 1, 2147483647};
		for (std::size_t i = 0; i < std::size(edge) && i < values.size(); ++i) values[i] = edge[i];
		for (bool irregular : {false, true}) {
			std::string text = " \t\r\n";
			for (auto value : values) text += std::to_string(value) + (irregular ? "\r\n \t " : " ");
			text += "1234567";
			{
				FastInput input{std::string_view(text)};
				check(input, values);
			}
			FILE *file = std::tmpfile();
			require(file != nullptr);
			require(std::fwrite(text.data(), 1, text.size(), file) == text.size());
			std::rewind(file);
			{
				FastInput input(file);
				check(input, values);
			}
			std::fclose(file);
#if defined(__linux__)
			file = fmemopen(text.data(), text.size(), "r");
			require(file != nullptr);
			{
				FastInput input(file);
				check(input, values);
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
				check(input, values);
			}
			require(munmap(mapping, bytes + page) == 0);
#endif
		}
	}
	for (std::size_t count : {0, 1, 2, 3, 63, 64, 65, 1023, 1024, 1025, 20001}) for (bool irregular : {false, true}) {
		records<1>(count, irregular);
		records<3>(count, irregular);
		records<7>(count, irregular);
	}
	{
		FastInput input("13");
		bool failed = false;
		try { read_signed_records<3>(input, std::numeric_limits<std::size_t>::max(), [](auto...) {}); }
		catch (const std::length_error &) { failed = true; }
		require(failed);
		int value = 0;
		require(input.read(value) && value == 13);
	}
	{
		FastInput input("-0 0000 -00000000000000000000000000001 -2147483648 2147483647 1234567");
		check(input, {0, 0, -1, -2147483647 - 1, 2147483647});
	}
	{
		std::string text;
		for (int i = 0; i < 2048; ++i) text += "-00000000000000000000000000001 ";
		text += "1234567";
		FastInput input{std::string_view(text)};
		check(input, std::vector<std::int32_t>(2048, -1));
	}
#if defined(__unix__) || defined(__APPLE__)
	{
		std::vector<std::int32_t> values(20000);
		std::string text;
		for (auto &value : values) value = std::int32_t(random()), text += std::to_string(value) + " ";
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
			check(input, values);
		}
		std::fclose(file);
		writer.join();
	}
#endif
}
