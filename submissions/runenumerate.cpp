#line 1 "test/run_enumerate.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/runenumerate"
#line 1 "string/run_enumerate.hpp"



#line 1 "components/sequence_match.hpp"



#line 1 "components/sequence_compare.hpp"



#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace sequence_compare_detail {
#if defined(__x86_64__)
__attribute__((target("avx2"))) inline std::size_t forward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countr_zero(difference);
	}
	while (i < size && a[i] == b[i]) ++i;
	return i;
}

__attribute__((target("avx2"))) inline std::size_t backward_avx2(const unsigned char *a, const unsigned char *b, std::size_t size) {
	std::size_t i = 0;
	for (; size - i >= 32; i += 32) {
		auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a - i - 32));
		auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b - i - 32));
		auto difference = ~std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(x, y)));
		if (difference) return i + std::countl_zero(difference);
	}
	while (i < size && a[-std::ptrdiff_t(i) - 1] == b[-std::ptrdiff_t(i) - 1]) ++i;
	return i;
}
#endif

template<bool Vectorize = true, class T>
inline int prefix(const T *first, const T *second, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first);
		const auto *b = reinterpret_cast<const unsigned char *>(second);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a + offset, 8);
			std::memcpy(&y, b + offset, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countr_zero(difference) : std::countl_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + forward_avx2(a + offset, b + offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first[matched] == second[matched]) ++matched;
	return matched;
}

template<bool Vectorize = true, class T>
inline int suffix(const T *first_end, const T *second_end, int limit) {
	if (!limit) return 0;
	int matched = 0;
	if constexpr (std::is_integral_v<T> && std::has_unique_object_representations_v<T>) {
		const auto *a = reinterpret_cast<const unsigned char *>(first_end);
		const auto *b = reinterpret_cast<const unsigned char *>(second_end);
		std::size_t offset = 0, size = std::size_t(limit) * sizeof(T);
		while (size - offset >= 8) {
			std::uint64_t x, y;
			std::memcpy(&x, a - offset - 8, 8);
			std::memcpy(&y, b - offset - 8, 8);
			auto difference = x ^ y;
			if (difference) {
				int bits = std::endian::native == std::endian::little ? std::countl_zero(difference) : std::countr_zero(difference);
				return static_cast<int>((offset + bits / 8) / sizeof(T));
			}
			offset += 8;
#if defined(__x86_64__)
			if constexpr (Vectorize) if (size - offset >= 64 && __builtin_cpu_supports("avx2"))
				return static_cast<int>((offset + backward_avx2(a - offset, b - offset, size - offset)) / sizeof(T));
#endif
		}
		matched = static_cast<int>(offset / sizeof(T));
	}
	while (matched < limit && first_end[-matched - 1] == second_end[-matched - 1]) ++matched;
	return matched;
}
}


#line 1 "components/z_algorithm.hpp"



namespace sequence_match_detail {
template <class F>
void z_table(int n, int *z, F match) {
	if (!n) return;
	z[0] = n;
	for (int i = 1, base = 0, end = 0; i < n; ++i) {
		int length = 0;
		if (i < end) {
			length = z[i - base];
			if (length < end - i) {
				z[i] = length;
				continue;
			}
			length = end - i;
		}
		length += match(length, i + length, n - i - length);
		z[i] = length;
		if (i + length > end) base = i, end = i + length;
	}
}

template <class F, class Emit>
void prefix_matches(int n, int m, const int *z, F match, Emit emit) {
	for (int i = 0, base = 0, end = 0; i < n; ++i) {
		int length = 0;
		if (i < end) {
			length = z[i - base];
			if (length < end - i) {
				emit(i, length);
				continue;
			}
			length = end - i;
		}
		length += match(length, i + length, m - length);
		if (i + length > end) base = i, end = i + length;
		emit(i, length);
	}
}
}


#line 6 "components/sequence_match.hpp"
#include <algorithm>
#include <array>
#include <span>
#include <utility>
#include <vector>

namespace sequence_match_detail {
#if defined(__x86_64__)
using sequence_compare_detail::forward_avx2;
using sequence_compare_detail::backward_avx2;
#endif

template <class T, bool Vectorize = true>
class Matcher {
	const T *data;
	std::vector<std::array<int, 2>> runs;
public:
	explicit Matcher(std::span<const T> values) : data(values.data()) {}
	void index_runs(int n) {
		runs.resize(n);
		for (int first = 0, last; first < n; first = last) {
			last = first + 1;
			while (last < n && data[first] == data[last]) ++last;
			for (int i = first; i < last; ++i) runs[i] = {first, last};
		}
	}
	std::pair<int, int> forward_counted(int i, int j, int limit) const {
		if (runs.empty() || !limit || data[i] != data[j]) {
			int length = forward(i, j, limit);
			return {length, length};
		}
		int a = runs[i][1] - i, b = runs[j][1] - j, skip = std::min({limit, a, b});
		if (a != b || skip == limit) return {skip, 1};
		int length = forward(i + skip, j + skip, limit - skip);
		return {skip + length, length + 1};
	}
	std::pair<int, int> backward_counted(int i, int j, int limit) const {
		if (runs.empty() || !limit || data[i - 1] != data[j - 1]) {
			int length = backward(i, j, limit);
			return {length, length};
		}
		int a = i - runs[i - 1][0], b = j - runs[j - 1][0], skip = std::min({limit, a, b});
		if (a != b || skip == limit) return {skip, 1};
		int length = backward(i - skip, j - skip, limit - skip);
		return {skip + length, length + 1};
	}
	int forward(int i, int j, int limit) const {
		if (!limit) return 0;
		return sequence_compare_detail::prefix<Vectorize>(data + i, data + j, limit);
	}
	int backward(int i, int j, int limit) const {
		if (!limit) return 0;
		return sequence_compare_detail::suffix<Vectorize>(data + i, data + j, limit);
	}
};

}


#line 1 "components/default_init_allocator.hpp"



#include <memory>
#include <new>

template <class T>
struct DefaultInitAllocator : std::allocator<T> {
	template <class U>
	struct rebind {
		using other = DefaultInitAllocator<U>;
	};
	DefaultInitAllocator() = default;
	template <class U>
	DefaultInitAllocator(const DefaultInitAllocator<U> &) noexcept {}
	template <class U>
	void construct(U *p) { ::new (static_cast<void *>(p)) U; }
};


#line 9 "string/run_enumerate.hpp"
#include <cassert>
#include <compare>
#line 12 "string/run_enumerate.hpp"
#include <limits>
#line 14 "string/run_enumerate.hpp"
#include <string_view>
#include <tuple>
#line 17 "string/run_enumerate.hpp"

struct Run {
	int period, l, r;
	friend auto operator<=>(const Run &, const Run &) = default;
};

namespace run_enumerate_detail {
template <class Key>
void radix_sort(std::vector<Run> &runs, std::vector<Run> &scratch, int bits, Key key) {
	scratch.resize(runs.size());
	for (int shift = 0; shift < bits; shift += 11) {
		std::array<std::size_t, 2048> count{};
		for (auto run : runs) ++count[(key(run) >> shift) & 2047];
		std::size_t sum = 0;
		for (auto &position : count) {
			auto next = sum + position;
			position = sum;
			sum = next;
		}
		for (auto run : runs) scratch[count[(key(run) >> shift) & 2047]++] = run;
		runs.swap(scratch);
	}
}

inline void sort_runs(std::vector<Run> &runs, int n) {
	if (runs.size() < 512) std::sort(runs.begin(), runs.end());
	else {
		std::vector<Run> scratch;
		int bits = std::bit_width(unsigned(n));
		radix_sort(runs, scratch, 2 * bits, [bits](Run run) { return (std::uint64_t(run.l) << bits) | unsigned(run.r); });
		radix_sort(runs, scratch, bits, [](Run run) { return unsigned(run.period); });
	}
	runs.erase(std::unique(runs.begin(), runs.end()), runs.end());
}

template <bool Indexed, class T>
bool lyndon_impl(std::span<const T> s, std::vector<Run> &runs, std::uint64_t budget, sequence_match_detail::Matcher<T, false> &match) {
	int n = static_cast<int>(s.size());
	auto extend = [&](bool backward, int i, int j, int limit) {
		if constexpr (Indexed) {
			auto [length, work] = backward ? match.backward_counted(i, j, limit) : match.forward_counted(i, j, limit);
			if (std::uint64_t(work) > budget) return -1;
			budget -= work;
			return length;
		} else {
			int allowed = static_cast<int>(std::min(std::uint64_t(limit), budget));
			int length = backward ? match.backward(i, j, allowed) : match.forward(i, j, allowed);
			budget -= length;
			return length == allowed && allowed < limit ? -1 : length;
		}
	};
	std::vector<int, DefaultInitAllocator<int>> stack(n + 1);
	for (int opposite = 0; opposite < 2; ++opposite) {
		int top = 0;
		stack[0] = n;
		for (int i = n - 1; i >= 0; --i) {
			int common = 0;
			while (top) {
				int j = stack[top], left = j - i, right = stack[top - 1] - j;
				int limit = std::min(left, right);
				common = s[i] == s[j] ? extend(false, i, j, limit) : 0;
				if (common < 0) return false;
				if ((common == limit && left < right) ||
					(common < limit && (bool(s[i + common] < s[j + common]) != bool(opposite)))) {
					--top;
					common = 0;
				} else break;
			}
			int j = stack[top];
			stack[++top] = i;
			if (j == n) continue;
			if constexpr (!Indexed) if (s[i] != s[j]) continue;
			int period = j - i;
			int back = i && s[i - 1] == s[j - 1] ? extend(true, i, j, std::min(i, period)) : 0;
			if (back < 0) return false;
			if (back == period) continue;
			int ahead = common;
			if (j + common < n && s[i + common] == s[j + common]) {
				int extra = extend(false, i + common, j + common, n - j - common);
				if (extra < 0) return false;
				ahead += extra;
			}
			if (back + ahead >= period) runs.push_back({period, i - back, j + ahead});
		}
	}
	return true;
}

template <class T>
bool lyndon(std::span<const T> s, std::vector<Run> &runs, std::uint64_t budget) {
	int n = static_cast<int>(s.size());
	sequence_match_detail::Matcher<T, false> match(s);
	if (s[0] == s[1]) {
		sequence_match_detail::Matcher<T> whole(s);
		int length = whole.forward(0, 1, n - 1);
		if (std::uint64_t(length) > budget) return false;
		budget -= length;
		if (length == n - 1) {
			runs.push_back({1, 0, n});
			return true;
		}
	}
	if (n >= 128) {
		int equal = 0;
		for (int k = 1; k <= 64; ++k) {
			int i = static_cast<int>(std::size_t(n - 1) * k / 64);
			equal += s[i - 1] == s[i];
		}
		if (equal >= 56) {
			match.index_runs(n);
			return lyndon_impl<true>(s, runs, budget, match);
		}
	}
	return lyndon_impl<false>(s, runs, budget, match);
}

template <class T>
void divide(std::span<const T> s, std::vector<Run> &runs) {
	int n = static_cast<int>(s.size());
	if (n < 2) return;
	sequence_match_detail::Matcher<T, false> match(s);
	std::vector<int, DefaultInitAllocator<int>> z(n);
	int *left_z = z.data(), *right_z = z.data() + n / 2;
	std::vector<Run> candidates, scratch;
	auto add = [&](int period, int l, int r, int middle) {
		if (l >= middle || r <= middle || (r - l) / 2 < period) return;
		if (l && s[l - 1] == s[l + period - 1]) return;
		if (r < n && s[r] == s[r - period]) return;
		candidates.push_back({period, l, r});
	};
	auto recurse = [&](auto &&self, int first, int last) -> void {
		if (last - first <= 16) {
			for (int period = 1; period <= (last - first) / 2; ++period) {
				int start = first;
				for (int i = first; i < last - period; ++i) {
					if (s[i] != s[i + period]) {
						if (i - start >= period) add(period, start, i + period, start + 1);
						start = i + 1;
					}
				}
				if (last - period - start >= period) add(period, start, last, start + 1);
			}
		} else {
			int middle = first + (last - first) / 2, left = middle - first, right = last - middle;
			self(self, first, middle);
			self(self, middle, last);
			sequence_match_detail::z_table(left, left_z, [&](int a, int b, int count) { return match.backward(middle - a, middle - b, count); });
			sequence_match_detail::z_table(right, right_z, [&](int a, int b, int count) { return match.forward(middle + a, middle + b, count); });
			sequence_match_detail::prefix_matches(left, right, right_z,
				[&](int a, int b, int count) { return match.forward(middle + a, first + b, count); },
				[&](int i, int length) {
					int period = left - i, back = period < left ? left_z[period] : 0;
					add(period, middle - period - back, middle + length, middle);
				});
			sequence_match_detail::prefix_matches(right, left, left_z,
				[&](int a, int b, int count) { return match.backward(middle - a, last - b, count); },
				[&](int i, int length) {
					int period = right - i, ahead = period < right ? right_z[period] : 0;
					add(period, middle - length, middle + period + ahead, middle);
				});
		}
		if (candidates.size() < 512) {
			std::sort(candidates.begin(), candidates.end(), [](Run a, Run b) {
				return std::tie(a.l, a.r, a.period) < std::tie(b.l, b.r, b.period);
			});
		} else {
			int bits = std::bit_width(unsigned(n));
			radix_sort(candidates, scratch, 2 * bits, [bits](Run run) { return (std::uint64_t(run.l) << bits) | unsigned(run.r); });
		}
		for (std::size_t i = 0; i < candidates.size();) {
			Run run = candidates[i++];
			while (i < candidates.size() && candidates[i].l == run.l && candidates[i].r == run.r)
				run.period = std::min(run.period, candidates[i++].period);
			runs.push_back(run);
		}
		candidates.clear();
	};
	recurse(recurse, 0, n);
}
}

template <class T>
std::vector<Run> run_enumerate(std::span<const T> s) {
	assert(s.size() < std::size_t(std::numeric_limits<int>::max()));
	std::vector<Run> runs;
	int n = static_cast<int>(s.size());
	if (n < 2) return runs;
	if (!run_enumerate_detail::lyndon(s, runs, std::uint64_t(n) * 64)) {
		runs.clear();
		run_enumerate_detail::divide(s, runs);
	}
	run_enumerate_detail::sort_runs(runs, n);
	return runs;
}

inline std::vector<Run> run_enumerate(std::string_view s) { return run_enumerate(std::span<const char>(s.data(), s.size())); }

template <class T, class Allocator>
std::vector<Run> run_enumerate(const std::vector<T, Allocator> &s) { return run_enumerate(std::span<const T>(s.data(), s.size())); }


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 14 "components/fast_io.hpp"
#include <stdexcept>
#include <string>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/run_enumerate.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string s;
	in.read(s);
	auto runs = run_enumerate(s);
	out.println(runs.size());
	for (auto [period, l, r] : runs) {
		out.write(period);
		out.write(' ');
		out.write(l);
		out.write(' ');
		out.println(r);
	}
}
