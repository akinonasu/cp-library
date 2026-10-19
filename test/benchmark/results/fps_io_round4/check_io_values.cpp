#include "components/fps_io.hpp"
#include <cstdlib>
#include <random>
int main() {
	std::mt19937 random(20261020);
	alignas(32) unsigned input[8], output[8];
	for (unsigned pass = 0; pass < 250000; ++pass) {
		for (int j = 0; j < 8; ++j) input[j] = pass < 8 ? unsigned((unsigned long long)(2 * mint::mod() - 1) * (pass * 8 + j) / 63) : random() % (2 * mint::mod());
		const auto values = fps_detail::io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(input)));
		_mm256_store_si256(reinterpret_cast<__m256i *>(output), values);
		for (int j = 0; j < 8; ++j) if (output[j] != mint::word(input[j]).val()) std::abort();
	}
}
