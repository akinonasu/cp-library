#include "fps/fps.hpp"
#include <cstdio>
#include <sys/resource.h>
int main() {
	constexpr int degree = 1 << 24;
	fps f(65, fps_mint(1));
	const auto logarithm = f.log(degree);
	if (logarithm.size() != unsigned(degree) || logarithm[0] != fps_mint(0)) return 1;
	unsigned long long checksum = 0;
	for (int i = 1; i < degree; ++i) {
		const auto expected = fps_mint(i % 65 ? 1 : -64);
		if (logarithm[i] * fps_mint(i) != expected) {
			std::fprintf(stderr, "coefficient %d failed\n", i);
			return 1;
		}
		checksum = checksum * 0x9e3779b97f4a7c15ULL + logarithm[i].val();
	}
	rusage usage{};
	getrusage(RUSAGE_SELF, &usage);
	std::printf("{\"precision\":%d,\"checked_coefficients\":%d,\"checksum\":%llu,\"max_rss_kib\":%ld}\n", degree, degree, checksum, usage.ru_maxrss);
}
