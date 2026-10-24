#ifndef MATRIX_BLOCK_INVERSE_HPP
#define MATRIX_BLOCK_INVERSE_HPP

#include "matrix_arithmetic.hpp"
#include "matrix_elimination_modular.hpp"
#include "matrix_core.hpp"

namespace matrix_detail {
template <class T>
struct GaussianMatrixArithmetic {
	static constexpr bool modular = small_modint<T>;
	template <class Ops>
	static auto eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double) {
		return ModularElimination<Ops>::run(a, h, stride, columns, reduced);
	}
	template <class F>
	static auto field(std::vector<T> &a, F &&f) { return matrix_detail::field(a, std::forward<F>(f)); }
};

template <class T>
std::optional<std::pair<T, std::vector<T>>> block_inverse(const std::vector<T> &a, int n, bool adjugate) {
	if (n < 112) return std::nullopt;
	int h = n / 2, k = n - h;
	for (int i = 0; i < h; ++i) {
		bool row = false, column = false;
		for (int j = 0; j < h; ++j) {
			row |= a[std::size_t(i) * n + j] != T(0);
			column |= a[std::size_t(j) * n + i] != T(0);
		}
		if (!row || !column) return std::nullopt;
	}
	auto extract = [&](int r, int c, int rows, int cols) {
		std::vector<T> block(std::size_t(rows) * cols);
		for (int i = 0; i < rows; ++i) std::copy_n(a.data() + std::size_t(r + i) * n + c, cols, block.data() + std::size_t(i) * cols);
		return block;
	};
	auto solve = [](const std::vector<T> &block, int size, bool adjugate) {
		if (auto fast = block_inverse(block, size, adjugate)) return std::move(*fast);
		Matrix<T, GaussianMatrixArithmetic<T>> matrix(size);
		std::copy(block.begin(), block.end(), matrix.data());
		if (adjugate) {
			auto result = matrix.adjugate();
			return std::pair{matrix.det(), std::vector<T>(result.data(), result.data() + block.size())};
		}
		auto result = matrix.inverse();
		if (!result) return std::pair{T(0), std::vector<T>{}};
		return std::pair{matrix.det(), std::vector<T>(result->data(), result->data() + block.size())};
	};
	auto [da, ai] = solve(extract(0, 0, h, h), h, false);
	if (ai.empty()) return std::nullopt;
	auto b = extract(0, h, h, k), c = extract(h, 0, k, h), d = extract(h, h, k, k);
	auto v = multiply(c, ai, k, h, h), vb = multiply(v, b, k, h, k);
	for (std::size_t i = 0; i < d.size(); ++i) d[i] -= vb[i];
	auto [ds, si] = solve(d, k, adjugate);
	if (si.empty()) return std::pair{T(0), std::vector<T>{}};
	auto u = multiply(ai, b, h, h, k);
	auto r = multiply(u, si, h, k, k), t = multiply(si, v, k, k, h);
	auto rv = multiply(r, v, h, k, h);
	for (std::size_t i = 0; i < ai.size(); ++i) ai[i] = (adjugate ? ai[i] * ds : ai[i]) + rv[i];
	std::vector<T> result(a.size());
	for (int i = 0; i < h; ++i) {
		std::copy_n(ai.data() + std::size_t(i) * h, h, result.data() + std::size_t(i) * n);
		for (int j = 0; j < k; ++j) result[std::size_t(i) * n + h + j] = -r[std::size_t(i) * k + j];
	}
	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < h; ++j) result[std::size_t(h + i) * n + j] = -t[std::size_t(i) * h + j];
		std::copy_n(si.data() + std::size_t(i) * k, k, result.data() + std::size_t(h + i) * n + h);
	}
	if (adjugate) for (auto &x : result) x *= da;
	return std::pair{da * ds, std::move(result)};
}

}

#endif
