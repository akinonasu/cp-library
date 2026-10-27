#ifndef CP_COMPONENTS_FPS_IO_HPP
#define CP_COMPONENTS_FPS_IO_HPP

#include "fast_io_bulk.hpp"
#include "fps_output.hpp"
#include "fps_write.hpp"
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
inline void read_coefficients(FastInput &input, std::vector<mint> &out, std::size_t n) {
	if (!n) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps input");
	prepare_fps_output(out, out.size() + n);
	fast_io_detail::BulkInput::read_blocks(input, n, [&](const unsigned *values, std::size_t count) {
		append_fps_words<false>(out, values, count);
	});
}
}
#pragma GCC pop_options
#endif
#endif
