#ifdef LIGHT_FPS
#include "fps/fps_light.hpp"
using Series = light_fps;
#else
#include "fps/fps.hpp"
using Series = fps;
#endif
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	const std::string operation = argv[1];
	const int n = std::atoi(argv[2]);
	if (n < 2) return 1;
	std::mt19937 random(731);
	Series a(n), b(n), result, remainder;
	for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random();
	if (operation == "inv" || operation == "log") a[0] = 1;
	if (operation == "exp") a[0] = 0;
	if (operation == "sqrt") a[0] = mint(17) * 17;
	if (operation == "div_series") b[0] = 1;
	if (operation == "divmod") {
		b.resize(n / 2);
		b.back() = 1;
	}
	auto start = std::chrono::steady_clock::now();
	if (operation == "multiply") result = a * b;
	else if (operation == "square") result = a * a;
	else if (operation == "inv") result = a.inv();
	else if (operation == "log") result = a.log();
	else if (operation == "exp") result = a.exp();
	else if (operation == "sqrt") result = a.sqrt();
	else if (operation == "pow") result = a.pow(123456789);
	else if (operation == "div_series") result = a.div_series(b);
	else if (operation == "divmod") {
		auto qr = a.divmod(b);
		result = std::move(qr.first);
		remainder = std::move(qr.second);
	} else return 1;
	auto end = std::chrono::steady_clock::now();
	std::uint64_t checksum = result.size();
	for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	checksum = (checksum ^ remainder.size()) * 0x9e3779b97f4a7c15ULL;
	for (auto x : remainder) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	const double ms = std::chrono::duration<double, std::milli>(end - start).count();
	std::printf("%.9f %zu %zu %llu\n", ms, result.size(), remainder.size(), static_cast<unsigned long long>(checksum));
}
