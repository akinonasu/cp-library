#include "math/matrix_light.hpp"
#include "math/mint.hpp"
#include <type_traits>

static_assert(std::is_same_v<Matrix<mint>, LightMatrix<mint>>);
static_assert(std::is_same_v<Matrix<double>, LightMatrix<double>>);

int main() {
	using M = Matrix<mint>;
	M a{ {2, 1}, {1, 1} };
	assert(a * a == M({{5, 3}, {3, 2}}));
	assert(a.pow(3) == M({{13, 8}, {8, 5}}));
	assert(a.det() == mint(1) && a.rank() == 2);
	assert(a.characteristic_polynomial() == std::vector<mint>({1, -3, 1}));
	auto inverse = a.inverse();
	assert(inverse && a * *inverse == M::identity(2));
	auto solution = a.solve(std::vector<mint>{3, 2});
	assert(solution && solution->particular == std::vector<mint>({1, 1}) && solution->basis.empty());
	M b{ {1, 2, 3}, {2, 4, 6} };
	assert(b.rank() == 1 && b.kernel().size() == 2);
	for (const auto &v : b.kernel()) assert(b * v == std::vector<mint>(2));
	Matrix<int> integers{ {2, 3}, {4, 1} };
	assert(integers.det() == -10);
	assert(integers * integers == Matrix<int>({{16, 9}, {12, 13}}));
	M empty;
	assert(empty.rows() == 0 && empty.cols() == 0 && empty.det() == mint(1));
	assert(empty.pow(0) == M::identity(0));
}
