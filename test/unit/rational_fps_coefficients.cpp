#include "fps/rational_fps_coefficients.hpp"
#include "fps/rational_fps_coefficients_light.hpp"
#include "test/fixtures/rational_fps_reference.hpp"
#include <cassert>
#include <limits>
#include <random>

void check(const fps &p, const fps &q, long long l, int count) {
	auto expected = rational_fps_test::range(p, q, l, count);
	auto a = rational_fps_coefficients(p, q, l, l + count);
	auto b = light_fps_algorithms::rational_fps_coefficients(light_fps(p), light_fps(q), l, l + count);
	assert(a == fps(expected));
	assert(b == light_fps(expected));
	if (count > 1) {
		int split = count / 2;
		auto left = rational_fps_coefficients(p, q, l, l + split);
		auto right = rational_fps_coefficients(p, q, l + split, l + count);
		left.insert(left.end(), right.begin(), right.end());
		assert(a == left);
	}
}

int main() {
	std::mt19937 random(202610);
	for (int n : {0, 1, 2, 7, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129})
		for (int m : {1, 2, 3, 16, 17, 32, 33, 65}) {
			fps p(n), q(m);
			for (auto &x : p) x = random();
			for (auto &x : q) x = random();
			q[0] = 7;
			for (int l : {0, 1, 31, 128, 511}) {
				auto prefix = rational_fps_test::prefix(p, q, l + 67);
				auto a = rational_fps_coefficients(p, q, l, l + 67);
				auto b = light_fps_algorithms::rational_fps_coefficients(light_fps(p), light_fps(q), l, l + 67);
				fps expected(prefix.begin() + l, prefix.end());
				assert(a == expected && b == light_fps(expected));
			}
			for (int count : {0, 1, 17, 128}) check(p, q, 1000000000000000000LL, count);
			p.resize(n + 11), q.resize(m + 11);
			check(p, q, 1023, 35);
		}
	for (int d : {1, 2, 16, 17, 32, 33, 64, 65, 129}) {
		fps q(d + 1);
		q[0] = 3, q.back() = 5;
		check(fps{1, 2, 3}, q, 1000000000000000000LL, 257);
		check(fps{1}, q, std::numeric_limits<long long>::max() - 257, 257);
	}
	check({}, fps{1, 0, 1}, 1000000000000000000LL, 17);
	check(fps{1, 0, 3}, fps{2}, 0, 8);
	check(fps{1, 0, 3}, fps{2}, 2, 8);
	check(fps{1, 0, 3}, fps{2}, 1000000000000000000LL, 8);
	for (int count : {1, 2, 31, 32, 33, 1000}) check(fps{1}, fps{1, -1}, 1000000000000000000LL, count);
	using E = rational_fps_test::StoredMint;
	FormalPowerSeries<E> p{E(0), E(1)}, q{E(1), E(-1), E(-1)};
	auto a = rational_fps_coefficients(p, q, 1000, 1033);
	auto b = light_fps_algorithms::rational_fps_coefficients(LightFormalPowerSeries<E>(p), LightFormalPowerSeries<E>(q), 1000, 1033);
	auto expected = rational_fps_test::range(fps{0, 1}, fps{1, -1, -1}, 1000, 33);
	for (int i = 0; i < 33; ++i) assert(a[i].val() == expected[i].val() && b[i].val() == expected[i].val());
}
