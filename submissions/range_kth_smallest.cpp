#line 1 "test/offline_range_kth_smallest.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#line 1 "data_structure/offline_range_kth_smallest.hpp"



#line 1 "components/offline_range_kth.hpp"



#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace offline_range_kth_detail {
struct Frontier {
	std::vector<std::uint32_t> left, right, rank, answer;
	explicit Frontier(std::size_t n) : left(n), right(n), rank(n), answer(n) {}
};
inline void advance(Frontier &queries, const std::vector<std::uint32_t> &current, const std::vector<std::uint32_t> &prefix,
	unsigned zeros, bool finish, std::size_t start = 0) {
	for (std::size_t i = start; i < queries.left.size(); ++i) {
		auto left = queries.left[i], right = queries.right[i];
		auto a = prefix[left], b = prefix[right], count = right - left - b + a;
		bool one = queries.rank[i] >= count;
		if (finish) queries.answer[i] = (current[left] & ~1U) | unsigned(one);
		else {
			queries.left[i] = one ? zeros + a : left - a;
			queries.right[i] = one ? zeros + b : right - b;
			queries.rank[i] -= one ? count : 0;
		}
	}
}
inline void run_scalar(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
	auto n = current.size();
	std::vector<std::uint32_t> next(n), prefix(n + 1);
	while (bits--) {
		for (std::size_t i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + ((current[i] >> bits) & 1);
		unsigned zeros = unsigned(n) - prefix[n];
		if (bits) {
			std::array<unsigned, 2> cursor{0, zeros};
			for (auto value : current) next[cursor[(value >> bits) & 1]++] = value;
		}
		advance(queries, current, prefix, zeros, bits == 0);
		if (bits) current.swap(next);
	}
}
#if defined(__x86_64__) || defined(__i386__)
struct Tables {
	std::array<unsigned char, 8> prefix{};
	std::array<std::uint32_t, 2> packed{};
	std::array<int, 8> permutation{};
};
inline constexpr auto tables = [] {
	std::array<Tables, 256> result{};
	for (unsigned mask = 0; mask < 256; ++mask) {
		unsigned count = 0, selected = 0;
		for (unsigned i = 0; i < 8; ++i) {
			if (!(i & 3)) result[mask].packed[i >> 2] = count << 8;
			else result[mask].packed[i >> 2] |= (count - (result[mask].packed[i >> 2] >> 8)) << (2 * (i & 3));
			if ((mask >> i) & 1) result[mask].permutation[selected++] = int(i);
			count += (mask >> i) & 1;
			result[mask].prefix[i] = static_cast<unsigned char>(count);
		}
	}
	return result;
}();
inline bool packed_ranks(const Frontier &queries, std::size_t n) {
	if (n < 32768 || n > 0x00ffffff || queries.left.size() < 4096) return false;
	unsigned shift = std::bit_width(n) - 6;
	std::size_t stride = queries.left.size() / 32;
	std::uint64_t occupied = 0;
	for (std::size_t i = 0; i < 32; ++i) {
		occupied |= std::uint64_t(1) << (queries.left[i * stride] >> shift);
		occupied |= std::uint64_t(1) << (queries.right[i * stride] >> shift);
	}
	return std::popcount(occupied) >= 12;
}
__attribute__((target("avx2"), always_inline))
inline void store_selected(std::uint32_t *output, unsigned remaining, __m256i values, unsigned mask) {
	auto permutation = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(tables[mask].permutation.data()));
	auto selected = _mm256_permutevar8x32_epi32(values, permutation);
	if (remaining >= 8) _mm256_storeu_si256(reinterpret_cast<__m256i *>(output), selected);
	else {
		auto active = _mm256_cmpgt_epi32(_mm256_set1_epi32(std::popcount(mask)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		_mm256_maskstore_epi32(reinterpret_cast<int *>(output), active, selected);
	}
}
template <bool Packed, bool Finish>
__attribute__((target("avx2")))
void route(Frontier &queries, const std::vector<std::uint32_t> &current, const std::vector<std::uint32_t> &prefix, unsigned zeros) {
	std::size_t i = 0;
	auto middle = _mm256_set1_epi32(int(zeros));
	for (; i + 8 <= queries.left.size(); i += 8) {
		auto left = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.left.data() + i));
		auto right = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.right.data() + i));
		auto rank = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(queries.rank.data() + i));
		auto a = _mm256_i32gather_epi32(reinterpret_cast<const int *>(prefix.data()), Packed ? _mm256_srli_epi32(left, 2) : left, 4);
		auto b = _mm256_i32gather_epi32(reinterpret_cast<const int *>(prefix.data()), Packed ? _mm256_srli_epi32(right, 2) : right, 4);
		if constexpr (Packed) {
			auto mask = _mm256_set1_epi32(3);
			auto sa = _mm256_slli_epi32(_mm256_and_si256(left, mask), 1), sb = _mm256_slli_epi32(_mm256_and_si256(right, mask), 1);
			a = _mm256_add_epi32(_mm256_srli_epi32(a, 8), _mm256_and_si256(_mm256_srlv_epi32(a, sa), mask));
			b = _mm256_add_epi32(_mm256_srli_epi32(b, 8), _mm256_and_si256(_mm256_srlv_epi32(b, sb), mask));
		}
		auto lzero = _mm256_sub_epi32(left, a), rzero = _mm256_sub_epi32(right, b);
		auto count = _mm256_sub_epi32(rzero, lzero), take_zero = _mm256_cmpgt_epi32(count, rank);
		if constexpr (Finish) {
			auto value = _mm256_i32gather_epi32(reinterpret_cast<const int *>(current.data()), left, 4);
			value = _mm256_or_si256(_mm256_and_si256(value, _mm256_set1_epi32(-2)), _mm256_andnot_si256(take_zero, _mm256_set1_epi32(1)));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.answer.data() + i), value);
		} else {
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.left.data() + i), _mm256_blendv_epi8(_mm256_add_epi32(middle, a), lzero, take_zero));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.right.data() + i), _mm256_blendv_epi8(_mm256_add_epi32(middle, b), rzero, take_zero));
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(queries.rank.data() + i), _mm256_sub_epi32(rank, _mm256_andnot_si256(take_zero, count)));
		}
	}
	for (; i < queries.left.size(); ++i) {
		auto left = queries.left[i], right = queries.right[i];
		auto a = prefix[Packed ? left >> 2 : left], b = prefix[Packed ? right >> 2 : right];
		if constexpr (Packed) {
			a = (a >> 8) + ((a >> (2 * (left & 3))) & 3);
			b = (b >> 8) + ((b >> (2 * (right & 3))) & 3);
		}
		auto count = right - left - b + a;
		bool one = queries.rank[i] >= count;
		if constexpr (Finish) queries.answer[i] = (current[left] & ~1U) | unsigned(one);
		else {
			queries.left[i] = one ? zeros + a : left - a;
			queries.right[i] = one ? zeros + b : right - b;
			queries.rank[i] -= one ? count : 0;
		}
	}
}
template <bool Packed>
__attribute__((target("avx2")))
void run_avx2(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
	std::size_t n = current.size();
	current.resize(n + 8);
	std::vector<std::uint32_t> next(n + 8), prefix(Packed ? (n + 3) / 4 + 1 : n + 1);
	std::vector<unsigned char> masks((n + 7) / 8);
	while (bits--) {
		unsigned ones = 0;
		std::size_t i = 0;
		for (; i + 8 <= n; i += 8) {
			auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(current.data() + i));
			auto selected = _mm256_sll_epi32(value, _mm_cvtsi32_si128(31 - int(bits)));
			unsigned mask = unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(selected)));
			masks[i / 8] = static_cast<unsigned char>(mask);
			if constexpr (Packed) {
				prefix[i / 4] = (ones << 8) + tables[mask].packed[0];
				prefix[i / 4 + 1] = (ones << 8) + tables[mask].packed[1];
			} else {
				auto local = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(tables[mask].prefix.data())));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(prefix.data() + i + 1), _mm256_add_epi32(local, _mm256_set1_epi32(int(ones))));
			}
			ones += unsigned(std::popcount(mask));
		}
		for (; i <= n; ++i) {
			if constexpr (Packed) {
				if (!(i & 3)) prefix[i / 4] = ones << 8;
				else prefix[i / 4] |= (ones - (prefix[i / 4] >> 8)) << (2 * (i & 3));
			} else prefix[i] = ones;
			if (i < n) ones += (current[i] >> bits) & 1;
		}
		unsigned zeros = unsigned(n) - ones;
		if (bits) {
			unsigned zero = 0, one = zeros;
			i = 0;
			for (; i + 8 <= n; i += 8) {
				auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(current.data() + i));
				unsigned mask = masks[i / 8], count = unsigned(std::popcount(mask));
				store_selected(next.data() + zero, zeros - zero, value, mask ^ 255);
				store_selected(next.data() + one, unsigned(n + 8) - one, value, mask);
				zero += 8 - count; one += count;
			}
			for (; i < n; ++i) {
				auto value = current[i];
				if ((value >> bits) & 1) next[one++] = value;
				else next[zero++] = value;
			}
			route<Packed, false>(queries, current, prefix, zeros);
			current.swap(next);
		} else route<Packed, true>(queries, current, prefix, zeros);
	}
}
#endif
inline void run(Frontier &queries, std::vector<std::uint32_t> current, unsigned bits) {
#if defined(__x86_64__) || defined(__i386__)
	if (__builtin_cpu_supports("avx2")) {
		if (packed_ranks(queries, current.size())) run_avx2<true>(queries, std::move(current), bits);
		else run_avx2<false>(queries, std::move(current), bits);
		return;
	}
#endif
	run_scalar(queries, std::move(current), bits);
}
}


#line 1 "components/wavelet_compression.hpp"



#include <algorithm>
#line 7 "components/wavelet_compression.hpp"
#include <cassert>
#line 9 "components/wavelet_compression.hpp"
#include <limits>
#include <type_traits>
#line 12 "components/wavelet_compression.hpp"

namespace wavelet_detail {
template<class T>
std::uint32_t lower_code(const std::vector<T> &values, bool dense, const T &x) {
	if (values.empty()) return 0;
	if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
		if (dense) {
			if (x <= values.front()) return 0;
			if (values.back() < x) return static_cast<int>(values.size());
			return std::uint64_t(x) - std::uint64_t(values.front());
		}
	int p = 0, length = static_cast<int>(values.size());
	while (length > 1) {
		int half = length / 2;
		p += (values[p + half] < x) * half;
		length -= half;
	}
	return p + (values[p] < x);
}

template<class T, bool WithPositions = true>
std::vector<std::uint32_t> compress(const std::vector<T> &a, std::vector<T> &values,
	int &ordered, std::vector<int> &positions, std::vector<int> &offsets) {
	assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(a.size());
	ordered = 0;
	if constexpr (WithPositions) offsets.clear();
	std::vector<std::uint32_t> current(n);
	bool ascending = std::is_sorted(a.begin(), a.end());
	if (ascending || std::is_sorted(a.rbegin(), a.rend())) {
		ordered = ascending ? 1 : -1;
		values.clear();
		values.reserve(n);
		for (int i = 0; i < n; ++i) {
			int j = ascending ? i : n - 1 - i;
			if (values.empty() || values.back() != a[j]) values.push_back(a[j]);
			current[j] = static_cast<int>(values.size()) - 1;
		}
		if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
		if constexpr (WithPositions) positions.clear();
		return current;
	}
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
		if (n >= 1024) {
			using U = std::make_unsigned_t<T>;
			struct [[gnu::packed]] Item {
				U key;
				std::uint32_t index;
			};
			std::vector<Item> order(n);
			U lo = std::numeric_limits<U>::max(), hi = 0;
			constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
			for (int i = 0; i < n; ++i) {
				U key = U(a[i]) ^ sign;
				order[i] = {key, std::uint32_t(i)};
				lo = std::min(lo, key);
				hi = std::max(hi, key);
			}
			if (hi - lo < 65536 && std::uint64_t(hi - lo) < unsigned(n)) {
				std::vector<int> codes(std::size_t(hi - lo) + 1, -1);
				for (const auto &v : order) codes[v.key - lo] = 0;
				values.clear();
				values.reserve(codes.size());
				for (std::size_t i = 0; i < codes.size(); ++i)
					if (codes[i] == 0) {
						codes[i] = static_cast<int>(values.size());
						values.push_back(T(U(lo + U(i)) ^ sign));
					}
				for (const auto &v : order) current[v.index] = codes[v.key - lo];
				if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
				if constexpr (WithPositions) positions.clear();
				return current;
			}
			std::vector<Item> next(n);
			int width = std::bit_width(U(lo ^ hi)), span_width = std::bit_width(U(hi - lo));
			if ((width + 10) / 11 > (span_width + 10) / 11) {
				for (auto &v : order) v.key -= lo;
				width = span_width;
			}
			for (int shift = 0; shift < width; shift += 11) {
				std::array<int, 2048> offsets{};
				for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
				int total = 0;
				for (int &c : offsets) {
					int t = c;
					c = total;
					total += t;
				}
				for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
				order.swap(next);
			}
			values.clear();
			values.reserve(n);
			U previous = 0;
			if constexpr (WithPositions) {
				positions.resize(n);
				offsets.reserve(std::size_t(n) + 1);
			}
			for (int j = 0; j < n; ++j) {
				const auto &v = order[j];
				if (values.empty() || v.key != previous) {
					values.push_back(a[v.index]);
					if constexpr (WithPositions) offsets.push_back(j);
				}
				previous = v.key;
				current[v.index] = static_cast<int>(values.size()) - 1;
				if constexpr (WithPositions) positions[j] = v.index;
			}
			if constexpr (WithPositions) offsets.push_back(n);
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			return current;
		}
	}
	values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
	for (int i = 0; i < n; ++i) current[i] = lower_code<T>(values, false, a[i]);
	if constexpr (WithPositions) positions.clear();
	return current;
}

template<class T> struct ValueIndex {
	int n = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = static_cast<int>(a.size());
		int ordered = 0;
		std::vector<int> positions, offsets;
		original = compress<T, false>(a, values, ordered, positions, offsets);
	}
	std::uint32_t code(const T &x) const { return lower_code(values, false, x); }
};
}


#line 9 "data_structure/offline_range_kth_smallest.hpp"
#include <span>
#line 11 "data_structure/offline_range_kth_smallest.hpp"

template <class T>
std::vector<T> offline_range_kth_smallest(const std::vector<T> &values, std::span<const std::array<int, 3>> queries) {
	assert(values.size() <= std::size_t(std::numeric_limits<int>::max()) && queries.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (queries.empty()) return {};
	offline_range_kth_detail::Frontier frontier(queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto [l, r, k] = queries[i];
		assert(0 <= l && l < r && std::size_t(r) <= values.size() && 0 <= k && k < r - l);
		frontier.left[i] = unsigned(l); frontier.right[i] = unsigned(r); frontier.rank[i] = unsigned(k);
	}
	std::vector<T> sorted;
	std::vector<int> positions, offsets;
	int ordered = 0;
	auto codes = wavelet_detail::compress<T, false>(values, sorted, ordered, positions, offsets);
	unsigned bits = std::bit_width(unsigned(sorted.size() - 1));
	if (bits && !ordered) offline_range_kth_detail::run(frontier, std::move(codes), bits);
	std::vector<T> result;
	result.reserve(queries.size());
	for (std::size_t i = 0; i < queries.size(); ++i) {
		if (ordered) {
			auto [l, r, k] = queries[i];
			result.push_back(values[ordered > 0 ? l + k : r - 1 - k]);
		} else result.push_back(sorted[frontier.answer[i]]);
	}
	return result;
}


#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
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


#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 8 "components/fast_io_bulk.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		if (a < 1 || a > 10 || b <= a + 1 || b - a - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		const auto window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	static std::uint32_t one(const char *&p, bool &bad) {
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		bad |= value > 0xffffffffULL;
		p += n + 1;
		return std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				const auto g0 = two(p[0], invalid), g1 = two(p[1], invalid), g2 = two(p[2], invalid), g3 = two(p[3], invalid);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, _mm256_set1_epi32(42)),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, _mm256_set1_epi32(42)), _mm256_cmpgt_epi32(low, _mm256_set1_epi32(94967295))));
				invalid = _mm256_or_si256(invalid, overflow);
				const auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, std::size_t... I>
void emit_record(Emit &emit, const std::uint32_t *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 6 "test/offline_range_kth_smallest.test.cpp"

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	uint32_t n = 0, q = 0;
	input.read(n); input.read(q);
	vector<uint32_t> a(n);
	read_unsigned(input, a);
	vector<array<int, 3>> queries(q);
	std::size_t position = 0;
	read_unsigned_records<3>(input, q, [&](std::uint32_t l, std::uint32_t r, std::uint32_t k) { queries[position++] = {int(l), int(r), int(k)}; });
	for (auto answer : offline_range_kth_smallest(a, queries)) output.println(answer);
}
