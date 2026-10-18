#include "fps/fps.hpp"
#include <random>

int main() {
	std::mt19937 random(928374);
	for (int n : {129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025, 4097}) {
		for (int m : {2, 31, 64, 65, 127, 128, 129, n / 4, n / 2, n}) {
			if (m > n) continue;
			fps a(n), b(m);
			for (auto& x : a) x = random();
			for (auto& x : b) x = random();
			b[0] = b.back() = 3;
			for (auto* f : {&a, &b}) for (auto& x : *f) if (x.v < mint::mod()) x.v += mint::mod();
			auto original_a = a, original_b = b;
			auto [q, r] = a.divmod(b);
			auto product = q * b + r;
			fps_detail::Operations<mint>::trim(product);
			assert(product == a && r.size() < b.size());
			for (int degree : {129, 257, 513, n}) {
				auto quotient = a.div_series(b, degree);
				auto restored = (quotient * b).pre(degree), expected = a.pre(degree);
				restored.resize(degree), expected.resize(degree);
				assert(restored == expected && quotient.size() == std::size_t(degree));
			}
			a.push_back(0), b.push_back(0);
			auto trimmed = a.divmod(b);
			assert(trimmed.first == q && trimmed.second == r);
			a.pop_back(), b.pop_back();
			assert(a == original_a && b == original_b);
		}
	}
}
