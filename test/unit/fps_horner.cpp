#include "fps/fps.hpp"
#include "components/fps_horner.hpp"
#include <random>

int main() {
	std::mt19937 random(20261019);
	for (int n : {0, 1, 2, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129, 1023, 1024, 1025, 32769}) {
		fps a(n);
		for (auto& x : a) {
			x = random();
			if (x.v < mint::mod()) x.v += mint::mod();
		}
		const auto copy = a;
		for (mint x : {mint(0), mint(1), mint(-1), mint(2), mint(-2), mint(random()), mint(random())}) {
			mint expected = 0;
			for (auto it = a.rbegin(); it != a.rend(); ++it) expected = expected * x + *it;
			assert(fps_detail::horner(a.data(), a.size(), x) == expected);
			assert(a.eval(x) == expected);
			assert(fps_detail::horner_detail::scalar<8>(a.data(), a.size(), x) == expected);
			assert(a == copy);
		}
	}
}
