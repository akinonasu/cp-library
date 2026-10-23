#line 1 "test/matrix_det.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/matrix_det"
#line 1 "test/fixtures/matrix_common.hpp"



#line 1 "components/fast_io.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
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


#line 1 "math/linear_algebra/matrix.hpp"



#line 1 "components/matrix_arithmetic.hpp"



#line 1 "components/matrix_base_arithmetic.hpp"



#line 1 "components/field_base.hpp"



#include <cmath>
#line 6 "components/field_base.hpp"

namespace field_detail {
template <class T>
struct Field {
	using V = T;
	static T add(T a, T b) { return a + b; }
	static T mul(T a, T b) { return a * b; }
	static T neg(T a) { return -a; }
	static T inv(T a) { return T(1) / a; }
	static bool zero(T a, long double eps) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) <= eps;
		else { (void)eps; return a == T(0); }
	}
	static bool better(T a, T b) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) > std::abs(b);
		else { (void)a; (void)b; return false; }
	}
	static T dot(const T *a, const T *b, int n) {
		T result = 0;
		for (int i = 0; i < n; ++i) result += a[i] * b[i];
		return result;
	}
	static void scale(T *a, int n, T x) { for (int i = 0; i < n; ++i) a[i] *= x; }
	static void axpy(T *a, const T *b, int n, T x) { for (int i = 0; i < n; ++i) a[i] += x * b[i]; }
};

}


#line 9 "components/matrix_base_arithmetic.hpp"
#include <limits>
#line 11 "components/matrix_base_arithmetic.hpp"
#include <utility>
#include <vector>

namespace matrix_detail {
template <class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;

using field_detail::Field;

template <class Ops>
struct Elimination {
	std::vector<int> pivots;
	typename Ops::V determinant = 1;
};

template <class Ops, bool exact = false>
Elimination<Ops> eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
	using V = typename Ops::V;
	Elimination<Ops> result;
	result.pivots.reserve(std::min(h, columns));
	int rank = 0;
	for (int col = 0; col < columns && rank < h; ++col) {
		int p = -1;
		for (int i = rank; i < h; ++i) {
			V x = a[std::size_t(i) * stride + col];
			if (!Ops::zero(x, eps) && (p < 0 || Ops::better(x, a[std::size_t(p) * stride + col]))) p = i;
		}
		if (p < 0) continue;
		V *row = a.data() + std::size_t(rank) * stride;
		if (p != rank) {
			std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
			result.determinant = Ops::neg(result.determinant);
		}
		result.determinant = Ops::mul(result.determinant, row[col]);
		Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
		row[col] = V(1);
		for (int i = rank + 1; i < h; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps)) Ops::axpy(dst + col + 1, row + col + 1, stride - col - 1, Ops::neg(x));
			dst[col] = V(0);
		}
		result.pivots.push_back(col);
		++rank;
	}
	bool full = exact && rank == columns;
	if (reduced) for (int k = rank - 1; k >= 0; --k) {
		int col = result.pivots[k], start = full ? columns : col + 1;
		const V *row = a.data() + std::size_t(k) * stride;
		for (int i = 0; i < k; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps) && (!full || start < stride)) Ops::axpy(dst + start, row + start, stride - start, Ops::neg(x));
			dst[col] = V(0);
		}
	}
	return result;
}

template <class T>
[[gnu::noinline]] T integer_determinant(const std::vector<T> &a, int n) {
	static_assert(std::numeric_limits<T>::is_signed);
	using W = __int128_t;
	std::vector<W> work(a.begin(), a.end());
	W previous = 1, sign = 1;
	if (!n) return T(1);
	for (int k = 0; k + 1 < n; ++k) {
		int pivot = k;
		while (pivot < n && work[std::size_t(pivot) * n + k] == 0) ++pivot;
		if (pivot == n) return T(0);
		if (pivot != k) { for (int j = 0; j < n; ++j) std::swap(work[std::size_t(k) * n + j], work[std::size_t(pivot) * n + j]); sign = -sign; }
		W p = work[std::size_t(k) * n + k];
		for (int i = k + 1; i < n; ++i) {
			for (int j = k + 1; j < n; ++j) {
				W value = work[std::size_t(i) * n + j] * p - work[std::size_t(i) * n + k] * work[std::size_t(k) * n + j];
				assert(value % previous == 0);
				work[std::size_t(i) * n + j] = value / previous;
			}
			work[std::size_t(i) * n + k] = 0;
		}
		previous = p;
	}
	W result = sign * work.back();
	assert(result >= std::numeric_limits<T>::lowest() && result <= std::numeric_limits<T>::max());
	return T(result);
}

template <class T>
std::vector<T> basic_multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
		const T &x = a[std::size_t(i) * inner + k];
		for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
	}
	return c;
}
}


#line 1 "components/field_arithmetic.hpp"



#line 9 "components/field_arithmetic.hpp"
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace field_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<T>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 31))
		&& std::is_convertible_v<decltype(std::declval<T>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

template <class T>
inline constexpr bool small_modint = SmallModint<T>::value;

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(FIELD_DISABLE_AVX2) && !defined(MATRIX_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

template <class T>
struct ModularField {
	using V = std::uint32_t;
	static constexpr V mod = T::mod();
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V mul(V a, V b) { return std::uint64_t(a) * b % mod; }
	static V neg(V a) { return a ? mod - a : 0; }
	static V dot(const V *a, const V *b, int n) {
		constexpr int step = mod <= (V(1) << 30) ? 16 : 4;
		std::uint64_t sum = 0;
		for (int i = 0; i < n; i += step) {
			for (int j = i; j < std::min(n, i + step); ++j) sum += std::uint64_t(a[j]) * b[j];
			sum %= mod;
		}
		return sum;
	}
	static V inv(V a) { return (T(1) / T::raw(a)).val(); }
	static bool zero(V a, long double) { return !a; }
	static bool better(V, V) { return false; }
	static V product(V a, V x, V q) {
		V r = a * x - V((std::uint64_t(a) * q) >> 32) * mod;
		return r < mod ? r : r - mod;
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx2"))) static __m256i product8(__m256i a, __m256i x, __m256i q) {
		__m256i p = _mm256_set1_epi32(mod);
		__m256i even = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		__m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		__m256i high = _mm256_or_si256(even, _mm256_slli_epi64(_mm256_srli_epi64(odd, 32), 32));
		__m256i r = _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(high, p));
		return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
	}
	__attribute__((target("avx512f"))) static __m512i product16(__m512i a, __m512i x, __m512i q) {
		const auto p = _mm512_set1_epi32(mod);
		auto even = _mm512_srli_epi64(_mm512_mul_epu32(a, q), 32);
		auto odd = _mm512_mul_epu32(_mm512_srli_epi64(a, 32), q);
		auto high = _mm512_mask_blend_epi32(0xaaaa, even, odd);
		auto r = _mm512_sub_epi32(_mm512_mullo_epi32(a, x), _mm512_mullo_epi32(high, p));
		return _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
	}
	template <bool add_to>
	__attribute__((target("avx512f"))) static void apply16(V *a, const V *b, int n, V x, V q) {
		const auto vx = _mm512_set1_epi32(x), vq = _mm512_set1_epi32(q), p = _mm512_set1_epi32(mod);
		int i = 0;
		for (; i + 16 <= n; i += 16) {
			auto r = product16(_mm512_loadu_si512(b + i), vx, vq);
			if constexpr (add_to) {
				r = _mm512_add_epi32(r, _mm512_loadu_si512(a + i));
				r = _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
			}
			_mm512_storeu_si512(a + i, r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	template <bool add_to>
	__attribute__((target("avx2"))) static void apply8(V *a, const V *b, int n, V x, V q) {
		__m256i vx = _mm256_set1_epi32(x), vq = _mm256_set1_epi32(q), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			__m256i r = product8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), vx, vq);
			if constexpr (add_to) {
				r = _mm256_add_epi32(r, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)));
				r = _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	static void apply(V *a, const V *b, int n, V x) {
		V q = (std::uint64_t(x) << 32) / mod;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 64 && avx2() && __builtin_cpu_supports("avx512f")) { apply16<add_to>(a, b, n, x, q); return; }
		if (n >= 16 && avx2()) { apply8<add_to>(a, b, n, x, q); return; }
#endif
		for (int i = 0; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	static void scale(V *a, int n, V x) { apply<false>(a, a, n, x); }
	static void axpy(V *a, const V *b, int n, V x) { apply<true>(a, b, n, x); }
};

}


#line 6 "components/matrix_arithmetic.hpp"

#line 15 "components/matrix_arithmetic.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace matrix_detail {
using field_detail::SmallModint;
using field_detail::small_modint;
using field_detail::avx2;
using field_detail::ModularField;

template <class T, class F>
auto field(std::vector<T> &a, F &&f) {
	static_assert(!integer<T>);
	if constexpr (small_modint<T>) {
		std::vector<std::uint32_t> work(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) work[i] = a[i].val();
		auto result = f.template operator()<ModularField<T>>(work);
		for (std::size_t i = 0; i < a.size(); ++i) a[i] = T::raw(work[i]);
		return result;
	} else return f.template operator()<Field<T>>(a);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template <std::uint32_t mod>
void product_scalar(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; j += 32) {
		int width = std::min(32, m - j);
		for (int k = 0; k < inner; k += step) {
			std::uint64_t sums[32]{};
			for (int t = k; t < std::min(inner, k + step); ++t) {
				std::uint64_t x = a[std::size_t(i) * inner + t];
				const auto *row = b + std::size_t(t) * m + j;
				for (int s = 0; s < width; ++s) sums[s] += x * row[s];
			}
			for (int s = 0; s < width; ++s) c[std::size_t(i) * m + j + s] = (sums[s] + c[std::size_t(i) * m + j + s]) % mod;
		}
	}
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
inline bool avx512_product() {
	static const bool supported = __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512dq");
	return avx2() && supported;
}

template <std::uint32_t mod>
__attribute__((target("avx512f,avx512dq"))) __m512i reduce_product_avx512(__m512i sum) {
	const auto p = _mm512_set1_epi64(mod);
	const auto q = _mm512_cvt_roundpd_epi64(
		_mm512_mul_pd(_mm512_cvtepi64_pd(sum), _mm512_set1_pd(1.0 / mod)),
		_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
	auto r = _mm512_sub_epi64(sum, _mm512_mullo_epi64(q, p));
	r = _mm512_mask_add_epi64(r, _mm512_cmp_epi64_mask(r, _mm512_setzero_si512(), _MM_CMPINT_LT), r, p);
	return _mm512_mask_sub_epi64(r, _mm512_cmp_epi64_mask(r, p, _MM_CMPINT_NLT), r, p);
}

template <std::uint32_t mod, int rows = 4>
__attribute__((target("avx512f,avx512dq"))) void product_packed_avx512(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - (mod - 1)) / (radius * radius)));
	int i = 0;
	for (; i + rows <= n; i += rows) {
		for (int j = 0; j < m; j += 16) {
			const int width = std::min(16, m - j);
			const __mmask16 active = (1u << width) - 1;
			for (int k = 0; k < inner; k += step) {
				__m512i sums[rows][2];
				for (int r = 0; r < rows; ++r) sums[r][0] = sums[r][1] = _mm512_setzero_si512();
				for (int t = k; t < std::min(inner, k + step); ++t) {
					auto x = _mm512_maskz_loadu_epi32(active, b + std::size_t(j) * inner + std::size_t(t) * width);
					auto y = _mm512_srli_epi64(x, 32);
					for (int r = 0; r < rows; ++r) {
						auto z = _mm512_set1_epi32(a[std::size_t(i + r) * inner + t]);
						sums[r][0] = _mm512_add_epi64(sums[r][0], _mm512_mul_epi32(x, z));
						sums[r][1] = _mm512_add_epi64(sums[r][1], _mm512_mul_epi32(y, z));
					}
				}
				for (int r = 0; r < rows; ++r) {
					auto *dst = c + std::size_t(i + r) * m + j;
					auto previous = _mm512_maskz_loadu_epi32(active, dst);
					auto even = reduce_product_avx512<mod>(_mm512_add_epi64(sums[r][0], _mm512_and_si512(previous, _mm512_set1_epi64(0xffffffffu))));
					auto odd = reduce_product_avx512<mod>(_mm512_add_epi64(sums[r][1], _mm512_srli_epi64(previous, 32)));
					_mm512_mask_storeu_epi32(dst, active, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
				}
			}
		}
	}
	if constexpr (rows == 4) {
		if (i < n) product_packed_avx512<mod, 1>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	}
}

template <std::uint32_t mod>
__attribute__((target("avx2"))) void product_avx2(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	int i = 0;
	for (; i + 2 <= n; i += 2) {
		int j = 0;
		for (; j + 8 <= m; j += 8) for (int k = 0; k < inner; k += step) {
			__m256i x0 = _mm256_setzero_si256(), x1 = x0, y0 = x0, y1 = x0;
			for (int t = k; t < std::min(inner, k + step); ++t) {
				__m256i row = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + std::size_t(t) * m + j));
				__m256i odd = _mm256_srli_epi64(row, 32);
				__m256i u = _mm256_set1_epi32(a[std::size_t(i) * inner + t]);
				__m256i v = _mm256_set1_epi32(a[std::size_t(i + 1) * inner + t]);
				x0 = _mm256_add_epi64(x0, _mm256_mul_epu32(u, row));
				x1 = _mm256_add_epi64(x1, _mm256_mul_epu32(u, odd));
				y0 = _mm256_add_epi64(y0, _mm256_mul_epu32(v, row));
				y1 = _mm256_add_epi64(y1, _mm256_mul_epu32(v, odd));
			}
			std::uint64_t even[4], odd[4], even2[4], odd2[4];
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even2), y0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd2), y1);
			for (int s = 0; s < 4; ++s) {
				auto *r = c + std::size_t(i) * m + j + s * 2;
				auto *r2 = r + m;
				r[0] = (even[s] + r[0]) % mod; r[1] = (odd[s] + r[1]) % mod;
				r2[0] = (even2[s] + r2[0]) % mod; r2[1] = (odd2[s] + r2[1]) % mod;
			}
		}
		if (j < m) for (int row = i; row < i + 2; ++row) for (int col = j; col < m; ++col) {
			std::uint64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::uint64_t(a[std::size_t(row) * inner + t]) * b[std::size_t(t) * m + col];
				sum %= mod;
			}
			c[std::size_t(row) * m + col] = sum;
		}
	}
	if (i < n) product_scalar<mod>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, 1, inner, m);
}

template <std::uint32_t mod, int rows = 4>
__attribute__((target("avx2"))) void product_packed_avx2(const std::int32_t *a, const std::int32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr std::int64_t radius = mod / 2;
	constexpr int step = int(std::min<std::int64_t>(32, (std::numeric_limits<std::int64_t>::max() - (mod - 1)) / (radius * radius)));
	auto reduce = [](std::int64_t sum, std::uint32_t c = 0) {
		std::int64_t r = (sum + std::int64_t(c)) % std::int64_t(mod);
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	int i = 0;
	for (; i + rows <= n; i += rows) {
		int j = 0;
		for (; j + 8 <= m; j += 8) for (int k = 0; k < inner; k += step) {
			__m256i x0 = _mm256_setzero_si256(), x1 = x0, y0 = x0, y1 = x0, z0 = x0, z1 = x0, w0 = x0, w1 = x0;
			for (int t = k; t < std::min(inner, k + step); ++t) {
				__m256i row = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + std::size_t(j) * inner + std::size_t(t) * 8));
				__m256i odd = _mm256_srli_epi64(row, 32);
				__m256i u = _mm256_set1_epi32(a[std::size_t(i) * inner + t]);
				__m256i v = _mm256_set1_epi32(a[std::size_t(i + 1) * inner + t]);
				x0 = _mm256_add_epi64(x0, _mm256_mul_epi32(u, row));
				x1 = _mm256_add_epi64(x1, _mm256_mul_epi32(u, odd));
				y0 = _mm256_add_epi64(y0, _mm256_mul_epi32(v, row));
				y1 = _mm256_add_epi64(y1, _mm256_mul_epi32(v, odd));
				if constexpr (rows == 4) {
					__m256i p = _mm256_set1_epi32(a[std::size_t(i + 2) * inner + t]);
					__m256i q = _mm256_set1_epi32(a[std::size_t(i + 3) * inner + t]);
					z0 = _mm256_add_epi64(z0, _mm256_mul_epi32(p, row));
					z1 = _mm256_add_epi64(z1, _mm256_mul_epi32(p, odd));
					w0 = _mm256_add_epi64(w0, _mm256_mul_epi32(q, row));
					w1 = _mm256_add_epi64(w1, _mm256_mul_epi32(q, odd));
				}
			}
			std::int64_t even[4], odd[4], even2[4], odd2[4], even3[4], odd3[4], even4[4], odd4[4];
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even2), y0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd2), y1);
			if constexpr (rows == 4) {
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even3), z0);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd3), z1);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(even4), w0);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd4), w1);
			}
			for (int s = 0; s < 4; ++s) {
				auto *r = c + std::size_t(i) * m + j + s * 2;
				auto *r2 = r + m;
				r[0] = reduce(even[s], r[0]); r[1] = reduce(odd[s], r[1]);
				r2[0] = reduce(even2[s], r2[0]); r2[1] = reduce(odd2[s], r2[1]);
				if constexpr (rows == 4) {
					auto *r3 = r2 + m, *r4 = r3 + m;
					r3[0] = reduce(even3[s], r3[0]); r3[1] = reduce(odd3[s], r3[1]);
					r4[0] = reduce(even4[s], r4[0]); r4[1] = reduce(odd4[s], r4[1]);
				}
			}
		}
		if (j < m) for (int row = i; row < i + rows; ++row) for (int col = j; col < m; ++col) {
			std::int64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::int64_t(a[std::size_t(row) * inner + t]) * b[std::size_t(j) * inner + std::size_t(t) * (m - j) + col - j];
				sum = reduce(sum);
			}
			c[std::size_t(row) * m + col] = sum;
		}
	}
	if constexpr (rows == 4) {
		if (i < n) product_packed_avx2<mod, 2>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, n - i, inner, m);
	} else for (; i < n; ++i) for (int j = 0; j < m; j += 8) {
		int width = std::min(8, m - j);
		for (int s = 0; s < width; ++s) {
			std::int64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::int64_t(a[std::size_t(i) * inner + t]) * b[std::size_t(j) * inner + std::size_t(t) * width + s];
				sum = reduce(sum);
			}
			c[std::size_t(i) * m + j + s] = sum;
		}
	}
}
#endif

template <class T>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	if (!n || !inner || !m) return c;
	auto basic = [&] {
		for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
			const T &x = a[std::size_t(i) * inner + k];
			for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
		}
	};
	if constexpr (small_modint<T>) {
		if (std::uint64_t(n) * m <= 128u / unsigned(inner)) { basic(); return c; }
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 4 && inner >= 16 && m >= 8 && avx2()) {
			const int block = avx512_product() ? 16 : 8;
			std::vector<std::int32_t> x(a.size()), y(b.size());
			std::vector<std::uint32_t> z(c.size());
			auto center = [](std::uint32_t value) {
				std::int32_t v = value;
				return v <= std::int32_t((T::mod() - 1) / 2) ? v : v - std::int32_t(T::mod());
			};
			for (std::size_t i = 0; i < a.size(); ++i) x[i] = center(a[i].val());
			for (int j = 0; j < m; j += block) {
				int width = std::min(block, m - j);
				for (int k = 0; k < inner; ++k) for (int s = 0; s < width; ++s)
					y[std::size_t(j) * inner + std::size_t(k) * width + s] = center(b[std::size_t(k) * m + j + s].val());
			}
			if (block == 16) product_packed_avx512<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			else product_packed_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
			for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
			return c;
		}
#endif
		std::vector<std::uint32_t> x(a.size()), y(b.size()), z(c.size());
		for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
		for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (inner >= 16 && m >= 8 && avx2()) product_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		else
#endif
		product_scalar<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
	} else basic();
	return c;
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif
}


#line 1 "components/matrix_power.hpp"



#line 1 "components/matrix_elimination_modular.hpp"



#line 6 "components/matrix_elimination_modular.hpp"

namespace matrix_detail {
template <class Ops>
struct ModularElimination {
	using V = typename Ops::V;
	static constexpr int batch = int(std::min<std::uint64_t>(8, (std::numeric_limits<std::int64_t>::max() - (Ops::mod - 1)) / (std::uint64_t(Ops::mod - 1) * (Ops::mod - 1))));
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx512f,avx512dq"))) static __m512i reduce512(__m512i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm512_add_epi64(sum, _mm512_mul_epu32(_mm512_mul_epu32(sum, _mm512_set1_epi32(negative_inverse)), _mm512_set1_epi32(Ops::mod)));
		sum = _mm512_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm512_min_epu32(sum, _mm512_sub_epi32(sum, _mm512_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx512f,avx512dq"))) static void update512(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 16 <= length; j += 16) {
			auto x = _mm512_loadu_si512(dst + j);
			auto even = _mm512_setzero_si512(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm512_loadu_si512(src[k] + first + j), b = _mm512_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm512_add_epi64(even, _mm512_mul_epu32(a, b));
				odd = _mm512_add_epi64(odd, _mm512_mul_epu32(_mm512_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) even = reduce512(even), odd = reduce512(odd);
			else even = reduce_product_avx512<Ops::mod>(even), odd = reduce_product_avx512<Ops::mod>(odd);
			x = _mm512_add_epi32(x, _mm512_or_si512(even, _mm512_slli_epi64(odd, 32)));
			_mm512_storeu_si512(dst + j, _mm512_min_epu32(x, _mm512_sub_epi32(x, _mm512_set1_epi32(Ops::mod))));
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
	__attribute__((target("avx2"))) static __m256i reduce256(__m256i sum) {
		constexpr V negative_inverse = [] { V v = Ops::mod; for (int i = 0; i < 4; ++i) v *= 2 - Ops::mod * v; return V(0) - v; }();
		sum = _mm256_add_epi64(sum, _mm256_mul_epu32(_mm256_mul_epu32(sum, _mm256_set1_epi32(negative_inverse)), _mm256_set1_epi32(Ops::mod)));
		sum = _mm256_srli_epi64(sum, 32);
		for (int i = 0; i < 2; ++i) sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(Ops::mod)));
		return sum;
	}
	__attribute__((target("avx2"))) static void update256(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		std::array<V, batch> encoded;
		for (int k = 0; k < count; ++k) encoded[k] = std::uint64_t(coeff[k]) * ((std::uint64_t(1) << 32) % Ops::mod) % Ops::mod;
		int j = 0;
		for (; j + 8 <= length; j += 8) {
			auto even = _mm256_setzero_si256(), odd = even;
			for (int k = 0; k < count; ++k) {
				auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src[k] + first + j));
				auto b = _mm256_set1_epi32(Ops::mod % 2 ? encoded[k] : coeff[k]);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(a, b));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(a, 32), b));
			}
			if constexpr (Ops::mod % 2) {
				even = reduce256(even), odd = reduce256(odd);
				auto value = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(dst + j)), _mm256_or_si256(even, _mm256_slli_epi64(odd, 32)));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(dst + j), _mm256_min_epu32(value, _mm256_sub_epi32(value, _mm256_set1_epi32(Ops::mod))));
			} else {
				std::uint64_t x[4], y[4];
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(x), even);
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(y), odd);
				for (int k = 0; k < 4; ++k) {
					dst[j + 2 * k] = (x[k] + dst[j + 2 * k]) % Ops::mod;
					dst[j + 2 * k + 1] = (y[k] + dst[j + 2 * k + 1]) % Ops::mod;
				}
			}
		}
		for (; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
#endif
	static void update(V *dst, const V *const *src, const V *coeff, int count, int first, int length) {
		if (!count || !length) return;
		if (count == 1) { if (coeff[0]) Ops::axpy(dst, src[0] + first, length, coeff[0]); return; }
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (length >= 32 && avx512_product()) { update512(dst, src, coeff, count, first, length); return; }
		if (length >= 16 && avx2()) { update256(dst, src, coeff, count, first, length); return; }
#endif
		for (int j = 0; j < length; ++j) {
			std::uint64_t sum = dst[j];
			for (int k = 0; k < count; ++k) sum += std::uint64_t(src[k][first + j]) * coeff[k];
			dst[j] = sum % Ops::mod;
		}
	}
	static Elimination<Ops> run(std::vector<V> &a, int h, int stride, int columns, bool reduced) {
		Elimination<Ops> result;
		result.pivots.reserve(std::min(h, columns));
		std::vector<std::array<V, batch>> factors(h);
		std::array<const V *, batch> source;
		int rank = 0;
		for (int first = 0; first < columns && rank < h; first += batch) {
			int end = std::min(columns, first + batch), start = rank;
			for (int i = rank; i < h; ++i) factors[i].fill(0);
			for (int col = first; col < end && rank < h; ++col) {
				int p = rank;
				while (p < h && !a[std::size_t(p) * stride + col]) ++p;
				if (p == h) continue;
				V *row = a.data() + std::size_t(rank) * stride;
				if (p != rank) {
					std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
					std::swap(factors[p], factors[rank]);
					result.determinant = Ops::neg(result.determinant);
				}
				update(row + end, source.data(), factors[rank].data(), rank - start, end, stride - end);
				result.determinant = Ops::mul(result.determinant, row[col]);
				Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
				row[col] = 1;
				source[rank - start] = row;
				for (int i = rank + 1; i < h; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					V x = factors[i][rank - start] = Ops::neg(dst[col]);
					if (x) Ops::axpy(dst + col + 1, row + col + 1, end - col - 1, x);
					dst[col] = 0;
				}
				result.pivots.push_back(col);
				++rank;
			}
			for (int i = rank; i < h; ++i) update(a.data() + std::size_t(i) * stride + end, source.data(), factors[i].data(), rank - start, end, stride - end);
		}
		bool full = rank == columns;
		if (reduced) for (int end = rank; end > 0;) {
			int start = std::max(0, end - batch);
			for (int k = end - 1; k >= start; --k) {
				int col = result.pivots[k], first = full ? columns : col + 1;
				const V *row = a.data() + std::size_t(k) * stride;
				for (int i = start; i < k; ++i) {
					V *dst = a.data() + std::size_t(i) * stride;
					if (V x = dst[col]) Ops::axpy(dst + first, row + first, stride - first, Ops::neg(x));
					dst[col] = 0;
				}
			}
			int first = full ? columns : result.pivots[start] + 1;
			for (int k = start; k < end; ++k) source[k - start] = a.data() + std::size_t(k) * stride;
			for (int i = 0; i < start; ++i) {
				V *dst = a.data() + std::size_t(i) * stride;
				for (int k = start; k < end; ++k) factors[i][k - start] = Ops::neg(dst[result.pivots[k]]);
				update(dst + first, source.data(), factors[i].data(), end - start, first, stride - first);
				for (int k = start; k < end; ++k) dst[result.pivots[k]] = 0;
			}
			end = start;
		}
		return result;
	}
};
}


#line 7 "components/matrix_power.hpp"
#include <optional>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace matrix_detail {
constexpr bool prime_modulus(std::uint32_t p) {
	if (p < 2 || p % 2 == 0) return p == 2;
	if (p % 3 == 0) return p == 3;
	for (std::uint32_t d = 5; std::uint64_t(d) * d <= p; d += 6)
		if (p % d == 0 || p % (d + 2) == 0) return false;
	return true;
}

template <class T>
struct FrobeniusPower {
	using Ops = ModularField<T>;
	using V = std::uint32_t;
	using Row = std::vector<V>;
	int n, work = 0;
	std::vector<Row> basis, reduced;
	std::vector<int> pivots;
	Row transpose, upper, inverses;
	std::uint64_t random = 0x243f6a8885a308d3ULL;
	explicit FrobeniusPower(const std::vector<T> &a, int size) : n(size), transpose(a.size()), upper(a.size()), inverses(size) {
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) transpose[std::size_t(j) * n + i] = a[std::size_t(i) * n + j].val();
	}
	Row seed() {
		Row x(n);
		for (auto &v : x) {
			random ^= random << 7;
			random ^= random >> 9;
			v = random % Ops::mod;
		}
		return x;
	}
	Row apply(const Row &x) const {
		Row result(n);
		constexpr int count = ModularElimination<Ops>::batch;
		std::array<const V *, count> source;
		for (int i = 0; i < n; i += count) {
			int used = std::min(count, n - i);
			for (int j = 0; j < used; ++j) source[j] = transpose.data() + std::size_t(i + j) * n;
			ModularElimination<Ops>::update(result.data(), source.data(), x.data() + i, used, 0, n);
		}
		return result;
	}
	std::optional<Row> chain(Row x, int first = 0) {
		while (++work <= 4 * n) {
			const int m = int(basis.size());
			Row y = x, factors(m);
			constexpr int count = ModularElimination<Ops>::batch;
			std::array<const V *, count> source;
			std::array<V, count> coeff;
			for (int j = 0; j < m; j += count) {
				int used = std::min(count, m - j);
				for (int u = 0; u < used; ++u) {
					int p = pivots[j + u];
					std::uint64_t value = y[p];
					for (int k = 0; k < u; ++k) value += std::uint64_t(source[k][p]) * coeff[k];
					V factor = factors[j + u] = value % Ops::mod;
					coeff[u] = Ops::neg(factor);
					source[u] = reduced[j + u].data();
				}
				ModularElimination<Ops>::update(y.data(), source.data(), coeff.data(), used, 0, n);
			}
			int p = 0;
			while (p < n && !y[p]) ++p;
			if (p == n) {
				for (int i = m - 1; i >= first; --i) {
					V sum = Ops::dot(upper.data() + std::size_t(i) * n + i + 1, factors.data() + i + 1, m - i - 1);
					factors[i] = Ops::mul(Ops::add(factors[i], Ops::neg(sum)), inverses[i]);
				}
				for (auto &value : factors) value = Ops::neg(value);
				factors.push_back(1);
				return factors;
			}
			for (int j = 0; j < m; ++j) upper[std::size_t(j) * n + m] = factors[j];
			upper[std::size_t(m) * n + m] = y[p];
			inverses[m] = Ops::inv(y[p]);
			Ops::scale(y.data() + p, n - p, inverses[m]);
			pivots.push_back(p);
			reduced.push_back(std::move(y));
			basis.push_back(std::move(x));
			x = apply(basis.back());
		}
		return std::nullopt;
	}
	static Row power(const Row &relation, std::uint64_t k) {
		const int d = int(relation.size());
		Row p{1};
		for (int bit = int(std::bit_width(k)) - 1; bit >= 0; --bit) {
			const int m = int(p.size());
			Row reversed(p.rbegin(), p.rend()), square(2 * m - 1);
			for (int i = 0; i < int(square.size()); ++i) {
				int first = std::max(0, i - m + 1), last = std::min(m, i + 1);
				square[i] = Ops::dot(p.data() + first, reversed.data() + m - 1 - i + first, last - first);
			}
			for (int i = int(square.size()) - 1; i >= d; --i)
				if (square[i]) Ops::axpy(square.data() + i - d, relation.data(), d, square[i]);
			square.resize(std::min<int>(d, square.size()));
			p = std::move(square);
			if ((k >> bit) & 1) {
				p.insert(p.begin(), 0);
				if (int(p.size()) > d) {
					V lead = p.back();
					p.pop_back();
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
			}
		}
		p.resize(d);
		return p;
	}
	std::optional<std::vector<Row>> decompose(bool diagonal = true) {
		for (int attempt = 0; attempt < 3; ++attempt) {
			basis.clear(), reduced.clear(), pivots.clear();
			work = 0;
			std::vector<Row> relations;
			bool valid = true;
			while (int(basis.size()) < n) {
				const int start = int(basis.size());
				Row x = seed();
				if (!diagonal) for (int p : pivots) x[p] = 0;
				auto rec = chain(std::move(x), diagonal ? 0 : start);
				if (!rec) { valid = false; break; }
				if (int(basis.size()) == start) {
					Row x(n);
					int p = 0;
					while (std::find(pivots.begin(), pivots.end(), p) != pivots.end()) ++p;
					x[p] = 1;
					rec = chain(std::move(x), diagonal ? 0 : start);
					if (!rec) { valid = false; break; }
				}
				const int d = int(basis.size()) - start;
				if (!diagonal && !start && d <= 2) return std::nullopt;
				if (diagonal && std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
					Row remainder = *rec, quotient(start + 1);
					for (int i = int(remainder.size()) - 1; i >= d; --i) {
						V factor = remainder[i];
						quotient[i - d] = factor;
						Ops::axpy(remainder.data() + i - d, rec->data() + start, d + 1, Ops::neg(factor));
					}
					Row x = basis[start];
					for (int j = 0; j < start; ++j) if (quotient[j]) Ops::axpy(x.data(), basis[j].data(), n, quotient[j]);
					basis.resize(start), reduced.resize(start), pivots.resize(start);
					rec = chain(std::move(x));
					if (!rec || int(basis.size()) != start + d || std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
						valid = false;
						break;
					}
				}
				Row relation(d);
				for (int i = 0; i < d; ++i) relation[i] = Ops::neg((*rec)[start + i]);
				relations.push_back(std::move(relation));
			}
			if (!valid) continue;
			return relations;
		}
		return std::nullopt;
	}
	std::optional<std::vector<T>> characteristic() {
		bool upper_triangular = true, lower_triangular = true;
		for (int i = 0; i < n && (upper_triangular || lower_triangular); ++i) for (int j = 0; j < i; ++j) {
			upper_triangular &= transpose[std::size_t(j) * n + i] == 0;
			lower_triangular &= transpose[std::size_t(i) * n + j] == 0;
		}
		if (upper_triangular || lower_triangular) {
			Row p{1};
			for (int i = 0; i < n; ++i) {
				Row next(p.size() + 1);
				std::copy(p.begin(), p.end(), next.begin() + 1);
				Ops::axpy(next.data(), p.data(), int(p.size()), Ops::neg(transpose[std::size_t(i) * n + i]));
				p = std::move(next);
			}
			std::vector<T> result(p.size());
			for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
			return result;
		}
		auto relations = decompose(false);
		if (!relations) return std::nullopt;
		Row p{1};
		for (const auto &relation : *relations) {
			const int d = int(relation.size());
			Row next(p.size() + d);
			for (int i = 0; i < int(p.size()); ++i) {
				next[i + d] = Ops::add(next[i + d], p[i]);
				if (p[i]) Ops::axpy(next.data() + i, relation.data(), d, Ops::neg(p[i]));
			}
			p = std::move(next);
		}
		std::vector<T> result(p.size());
		for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
		return result;
	}
	std::optional<std::vector<T>> run(std::uint64_t exponent) {
		auto relation_blocks = decompose();
		if (!relation_blocks) return std::nullopt;
		const auto &relations = *relation_blocks;
			std::vector<T> b(std::size_t(n) * n), companion(b.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				b[std::size_t(j) * n + i] = T::raw(basis[i][j]);
			int start = 0;
			for (const auto &relation : relations) {
				const int d = int(relation.size());
				Row p = power(relation, exponent);
				for (int j = 0; j < d; ++j) {
					for (int i = 0; i < d; ++i) companion[std::size_t(start + i) * n + start + j] = T::raw(p[i]);
					V lead = p.back();
					std::move_backward(p.begin(), p.end() - 1, p.end());
					p[0] = 0;
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
				start += d;
			}
			auto result = multiply(b, companion, n, n, n);
			Row columns(result.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				columns[std::size_t(j) * n + i] = result[std::size_t(i) * n + j].val();
			for (int j = 0; j < n; ++j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = 0; i < j; ++i) if (upper[std::size_t(i) * n + j])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(upper[std::size_t(i) * n + j]));
				Ops::scale(column, n, inverses[j]);
			}
			for (int j = n - 1; j >= 0; --j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = j + 1; i < n; ++i) if (reduced[j][pivots[i]])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(reduced[j][pivots[i]]));
			}
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				result[std::size_t(i) * n + pivots[j]] = T::raw(columns[std::size_t(j) * n + i]);
		return result;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 7 "math/linear_algebra/matrix.hpp"

namespace matrix_detail {
template <class T>
struct FastMatrixArithmetic {
	static constexpr bool modular = small_modint<T>;
	template <class Ops>
	static auto eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
		if constexpr (modular) if (h >= 32 && columns >= 32) return ModularElimination<Ops>::run(a, h, stride, columns, reduced);
		return matrix_detail::eliminate<Ops, modular>(a, h, stride, columns, reduced, eps);
	}
	template <class F>
	static auto field(std::vector<T> &a, F &&f) { return matrix_detail::field(a, std::forward<F>(f)); }
	static std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
		return matrix_detail::multiply(a, b, n, inner, m);
	}
	static std::optional<std::vector<T>> characteristic(const std::vector<T> &a, int n) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 32) return FrobeniusPower<T>(a, n).characteristic();
		}
		return std::nullopt;
	}
	static std::optional<std::vector<T>> power(const std::vector<T> &a, int n, std::uint64_t exponent) {
		if constexpr (small_modint<T>) if constexpr (prime_modulus(T::mod())) {
			if (n >= 16 && exponent >= 256) return FrobeniusPower<T>(a, n).run(exponent);
		}
		return std::nullopt;
	}
};
}

#line 1 "components/matrix_core.hpp"



#line 9 "components/matrix_core.hpp"
#include <initializer_list>
#line 12 "components/matrix_core.hpp"
#include <span>
#line 16 "components/matrix_core.hpp"

template <class T>
struct LinearSolution {
	std::vector<T> particular;
	std::vector<std::vector<T>> basis;
};

namespace matrix_detail {
template <class T> struct FastMatrixArithmetic;
}

template <class T, class Arithmetic = matrix_detail::FastMatrixArithmetic<T>>
class Matrix {
	static_assert(!std::is_same_v<T, bool>);
	int h = 0, w = 0;
	std::vector<T> a;
	std::pair<T, std::vector<int>> reduce(int columns, bool reduced, long double eps) {
		assert(eps >= 0);
		return Arithmetic::field(a, [&]<class Ops>(auto &work) {
			auto result = [&] {
				if constexpr (requires { Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps); })
					return Arithmetic::template eliminate<Ops>(work, h, w, columns, reduced, eps);
				else return matrix_detail::eliminate<Ops, Arithmetic::modular>(work, h, w, columns, reduced, eps);
			}();
			return std::pair{T(result.determinant), std::move(result.pivots)};
		});
	}
	static std::vector<std::vector<T>> nullspace(const Matrix &r, const std::vector<int> &pivots, int variables) {
		std::vector<bool> pivot(variables);
		for (int p : pivots) pivot[p] = true;
		std::vector<std::vector<T>> basis;
		basis.reserve(variables - pivots.size());
		for (int col = 0; col < variables; ++col) if (!pivot[col]) {
			std::vector<T> v(variables, T(0));
			v[col] = T{1};
			for (int i = 0; i < int(pivots.size()); ++i) v[pivots[i]] = -r(i, col);
			basis.push_back(std::move(v));
		}
		return basis;
	}
	Matrix augmented_identity() const {
		static_assert(!matrix_detail::integer<T>);
		assert(h == w && w <= std::numeric_limits<int>::max() / 2);
		Matrix work(h, w * 2);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w + i) = T{1};
		}
		return work;
	}

  public:
	using value_type = T;
	static constexpr long double default_epsilon = std::is_floating_point_v<T> ? 1e-12L : 0;
	Matrix() = default;
	explicit Matrix(int n) : Matrix(n, n) {}
	Matrix(int rows, int columns, T value = T(0)) : h(rows), w(columns) {
		assert(rows >= 0 && columns >= 0);
		a.assign(std::size_t(h) * w, value);
	}
	Matrix(std::initializer_list<std::initializer_list<T>> values) : Matrix(int(values.size()), values.size() ? int(values.begin()->size()) : 0) {
		int i = 0;
		for (const auto &row : values) {
			assert(int(row.size()) == w);
			std::copy(row.begin(), row.end(), (*this)[i++].begin());
		}
	}
	explicit Matrix(const std::vector<std::vector<T>> &values) : Matrix(int(values.size()), values.empty() ? 0 : int(values[0].size())) {
		for (int i = 0; i < h; ++i) {
			assert(int(values[i].size()) == w);
			std::copy(values[i].begin(), values[i].end(), (*this)[i].begin());
		}
	}
	Matrix(const Matrix &) = default;
	Matrix &operator=(const Matrix &) = default;
	Matrix(Matrix &&other) noexcept { swap(other); }
	Matrix &operator=(Matrix &&other) noexcept {
		if (this != &other) { Matrix next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(Matrix &other) noexcept { std::swap(h, other.h); std::swap(w, other.w); a.swap(other.a); }
	int rows() const { return h; }
	int cols() const { return w; }
	bool empty() const { return !h || !w; }
	T *data() { return a.data(); }
	const T *data() const { return a.data(); }
	std::span<T> operator[](int i) { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	std::span<const T> operator[](int i) const { assert(0 <= i && i < h); return {w ? a.data() + std::size_t(i) * w : a.data(), std::size_t(w)}; }
	T &operator()(int i, int j) { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	const T &operator()(int i, int j) const { assert(0 <= i && i < h && 0 <= j && j < w); return a[std::size_t(i) * w + j]; }
	bool operator==(const Matrix &) const = default;
	static Matrix identity(int n) {
		Matrix result(n);
		for (int i = 0; i < n; ++i) result(i, i) = T{1};
		return result;
	}
	void fill(T value) { std::fill(a.begin(), a.end(), value); }
	Matrix transpose() const {
		Matrix result(w, h);
		for (int i = 0; i < h; i += 32) for (int j = 0; j < w; j += 32)
			for (int x = i; x < std::min(h, i + 32); ++x) for (int y = j; y < std::min(w, j + 32); ++y) result(y, x) = (*this)(x, y);
		return result;
	}
	Matrix submatrix(int row_l, int row_r, int col_l, int col_r) const {
		assert(0 <= row_l && row_l <= row_r && row_r <= h && 0 <= col_l && col_l <= col_r && col_r <= w);
		Matrix result(row_r - row_l, col_r - col_l);
		for (int i = row_l; i < row_r; ++i) std::copy((*this)[i].begin() + col_l, (*this)[i].begin() + col_r, result[i - row_l].begin());
		return result;
	}
	Matrix operator-() const { Matrix result = *this; for (auto &x : result.a) x = -x; return result; }
	Matrix &operator+=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] += b.a[i]; return *this; }
	Matrix &operator-=(const Matrix &b) { assert(h == b.h && w == b.w); for (std::size_t i = 0; i < a.size(); ++i) a[i] -= b.a[i]; return *this; }
	Matrix &operator*=(const Matrix &b) { return *this = *this * b; }
	Matrix &operator*=(T x) { for (auto &v : a) v *= x; return *this; }
	Matrix &operator/=(T x) { static_assert(!matrix_detail::integer<T>); assert(x != T(0)); return *this *= T{1} / x; }
	friend Matrix operator+(Matrix a, const Matrix &b) { a += b; return a; }
	friend Matrix operator-(Matrix a, const Matrix &b) { a -= b; return a; }
	friend Matrix operator*(const Matrix &a, const Matrix &b) {
		assert(a.w == b.h);
		Matrix result;
		result.h = a.h; result.w = b.w;
		result.a = Arithmetic::multiply(a.a, b.a, a.h, a.w, b.w);
		return result;
	}
	friend Matrix operator*(Matrix a, T x) { a *= x; return a; }
	friend Matrix operator*(T x, Matrix a) { a *= x; return a; }
	friend Matrix operator/(Matrix a, T x) { a /= x; return a; }
	std::vector<T> operator*(const std::vector<T> &v) const {
		assert(int(v.size()) == w);
		return Arithmetic::multiply(a, v, h, w, 1);
	}
	Matrix pow(std::uint64_t exponent) const {
		assert(h == w);
		if (auto power = Arithmetic::power(a, h, exponent)) {
			Matrix result(h);
			result.a = std::move(*power);
			return result;
		}
		return pow(exponent, [](Matrix &) {});
	}
	template <class Normalize>
	Matrix pow(std::uint64_t exponent, Normalize normalize) const {
		assert(h == w);
		Matrix result = identity(h), x = *this;
		normalize(result);
		normalize(x);
		while (exponent) {
			if (exponent & 1) {
				result *= x;
				normalize(result);
			}
			exponent >>= 1;
			if (exponent) {
				x *= x;
				normalize(x);
			}
		}
		return result;
	}
	T trace() const { assert(h == w); T result = T(0); for (int i = 0; i < h; ++i) result += (*this)(i, i); return result; }
	T det(long double eps = default_epsilon) const {
		assert(h == w);
		if constexpr (matrix_detail::integer<T>) {
			return matrix_detail::integer_determinant(a, h);
		} else {
			Matrix work = *this;
			auto [value, pivots] = work.reduce(w, false, eps);
			return int(pivots.size()) == h ? value : T(0);
		}
	}
	int rank(long double eps = default_epsilon) const { Matrix work = *this; return int(work.reduce(w, false, eps).second.size()); }
	std::pair<Matrix, std::vector<int>> row_echelon(long double eps = default_epsilon) const {
		Matrix work = *this;
		auto pivots = work.reduce(w, false, eps).second;
		return {std::move(work), std::move(pivots)};
	}
	std::pair<Matrix, std::vector<int>> rref(long double eps = default_epsilon) const {
		Matrix work = *this;
		auto pivots = work.reduce(w, true, eps).second;
		return {std::move(work), std::move(pivots)};
	}
	std::optional<Matrix> inverse(long double eps = default_epsilon) const {
		auto work = augmented_identity();
		auto pivots = work.reduce(w, true, eps).second;
		if (int(pivots.size()) < h) return std::nullopt;
		return work.submatrix(0, h, w, w * 2);
	}
	std::vector<std::vector<T>> kernel(long double eps = default_epsilon) const {
		auto [work, pivots] = rref(eps);
		return nullspace(work, pivots, w);
	}
	std::optional<LinearSolution<T>> solve(const std::vector<T> &b, long double eps = default_epsilon) const {
		assert(int(b.size()) == h && w < std::numeric_limits<int>::max());
		Matrix work(h, w + 1);
		for (int i = 0; i < h; ++i) {
			std::copy((*this)[i].begin(), (*this)[i].end(), work[i].begin());
			work(i, w) = b[i];
		}
		auto pivots = work.reduce(w, true, eps).second;
		for (int i = int(pivots.size()); i < h; ++i) if (!matrix_detail::Field<T>::zero(work(i, w), eps)) return std::nullopt;
		LinearSolution<T> result{std::vector<T>(w, T(0)), nullspace(work, pivots, w)};
		for (int i = 0; i < int(pivots.size()); ++i) result.particular[pivots[i]] = work(i, w);
		return result;
	}
	std::vector<T> characteristic_polynomial(long double eps = default_epsilon) const {
		assert(h == w && eps >= 0);
		if constexpr (requires { Arithmetic::characteristic(a, h); })
			if (auto result = Arithmetic::characteristic(a, h)) return std::move(*result);
		auto work = a;
		return Arithmetic::field(work, [&]<class Ops>(auto &b) {
			using V = typename Ops::V;
			std::vector<V> factors;
			if constexpr (Arithmetic::modular) if (h >= 16) factors.resize(h);
			for (int k = 1; k < h; ++k) {
				int pivot = -1;
				for (int i = k; i < h; ++i) if (!Ops::zero(b[std::size_t(i) * w + k - 1], eps)
					&& (pivot < 0 || Ops::better(b[std::size_t(i) * w + k - 1], b[std::size_t(pivot) * w + k - 1]))) pivot = i;
				if (pivot < 0) continue;
				if (pivot != k) {
					for (int j = 0; j < w; ++j) std::swap(b[std::size_t(k) * w + j], b[std::size_t(pivot) * w + j]);
					for (int i = 0; i < h; ++i) std::swap(b[std::size_t(i) * w + k], b[std::size_t(i) * w + pivot]);
				}
				V inverse = Ops::inv(b[std::size_t(k) * w + k - 1]);
				if constexpr (Arithmetic::modular) if (h >= 16) {
					bool any = false;
					for (int i = k + 1; i < h; ++i) {
						V factor = factors[i] = Ops::mul(b[std::size_t(i) * w + k - 1], inverse);
						if (!factor) continue;
						any = true;
						Ops::axpy(b.data() + std::size_t(i) * w + k, b.data() + std::size_t(k) * w + k, w - k, Ops::neg(factor));
						b[std::size_t(i) * w + k - 1] = 0;
					}
					if (!any) continue;
					for (int j = 0; j < h; ++j) {
						auto *row = b.data() + std::size_t(j) * w;
						row[k] = Ops::add(row[k], Ops::dot(row + k + 1, factors.data() + k + 1, h - k - 1));
					}
					continue;
				}
				for (int i = k + 1; i < h; ++i) {
					V factor = Ops::mul(b[std::size_t(i) * w + k - 1], inverse);
					if (Ops::zero(factor, eps)) continue;
					Ops::axpy(b.data() + std::size_t(i) * w + k, b.data() + std::size_t(k) * w + k, w - k, Ops::neg(factor));
					b[std::size_t(i) * w + k - 1] = V(0);
					for (int j = 0; j < h; ++j) b[std::size_t(j) * w + k] = Ops::add(b[std::size_t(j) * w + k], Ops::mul(factor, b[std::size_t(j) * w + i]));
				}
			}
			std::vector<std::vector<V>> p(h + 1);
			p[0] = {V(1)};
			for (int n = 1; n <= h; ++n) {
				p[n].assign(n + 1, V(0));
				std::copy(p[n - 1].begin(), p[n - 1].end(), p[n].begin() + 1);
				Ops::axpy(p[n].data(), p[n - 1].data(), n, Ops::neg(b[std::size_t(n - 1) * w + n - 1]));
				V product = 1;
				for (int i = n - 2; i >= 0; --i) {
					product = Ops::mul(product, b[std::size_t(i + 1) * w + i]);
					V factor = Ops::mul(product, b[std::size_t(i) * w + n - 1]);
					Ops::axpy(p[n].data(), p[i].data(), i + 1, Ops::neg(factor));
				}
			}
			std::vector<T> result(h + 1);
			for (int i = 0; i <= h; ++i) result[i] = T(p[h][i]);
			return result;
		});
	}
	Matrix adjugate(long double eps = default_epsilon) const {
		assert(h == w);
		auto work = augmented_identity();
		auto [determinant, pivots] = work.reduce(w, true, eps);
		if (int(pivots.size()) == h) return work.submatrix(0, h, w, w * 2) * determinant;
		Matrix result(h);
		if (int(pivots.size()) + 1 < h) return result;
		auto right = nullspace(work, pivots, w)[0];
		int row = 0, col = 0;
		for (int i = 1; i < h; ++i) if (work(h - 1, w + row) == T(0) || matrix_detail::Field<T>::better(work(h - 1, w + i), work(h - 1, w + row))) row = i;
		if (work(h - 1, w + row) == T(0)) return result;
		std::vector<T> left(h);
		T inverse = T{1} / work(h - 1, w + row);
		for (int i = 0; i < h; ++i) left[i] = work(h - 1, w + i) * inverse;
		while (std::find(pivots.begin(), pivots.end(), col) != pivots.end()) ++col;
		Matrix minor(h - 1);
		for (int i = 0, x = 0; i < h; ++i) if (i != row) {
			for (int j = 0, y = 0; j < w; ++j) if (j != col) minor(x, y++) = (*this)(i, j);
			++x;
		}
		T scale = minor.det(eps);
		if ((row + col) & 1) scale = -scale;
		for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) result(i, j) = scale * right[i] * left[j];
		return result;
	}
};


#line 38 "math/linear_algebra/matrix.hpp"


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 7 "math/mint.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	[[gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[gnu::always_inline]] Mint operator+() const { return *this; }
	[[gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 7 "test/fixtures/matrix_common.hpp"

inline Matrix<mint> read_matrix(FastInput &in, int n, int m) {
	Matrix<mint> a(n, m);
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) { unsigned x = 0; in.read(x); a(i, j) = mint::raw(x); }
	return a;
}

inline void write_matrix(FastOutput &out, const Matrix<mint> &a) {
	for (int i = 0; i < a.rows(); ++i) for (int j = 0; j < a.cols(); ++j) { out.write(a(i, j).val()); out.write(j + 1 == a.cols() ? '\n' : ' '); }
}


#line 3 "test/matrix_det.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0;
	in.read(n);
	out.println(read_matrix(in, n, n).det().val());
}
