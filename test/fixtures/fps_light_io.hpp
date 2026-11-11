#ifndef TEST_FPS_LIGHT_IO_HPP
#define TEST_FPS_LIGHT_IO_HPP

#include "fps/fps_light.hpp"
#include "components/fast_io.hpp"

inline fps read_light_fps(FastInput &input, int n) {
	fps result(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_light_fps(FastOutput &output, const fps &f) {
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}

#endif
