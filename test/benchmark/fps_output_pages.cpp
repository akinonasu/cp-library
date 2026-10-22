#include "components/fps_output.hpp"
#include "components/fps_workspace.hpp"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

using Clock = std::chrono::steady_clock;
struct Advice {
	std::size_t bytes = 0;
	int result = 0, error = 0, calls = 0;
};
static Advice advise(void *data, std::size_t bytes, std::size_t alignment, int mode) {
	const auto first = reinterpret_cast<std::uintptr_t>(data), end = first + bytes;
	const auto lo = (first + alignment - 1) & ~(alignment - 1), hi = end & ~(alignment - 1);
	if (lo >= hi) return {};
	Advice result;
	result.bytes = hi - lo;
	result.calls = 1;
	result.result = madvise(reinterpret_cast<void *>(lo), hi - lo, mode);
	result.error = result.result ? errno : 0;
	return result;
}
static double milliseconds(Clock::time_point first, Clock::time_point last) {
	return std::chrono::duration<double, std::milli>(last - first).count();
}
int main(int argc, char **argv) {
	if (argc != 4) return 1;
	const int mode = std::atoi(argv[1]), encoded = std::atoi(argv[3]);
	const std::size_t n = std::strtoull(argv[2], nullptr, 10);
	auto source = fps_detail::allocate_fps_words(n);
	for (std::size_t i = 0; i < n; ++i) source[i] = std::uint64_t(i + 17) * 236887691 % mint::mod();
	Advice huge, populate;
	Clock::time_point t1, t2, t3, t4;
	struct rusage before{}, after{};
	getrusage(RUSAGE_SELF, &before);
	const auto start = Clock::now();
	std::uintptr_t data = 0;
	std::size_t capacity = 0;
	unsigned long long checksum = 0;
	{
		std::vector<mint> out;
		out.reserve(n);
		data = reinterpret_cast<std::uintptr_t>(out.data());
		capacity = out.capacity();
		t1 = Clock::now();
		if (mode & 2) huge = advise(out.data(), n * sizeof(mint), 2 * 1024 * 1024, MADV_HUGEPAGE);
#ifdef MADV_POPULATE_WRITE
		if (mode & 1) populate = advise(out.data(), n * sizeof(mint), 4096, MADV_POPULATE_WRITE);
#endif
		t2 = Clock::now();
		if (encoded) fps_detail::append_fps_words<true>(out, source.get(), n);
		else fps_detail::append_fps_words<false>(out, source.get(), n);
		t3 = Clock::now();
		if (out.size() != n) return 2;
		for (std::size_t i = 0; i < n; ++i) {
			const auto expected = encoded ? mint::word(source[i]) : mint::raw(source[i]);
			if (out[i] != expected) return 3;
			checksum = (checksum ^ out[i].val()) * 0x9e3779b97f4a7c15ULL;
		}
		t4 = Clock::now();
	}
	const auto end = Clock::now();
	getrusage(RUSAGE_SELF, &after);
	const double reserve = milliseconds(start, t1), advice = milliseconds(t1, t2), append = milliseconds(t2, t3), destroy = milliseconds(t4, end);
	std::printf("{\"total_ms\":%.9f,\"reserve_ms\":%.9f,\"advice_ms\":%.9f,\"append_ms\":%.9f,\"destroy_ms\":%.9f,\"minor_faults\":%ld,\"major_faults\":%ld,\"capacity\":%zu,\"data\":%llu,\"huge_bytes\":%zu,\"huge_calls\":%d,\"huge_result\":%d,\"huge_errno\":%d,\"populate_bytes\":%zu,\"populate_calls\":%d,\"populate_result\":%d,\"populate_errno\":%d,\"checksum\":%llu}\n",
		reserve + advice + append + destroy, reserve, advice, append, destroy, after.ru_minflt-before.ru_minflt, after.ru_majflt-before.ru_majflt, capacity, static_cast<unsigned long long>(data), huge.bytes, huge.calls, huge.result, huge.error, populate.bytes, populate.calls, populate.result, populate.error, checksum);
}
