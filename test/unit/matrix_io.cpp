#include "test/fixtures/matrix_common.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

constexpr std::array<std::array<int, 2>, 4> shapes{{{0, 37}, {37, 0}, {5, 7}, {65, 67}}};

unsigned value(int matrix, int row, int column) {
	return (std::uint64_t(matrix + 1) * 483271 + std::uint64_t(row) * 1234567 + column * 89123) % mint::mod();
}

void check(FastInput &input) {
	for (int k = 0; k < int(shapes.size()); ++k) {
		auto [n, m] = shapes[k];
		auto a = read_matrix(input, n, m);
		assert(a.rows() == n && a.cols() == m);
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) assert(a(i, j).val() == value(k, i, j));
	}
	unsigned last = 0;
	assert(input.read(last) && last == 123456789);
}

int main() {
	std::string text = " \n\t";
	for (int k = 0; k < int(shapes.size()); ++k) {
		auto [n, m] = shapes[k];
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j)
			text += std::to_string(value(k, i, j)) + (j + 1 == m ? "\n" : " ");
	}
	text += "123456789";
	FastInput memory{std::string_view(text)};
	check(memory);
	FILE *file = std::tmpfile();
	assert(file && std::fwrite(text.data(), 1, text.size(), file) == text.size());
	std::rewind(file);
	{
		FastInput mapped(file);
		check(mapped);
	}
	std::fclose(file);
	int descriptors[2];
	assert(pipe(descriptors) == 0);
	pid_t child = fork();
	assert(child >= 0);
	if (!child) {
		close(descriptors[0]);
		for (std::size_t sent = 0; sent < text.size();) {
			auto count = write(descriptors[1], text.data() + sent, text.size() - sent);
			if (count <= 0) _exit(1);
			sent += count;
		}
		close(descriptors[1]);
		_exit(0);
	}
	close(descriptors[1]);
	FILE *stream = fdopen(descriptors[0], "r");
	assert(stream);
	{
		FastInput piped(stream);
		check(piped);
	}
	std::fclose(stream);
	int status = 0;
	assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
