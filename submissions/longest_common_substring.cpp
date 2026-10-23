#line 1 "test/longest_common_substring.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#line 1 "string/longest_common_substring.hpp"



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


#line 1 "components/suffix_array.hpp"



#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <string_view>
#include <vector>

namespace suffix_array_detail {
template<class Sequence>
std::vector<int> induced_sort(const Sequence &s, int upper) {
	int n = static_cast<int>(s.size());
	std::vector<int> sa(n);
	if (n < 16) {
		std::iota(sa.begin(), sa.end(), 0);
		std::sort(sa.begin(), sa.end(), [&](int a, int b) {
			while (a < n && b < n && s[a] == s[b]) ++a, ++b;
			return a != n ? b != n && s[a] < s[b] : b != n;
		});
		return sa;
	}
	std::vector<unsigned char> small(n);
	for (int i = n - 2; i >= 0; --i) small[i] = s[i] == s[i + 1] ? small[i + 1] : s[i] < s[i + 1];
	std::vector<int> left(upper + 2), right(upper + 2), cursor(upper + 2);
	for (int i = 0; i < n; ++i) {
		if (small[i]) ++left[s[i] + 1];
		else ++right[s[i]];
	}
	for (int c = 0; c <= upper; ++c) {
		right[c] += left[c];
		left[c + 1] += right[c];
	}
	auto induce = [&](const std::vector<int> &seeds) {
		std::fill(sa.begin(), sa.end(), -1);
		cursor = right;
		for (int v : seeds) sa[cursor[s[v]]++] = v;
		cursor = left;
		sa[cursor[s[n - 1]]++] = n - 1;
		for (int i = 0; i < n; ++i) {
			int v = sa[i] - 1;
			if (v >= 0 && !small[v]) sa[cursor[s[v]]++] = v;
		}
		cursor = left;
		for (int i = n - 1; i >= 0; --i) {
			int v = sa[i] - 1;
			if (v >= 0 && small[v]) sa[--cursor[s[v] + 1]] = v;
		}
	};
	std::vector<int> number(n, -1), seeds;
	for (int i = 1; i < n; ++i) if (!small[i - 1] && small[i]) {
		number[i] = static_cast<int>(seeds.size());
		seeds.push_back(i);
	}
	induce(seeds);
	int m = static_cast<int>(seeds.size());
	if (!m) return sa;
	std::vector<int> sorted;
	sorted.reserve(m);
	for (int v : sa) if (number[v] >= 0) sorted.push_back(v);
	std::vector<int> reduced(m);
	int alphabet = 0;
	for (int i = 1; i < m; ++i) {
		int a = sorted[i - 1], b = sorted[i];
		int end_a = number[a] + 1 < m ? seeds[number[a] + 1] : n;
		int end_b = number[b] + 1 < m ? seeds[number[b] + 1] : n;
		bool equal = end_a - a == end_b - b && end_a < n && end_b < n;
		if (equal) for (int k = 0; k <= end_a - a; ++k) if (s[a + k] != s[b + k]) { equal = false; break; }
		if (!equal) ++alphabet;
		reduced[number[b]] = alphabet;
	}
	std::vector<int> order(m);
	if (alphabet + 1 == m) for (int i = 0; i < m; ++i) order[reduced[i]] = i;
	else order = induced_sort(reduced, alphabet);
	for (int i = 0; i < m; ++i) sorted[i] = seeds[order[i]];
	induce(sorted);
	return sa;
}

inline std::vector<int> bytes(std::string_view s) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
	struct Bytes {
		std::string_view value;
		std::size_t size() const { return value.size(); }
		unsigned char operator[](int i) const { return static_cast<unsigned char>(value[i]); }
	};
	return induced_sort(Bytes{s}, 255);
}
}


#line 11 "string/longest_common_substring.hpp"

struct LongestCommonSubstringResult {
	int s_pos = 0, t_pos = 0, length = 0;
};

inline LongestCommonSubstringResult longest_common_substring(std::string_view s, std::string_view t) {
	[[maybe_unused]] constexpr std::size_t maximum = std::numeric_limits<int>::max() - 1;
	assert(s.size() <= maximum && t.size() <= maximum - s.size());
	if (s.empty() || t.empty()) return {};
	int n = int(s.size()), m = int(t.size()), bound = std::min(n, m);
	LongestCommonSubstringResult best{0, 0, sequence_compare_detail::prefix(s.data(), t.data(), bound)};
	if (best.length == bound) return best;
	struct Joined {
		std::string_view s, t;
		int split;
		std::size_t size() const { return s.size() + t.size() + 1; }
		int operator[](int p) const {
			if (p < split) return static_cast<unsigned char>(s[p]) + 1;
			return p == split ? 0 : static_cast<unsigned char>(t[p - split - 1]) + 1;
		}
	};
	auto sa = suffix_array_detail::induced_sort(Joined{s, t, n}, 256);
	int size = int(sa.size());
	std::vector<int> rank(size);
	for (int i = 0; i < size; ++i) rank[sa[i]] = i;
	for (int p = 0, h = 0; p < size; ++p) {
		if (h) --h;
		if (!rank[p]) { h = 0; continue; }
		int q = sa[rank[p] - 1];
		if (q == n) { h = 0; continue; }
		bool first = p < n, other = q < n;
		int a = first ? p : p - n - 1, b = other ? q : q - n - 1;
		auto left = first ? s : t, right = other ? s : t;
		int limit = std::min(int(left.size()) - a, int(right.size()) - b);
		h += sequence_compare_detail::prefix(left.data() + a + h, right.data() + b + h, limit - h);
		if (first != other && h > best.length) {
			best = first ? LongestCommonSubstringResult{a, b, h} : LongestCommonSubstringResult{b, a, h};
			if (h == bound) return best;
		}
	}
	return best;
}


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
#include <memory>
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


#line 4 "test/longest_common_substring.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	std::string s, t;
	input.read(s);
	input.read(t);
	auto [a, b, length] = longest_common_substring(s, t);
	output.write(a);
	output.write(' ');
	output.write(a + length);
	output.write(' ');
	output.write(b);
	output.write(' ');
	output.println(b + length);
}
