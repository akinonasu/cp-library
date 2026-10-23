#ifndef TEST_FPS_IO_HPP
#define TEST_FPS_IO_HPP

#include "components/fast_io.hpp"
#include "fps/fps.hpp"

inline fps read_fps(FastInput &input, int n) {
	fps result;
	result.resize(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_fps(FastOutput &output, const fps &f) {
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}

#endif
