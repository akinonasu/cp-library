#include "snapshot/test/fps_io.hpp"
#include "overlap_append.hpp"
#ifndef LENGTH
#define LENGTH 1024
#endif
int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto a = read_fps(input, n), b = read_fps(input, m);
	#if LENGTH == 0
	const auto len = std::max(std::size_t(64), std::bit_ceil(std::min(a.size(), b.size())) * FACTOR);
#else
	const auto len = LENGTH;
#endif
	write_fps(output, round4_overlap_append::multiply(a, b, len));
}
