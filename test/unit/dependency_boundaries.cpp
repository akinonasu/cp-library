#define COMB_MAX 32
#include "math/combinatorics/comb.hpp"
#include "math/combinatorics/binomial_prefix_sum_incremental.hpp"
#include "components/factorial_table.hpp"
#include "components/fps_factorials.hpp"
#include "components/fps_horner.hpp"
#include "components/fps_direct_eval.hpp"
#include "data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp"
#include "data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp"

#if defined(NTT_998244353_HPP) || defined(CP_FPS_BACKEND_HPP) || defined(WAVELET_MATRIX_HPP)
#error "Unnecessary dependency"
#endif

#include "fps/fps.hpp"
#include "data_structure/wavelet_matrix/wavelet_matrix.hpp"

int main() { fps_detail::factorials(nullptr, nullptr, 0); }
