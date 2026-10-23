#line 1 "test/range_add_range_min.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#line 1 "data_structure/segment_tree/range_add_range_min.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__AVX2__) && defined(__x86_64__)
#include <immintrin.h>
#endif

template<class T = std::int64_t> class RangeAddRangeMin {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	static constexpr unsigned branch = 16;
	static constexpr T infinity = std::numeric_limits<T>::max();
	std::vector<T> data_;
	std::vector<U> lazy_;
	std::array<std::size_t, (sizeof(int) * 8 + 3) / 4 + 1> offsets_{};
	int n_ = 0;
	unsigned height_ = 0;
	std::size_t width_ = 1, leaves_ = 0;
	static T added(T value, U delta) { return std::bit_cast<T>(U(U(value) + delta)); }
#if defined(__AVX2__) && defined(__x86_64__)
	static __m256i minimum(__m256i a, __m256i b) { return _mm256_blendv_epi8(a, b, _mm256_cmpgt_epi64(a, b)); }
#endif
	static T minimum(const T *values, unsigned l, unsigned r) {
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			if (r - l > 3) {
				const auto inf = _mm256_set1_epi64x(infinity);
				auto result = inf;
				const auto low = _mm256_set1_epi64x(static_cast<long long>(l) - 1), high = _mm256_set1_epi64x(r);
				for (unsigned i = 0; i < branch; i += 4) {
					auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + i));
					if (l > i || r < i + 4) {
						const auto indices = _mm256_setr_epi64x(i, i + 1, i + 2, i + 3);
						const auto active = _mm256_and_si256(_mm256_cmpgt_epi64(indices, low), _mm256_cmpgt_epi64(high, indices));
						value = _mm256_blendv_epi8(inf, value, active);
					}
					result = minimum(result, value);
				}
				alignas(32) T lanes[4];
				_mm256_store_si256(reinterpret_cast<__m256i *>(lanes), result);
				return std::min(std::min(lanes[0], lanes[1]), std::min(lanes[2], lanes[3]));
			}
		}
#endif
		T result = infinity;
		for (unsigned i = l; i < r; ++i) result = std::min(result, values[i]);
		return result;
	}
	void pull(std::size_t k, std::size_t children) { data_[k] = minimum(data_.data() + children, 0, branch); }
	void add_children(std::size_t p, unsigned l, unsigned r, U delta) {
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			if (r - l > 3) {
				const auto value = _mm256_set1_epi64x(std::bit_cast<T>(delta));
				const auto low = _mm256_set1_epi64x(static_cast<long long>(l) - 1), high = _mm256_set1_epi64x(r);
				for (unsigned i = 0; i < branch; i += 4) {
					if (i + 4 <= l || r <= i) continue;
					auto change = value;
					if (l > i || r < i + 4) {
						const auto indices = _mm256_setr_epi64x(i, i + 1, i + 2, i + 3);
						change = _mm256_and_si256(value, _mm256_and_si256(_mm256_cmpgt_epi64(indices, low), _mm256_cmpgt_epi64(high, indices)));
					}
					auto *data = reinterpret_cast<__m256i *>(data_.data() + p + i);
					_mm256_storeu_si256(data, _mm256_add_epi64(_mm256_loadu_si256(data), change));
					if (p >= leaves_) {
						auto *tags = reinterpret_cast<__m256i *>(lazy_.data() + p - leaves_ + i);
						_mm256_storeu_si256(tags, _mm256_add_epi64(_mm256_loadu_si256(tags), change));
					}
				}
				return;
			}
		}
#endif
		for (unsigned i = l; i < r; ++i) data_[p + i] = added(data_[p + i], delta);
		if (p >= leaves_) for (unsigned i = l; i < r; ++i) lazy_[p - leaves_ + i] += delta;
	}
	void push(std::size_t k, std::size_t children) {
		U delta = lazy_[k - leaves_];
		if (!delta) return;
		add_children(children, 0, branch, delta);
		lazy_[k - leaves_] = 0;
	}
	void update(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t ql, std::size_t qr, U delta) {
		std::size_t k = offsets_[level] + index;
		if (ql <= l && l + width <= qr) {
			data_[k] = added(data_[k], delta);
			if (level) lazy_[k - leaves_] += delta;
			return;
		}
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		push(k, p);
		unsigned shift = 4 * (level - 1);
		unsigned a = ql <= l ? 0 : unsigned((ql - l) >> shift);
		unsigned b = qr >= l + width ? branch : unsigned(((qr - l - 1) >> shift) + 1);
		if (a + 1 == b) update(level - 1, index * branch + a, l + a * part, part, ql, qr, delta);
		else {
			if (ql > l + a * part) { update(level - 1, index * branch + a, l + a * part, part, ql, qr, delta); ++a; }
			if (qr < l + b * part) { --b; update(level - 1, index * branch + b, l + b * part, part, ql, qr, delta); }
			add_children(p, a, b, delta);
		}
		pull(k, p);
	}
	T query(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t ql, std::size_t qr, U carry) const {
		std::size_t k = offsets_[level] + index;
		if (ql <= l && l + width <= qr) return added(data_[k], carry);
		carry += lazy_[k - leaves_];
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		unsigned shift = 4 * (level - 1);
		unsigned a = ql <= l ? 0 : unsigned((ql - l) >> shift);
		unsigned b = qr >= l + width ? branch : unsigned(((qr - l - 1) >> shift) + 1);
		if (a + 1 == b) return query(level - 1, index * branch + a, l + a * part, part, ql, qr, carry);
		T result = infinity;
		if (ql > l + a * part) { result = query(level - 1, index * branch + a, l + a * part, part, ql, qr, carry); ++a; }
		if (qr < l + b * part) { --b; result = std::min(result, query(level - 1, index * branch + b, l + b * part, part, ql, qr, carry)); }
		if (a < b) result = std::min(result, added(minimum(data_.data() + p, a, b), carry));
		return result;
	}
	void assign(unsigned level, std::size_t index, std::size_t l, std::size_t width, std::size_t pos, T value) {
		std::size_t k = offsets_[level] + index;
		if (!level) { data_[k] = value; return; }
		std::size_t part = width / branch, p = offsets_[level - 1] + index * branch;
		push(k, p);
		unsigned child = unsigned((pos - l) >> (4 * (level - 1)));
		assign(level - 1, index * branch + child, l + child * part, part, pos, value);
		pull(k, p);
	}
	void collect(unsigned level, std::size_t index, std::size_t l, std::size_t width, U carry, std::vector<T> &result) const {
		std::size_t k = offsets_[level] + index;
		if (!level) { result[l] = added(data_[k], carry); return; }
		carry += lazy_[k - leaves_];
		std::size_t part = width / branch;
		for (unsigned child = 0; child < branch && l + child * part < std::size_t(n_); ++child)
			collect(level - 1, index * branch + child, l + child * part, part, carry, result);
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }

  public:
	using value_type = T;
	RangeAddRangeMin() = default;
	explicit RangeAddRangeMin(int n, T value = 0) { assert(n >= 0); build(std::vector<T>(n, value)); }
	explicit RangeAddRangeMin(const std::vector<T> &values) { build(values); }
	RangeAddRangeMin(const RangeAddRangeMin &) = default;
	RangeAddRangeMin(RangeAddRangeMin &&other) noexcept { swap(other); }
	RangeAddRangeMin &operator=(RangeAddRangeMin other) noexcept { swap(other); return *this; }
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		RangeAddRangeMin next;
		next.n_ = int(values.size());
		decltype(offsets_) counts{};
		std::size_t total = 0, real = std::max(std::size_t(1), values.size());
		for (unsigned level = 0;; ++level) {
			next.offsets_[level] = total;
			counts[level] = real;
			std::size_t padded = real == 1 ? 1 : (real + branch - 1) / branch * branch;
			total += padded;
			if (!level) next.leaves_ = padded;
			if (real == 1) { next.height_ = level; break; }
			real = (real + branch - 1) / branch;
			if (next.width_ > std::numeric_limits<std::size_t>::max() / branch) throw std::length_error("RangeAddRangeMin size");
			next.width_ *= branch;
		}
		next.data_.assign(total, infinity);
		next.lazy_.resize(total - next.leaves_);
		std::copy(values.begin(), values.end(), next.data_.begin());
		for (unsigned level = 1; level <= next.height_; ++level)
			for (std::size_t index = 0; index < counts[level]; ++index)
				next.pull(next.offsets_[level] + index, next.offsets_[level - 1] + index * branch);
		swap(next);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	void add(int l, int r, T value) { check(l, r); if (l < r && value) update(height_, 0, 0, width_, l, r, U(value)); }
	void apply(int l, int r, T value) { add(l, r, value); }
	T prod(int l, int r) const { check(l, r); return l < r ? query(height_, 0, 0, width_, l, r, 0) : infinity; }
	T min(int l, int r) const { return prod(l, r); }
	T all_prod() const { return n_ ? data_[offsets_[height_]] : infinity; }
	T all_min() const { return all_prod(); }
	T get(int p) const { assert(0 <= p && p < n_); return prod(p, p + 1); }
	void set(int p, T value) { assert(0 <= p && p < n_); assign(height_, 0, 0, width_, p, value); }
	std::vector<T> values() const { std::vector<T> result(n_); if (n_) collect(height_, 0, 0, width_, 0, result); return result; }
	std::size_t memory_usage() const { return data_.capacity() * sizeof(T) + lazy_.capacity() * sizeof(U); }
	void swap(RangeAddRangeMin &other) noexcept {
		data_.swap(other.data_), lazy_.swap(other.lazy_), offsets_.swap(other.offsets_);
		std::swap(n_, other.n_), std::swap(height_, other.height_), std::swap(width_, other.width_), std::swap(leaves_, other.leaves_);
	}
	friend void swap(RangeAddRangeMin &a, RangeAddRangeMin &b) noexcept { a.swap(b); }
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 15 "components/fast_io.hpp"
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


#line 6 "test/range_add_range_min.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> values(n);
	for (auto &value : values) input.read(value);
	RangeAddRangeMin tree(values);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (type == 0) { std::int64_t value = 0; input.read(value); tree.add(l, r, value); }
		else output.println(tree.prod(l, r));
	}
}
