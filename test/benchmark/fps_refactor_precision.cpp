#include "fps/fps.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include <cstdio>
#include <cstdlib>

void require(bool ok) {
	if (!ok) std::abort();
}

int main() {
	using M = fps_mint;
	constexpr int maximum_transform = 1 << 23;
	{
		constexpr int half = maximum_transform / 2;
		fps a(half, M(1)), b(half + 1, M(1));
		auto product = a * b;
		require(product.size() == maximum_transform);
		for (int i = 0; i < maximum_transform; ++i)
			require(product[i] == M(i < half ? i + 1 : maximum_transform - i));
	}
	{
		auto inverse = fps{1, -1}.inv(maximum_transform);
		require(inverse.size() == maximum_transform);
		for (auto coefficient : inverse) require(coefficient == M(1));
	}
	{
		constexpr int length = 1 << 24;
		auto logarithm = fps{1, -1}.log(length);
		require(logarithm.size() == length && logarithm[0] == M(0));
		for (int i = 1; i < length; ++i) require(logarithm[i] * M(i) == M(-1));
		auto exponential = fps{0, 1}.exp(length);
		require(exponential.size() == length && exponential[0] == M(1));
		for (int i = 1; i < length; ++i) require(exponential[i] * M(i) == exponential[i - 1]);
	}
	{
		auto square_root = fps{1, 2, 1}.sqrt(maximum_transform);
		require(square_root.size() == maximum_transform && square_root[0] == M(1) && square_root[1] == M(1));
		for (int i = 2; i < maximum_transform; ++i) require(square_root[i] == M(0));
	}
	{
		auto division = fps{1}.div_series(fps{1, -1}, maximum_transform);
		require(division.size() == maximum_transform);
		for (auto coefficient : division) require(coefficient == M(1));
	}
	{
		constexpr int length = 1 << 22;
		auto composed = compose(fps{2, 3, 4}, fps{0, 1}, length);
		require(composed.size() == length && composed[0] == M(2) && composed[1] == M(3) && composed[2] == M(4));
		for (int i = 3; i < length; ++i) require(composed[i] == M(0));
	}
	{
		constexpr int length = 1 << 21;
		auto reverse = compositional_inverse(fps{0, 3}, length);
		require(reverse.size() == length && reverse[0] == M(0) && reverse[1] * M(3) == M(1));
		for (int i = 2; i < length; ++i) require(reverse[i] == M(0));
	}
	{
		constexpr int weights_length = 1 << 21, output_length = 1 << 22;
		fps weights(weights_length);
		weights.back() = 1;
		auto projected = power_projection(fps{0, 1}, weights, output_length);
		require(projected.size() == output_length);
		for (int i = 0; i < output_length; ++i) require(projected[i] == M(i == weights_length - 1));
	}
	std::puts("maximum precision endpoints passed: NTT 2^23, log/exp 2^24, compose/projection 2^22, reversion 2^21");
}
