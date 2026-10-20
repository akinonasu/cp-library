#include "snapshot/test/fps_io.hpp"
#include "overlap.hpp"
#ifndef LENGTH
#define LENGTH 1024
#endif
int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0; input.read(n); input.read(m);
	auto a = read_fps(input, n), b = read_fps(input, m);
	write_fps(output, round4_overlap::multiply(a, b, LENGTH));
}
