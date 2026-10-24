#line 1 "test/predecessor_set_priority_queue.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#line 1 "components/predecessor_set.hpp"



#line 1 "components/xor_select.hpp"



#include <bit>
#include <cassert>
#include <cstdint>

namespace xor_select_detail {
inline unsigned argmin(std::uint64_t word, unsigned key) {
	assert(word && key < 64);
	if (std::has_single_bit(word)) return std::countr_zero(word);
	if ((word >> key) & 1) return key;
	unsigned shift = key & 32;
	word = std::rotl(word, shift);
	shift = key & 16;
	word = ((word & 0x0000ffff0000ffffULL) << shift) | ((word & 0xffff0000ffff0000ULL) >> shift);
	shift = key & 8;
	word = ((word & 0x00ff00ff00ff00ffULL) << shift) | ((word & 0xff00ff00ff00ff00ULL) >> shift);
	shift = key & 4;
	word = ((word & 0x0f0f0f0f0f0f0f0fULL) << shift) | ((word & 0xf0f0f0f0f0f0f0f0ULL) >> shift);
	shift = key & 2;
	word = ((word & 0x3333333333333333ULL) << shift) | ((word & 0xccccccccccccccccULL) >> shift);
	shift = key & 1;
	word = ((word & 0x5555555555555555ULL) << shift) | ((word & 0xaaaaaaaaaaaaaaaaULL) >> shift);
	return std::countr_zero(word) ^ key;
}
}


#line 5 "components/predecessor_set.hpp"
#include <algorithm>
#include <array>
#line 9 "components/predecessor_set.hpp"
#include <cstddef>
#line 11 "components/predecessor_set.hpp"
#include <cstring>
#include <limits>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace predecessor_set_detail {
inline void pack_scalar(const char *src, std::uint64_t *dst, std::size_t words) {
	for (std::size_t i = 0; i < words; ++i) {
		std::uint64_t word = 0;
		if constexpr (std::endian::native == std::endian::little) {
			for (int j = 0; j < 8; ++j) {
				std::uint64_t x;
				std::memcpy(&x, src + i * 64 + j * 8, 8);
				assert((x & 0xfefefefefefefefeULL) == 0x3030303030303030ULL);
				word |= ((x & 0x0101010101010101ULL) * 0x0102040810204080ULL >> 56) << (j * 8);
			}
		} else {
			for (int j = 0; j < 64; ++j) {
				assert(src[i * 64 + j] == '0' || src[i * 64 + j] == '1');
				word |= std::uint64_t(src[i * 64 + j] - '0') << j;
			}
		}
		dst[i] = word;
	}
}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((target("avx2"))) inline void pack_avx2(const char *src, std::uint64_t *dst, std::size_t words) {
	const auto ones = _mm256_set1_epi8('1');
	for (std::size_t i = 0; i < words; ++i) {
		auto a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64));
		auto b = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * 64 + 32));
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(a, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		assert(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_and_si256(b, _mm256_set1_epi8(-2)), _mm256_set1_epi8('0'))) == -1);
		auto low = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(a, ones)));
		auto high = std::uint32_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(b, ones)));
		dst[i] = low | (std::uint64_t(high) << 32);
	}
}
#endif
inline void pack(const char *src, std::uint64_t *dst, std::size_t words) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (words >= 4 && __builtin_cpu_supports("avx2")) return pack_avx2(src, dst, words);
#endif
	pack_scalar(src, dst, words);
}
}

class PredecessorSet {
	int n = 0, count = 0, height = 0;
	std::array<int, 6> offset{}, length{};
	std::vector<std::uint64_t> data;
	void mask_tail() {
		int bits = n;
		for (int h = 0; h < height; ++h) {
			if (bits & 63) data[offset[h] + length[h] - 1] &= ~std::uint64_t(0) >> (64 - (bits & 63));
			bits = length[h];
		}
	}
	void rebuild() {
		count = 0;
		for (int i = 0; i < length[0]; ++i) count += std::popcount(data[i]);
		for (int h = 1; h < height; ++h)
			for (int i = 0; i < length[h - 1]; ++i)
				if (data[offset[h - 1] + i]) data[offset[h] + (i >> 6)] |= std::uint64_t(1) << (i & 63);
	}

public:
	int argmin_xor(std::uint32_t x) const {
		if (!count) return -1;
		std::uint64_t p = std::uint64_t(x) & ((std::uint64_t(1) << (height * 6)) - 1);
		for (int h = 0; h < height; ++h, p >>= 6) {
			auto index = p >> 6;
			if (index >= unsigned(length[h])) continue;
			auto word = data[offset[h] + index];
			if (!word) continue;
			p = index * 64 + xor_select_detail::argmin(word, p & 63);
			while (h--) p = p * 64 + xor_select_detail::argmin(data[offset[h] + p], (x >> (h * 6)) & 63);
			return int(p);
		}
		return -1;
	}
	int argmax_xor(std::uint32_t x) const { return argmin_xor(~x); }
	explicit PredecessorSet(int n = 0, bool filled = false) { assign(n, filled); }
	explicit PredecessorSet(std::string_view bits) { assign(bits); }
	PredecessorSet(int n, std::span<const std::uint64_t> words) { assign(n, words); }
	PredecessorSet(const PredecessorSet &) = default;
	PredecessorSet &operator=(const PredecessorSet &) = default;
	PredecessorSet(PredecessorSet &&other) noexcept : PredecessorSet() { swap(other); }
	PredecessorSet &operator=(PredecessorSet &&other) noexcept {
		if (this != &other) { PredecessorSet moved(std::move(other)); swap(moved); }
		return *this;
	}
	void assign(int length, bool filled = false) {
		assert(length >= 0);
		n = length; count = filled ? n : 0; height = 0;
		offset.fill(0); this->length.fill(0);
		int total = 0;
		for (std::size_t bits = n; bits;) {
			std::size_t words = (bits + 63) / 64;
			offset[height] = total; this->length[height++] = words;
			total += words;
			if (words == 1) break;
			bits = words;
		}
		data.assign(total, filled ? ~std::uint64_t(0) : 0);
		if (filled) mask_tail();
	}
	void assign(std::string_view bits) {
		assert(bits.size() <= std::size_t(std::numeric_limits<int>::max()));
		PredecessorSet next(static_cast<int>(bits.size()));
		std::size_t words = bits.size() / 64;
		predecessor_set_detail::pack(bits.data(), next.data.data(), words);
		for (std::size_t i = words * 64; i < bits.size(); ++i) {
			assert(bits[i] == '0' || bits[i] == '1');
			next.data[i >> 6] |= std::uint64_t(bits[i] - '0') << (i & 63);
		}
		next.rebuild(); swap(next);
	}
	void assign(int n, std::span<const std::uint64_t> words) {
		assert(n >= 0 && words.size() == (std::size_t(n) + 63) / 64);
		PredecessorSet next(n);
		std::copy(words.begin(), words.end(), next.data.begin());
		if (n & 63) next.data[next.length[0] - 1] &= ~std::uint64_t(0) >> (64 - (n & 63));
		next.rebuild(); swap(next);
	}
	int universe_size() const { return n; }
	int size() const { return count; }
	bool empty() const { return count == 0; }
	bool contains(int p) const { assert(0 <= p && p < n); return data[p >> 6] >> (p & 63) & 1; }
	bool insert(int p) {
		assert(0 <= p && p < n);
		if (contains(p)) return false;
		++count;
		for (int h = 0; h < height; ++h) {
			auto &word = data[offset[h] + (p >> 6)];
			auto before = word;
			word |= std::uint64_t(1) << (p & 63);
			if (before) break;
			p >>= 6;
		}
		return true;
	}
	bool erase(int p) {
		assert(0 <= p && p < n);
		if (!contains(p)) return false;
		--count;
		for (int h = 0; h < height; ++h) {
			auto &word = data[offset[h] + (p >> 6)];
			word &= ~(std::uint64_t(1) << (p & 63));
			if (word) break;
			p >>= 6;
		}
		return true;
	}
	int next(int p) const {
		p = std::max(p, 0);
		if (p >= n || !count) return n;
		for (int h = 0; h < height; ++h) {
			if ((p >> 6) >= length[h]) return n;
			auto word = data[offset[h] + (p >> 6)] >> (p & 63);
			if (word) {
				p += std::countr_zero(word);
				while (h--) p = p * 64 + std::countr_zero(data[offset[h] + p]);
				return p;
			}
			p = (p >> 6) + 1;
		}
		return n;
	}
	int prev(int p) const {
		p = std::min(p, n - 1);
		if (p < 0 || !count) return -1;
		for (int h = 0; h < height; ++h) {
			auto word = data[offset[h] + (p >> 6)] << (63 - (p & 63));
			if (word) {
				p -= std::countl_zero(word);
				while (h--) p = p * 64 + 63 - std::countl_zero(data[offset[h] + p]);
				return p;
			}
			p = (p >> 6) - 1;
			if (p < 0) return -1;
		}
		return -1;
	}
	bool set(int p, bool present) { return present ? insert(p) : erase(p); }
	int min() const { return next(0); }
	int max() const { return prev(n - 1); }
	void clear() { std::fill(data.begin(), data.end(), 0); count = 0; }
	void fill() {
		std::fill(data.begin(), data.end(), ~std::uint64_t(0));
		mask_tail();
		count = n;
	}
	template <class F> void for_each(int l, int r, F &&f) const {
		assert(0 <= l && l <= r && r <= n);
		for (int p = next(l); p < r;) {
			int base = p & ~63;
			auto word = data[p >> 6] & (~std::uint64_t(0) << (p & 63));
			if (r - base < 64) word &= ~std::uint64_t(0) >> (64 - (r - base));
			while (word) { f(base + std::countr_zero(word)); word &= word - 1; }
			if (r - base <= 64) break;
			p = next(base + 64);
		}
	}
	template <class F> void for_each(F &&f) const { for_each(0, n, f); }
	std::vector<int> to_vector() const {
		std::vector<int> out; out.reserve(count);
		for_each([&](int x) { out.push_back(x); }); return out;
	}
	std::span<const std::uint64_t> words() const & { return {data.data(), std::size_t(length[0])}; }
	void words() const && = delete;
	void swap(PredecessorSet &other) noexcept {
		using std::swap;
		swap(n, other.n); swap(count, other.count); swap(height, other.height);
		offset.swap(other.offset); length.swap(other.length); data.swap(other.data);
	}
	std::size_t memory_usage() const { return data.capacity() * sizeof(std::uint64_t); }
};


#line 1 "components/radix_sort.hpp"



#line 8 "components/radix_sort.hpp"
#include <type_traits>
#line 11 "components/radix_sort.hpp"

template <int Bits, class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key &&key) {
	static_assert(1 <= Bits && Bits <= 16);
	constexpr unsigned buckets = 1U << Bits, mask = buckets - 1;
	using K = std::remove_cvref_t<decltype(key(std::declval<const T &>()))>;
	static_assert(std::is_integral_v<K> && std::is_unsigned_v<K> && sizeof(K) <= sizeof(std::uint64_t));
	if (values.size() < 2) return;
	if (values.size() < 256) {
		std::stable_sort(values.begin(), values.end(), [&](const T &a, const T &b) { return key(a) < key(b); });
		return;
	}
	std::uint64_t first = key(values.front()), previous = first, varying = 0;
	bool sorted = true;
	for (std::size_t i = 1; i < values.size(); ++i) {
		std::uint64_t current = key(values[i]);
		varying |= first ^ current;
		sorted &= previous <= current;
		previous = current;
	}
	if (sorted) return;
	std::vector<T> scratch(values.size());
	using Counts = std::conditional_t<(Bits <= 11), std::array<std::size_t, buckets>, std::vector<std::size_t>>;
	Counts count = [] {
		if constexpr (Bits <= 11) return Counts{};
		else return Counts(buckets);
	}();
	for (int shift = 0; shift < 64; shift += Bits) {
		if (!((varying >> shift) & mask)) continue;
		std::fill(count.begin(), count.end(), 0);
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & mask]++] = std::move(value);
		values.swap(scratch);
	}
}

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
	stable_radix_sort<8>(values, std::move(key));
}


#line 1 "components/fast_io.hpp"



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


#line 7 "test/predecessor_set_priority_queue.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	struct Item { int value, time; };
	struct Query { int type, rank = 0; };
	std::vector<Item> items(n);
	for (auto &item : items) input.read(item.value), item.time = -1;
	std::vector<Query> queries(q);
	for (int i = 0; i < q; ++i) {
		input.read(queries[i].type);
		if (queries[i].type == 0) {
			int x = 0;
			input.read(x);
			items.push_back({x, i});
		}
	}
	stable_radix_sort<11>(items, [](const Item &item) { return std::uint32_t(item.value) ^ (1U << 31); });
	PredecessorSet values(int(items.size()));
	for (int i = 0; i < int(items.size()); ++i) {
		if (items[i].time < 0) values.insert(i);
		else queries[items[i].time].rank = i;
	}
	for (auto query : queries) {
		if (query.type == 0) values.insert(query.rank);
		else {
			int p = query.type == 1 ? values.min() : values.max();
			output.println(items[p].value);
			values.erase(p);
		}
	}
}
