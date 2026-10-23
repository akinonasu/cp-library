#line 1 "test/li_chao_tree_line.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"
#line 1 "data_structure/li_chao_tree/li_chao_tree.hpp"



#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, bool maximize = false, class R = std::conditional_t<std::is_integral_v<T>, std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>, T>>
class LiChaoTree {
	static_assert((std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8) || std::is_floating_point_v<T>);
	struct Line {
		T a, b;
		template <class V>
		V eval(T x) const { return V(R(a) * R(x) + R(b)); }
	};
	int n = 0, base = 1;
	bool wide = false, dense = false;
	std::vector<T> xs;
	std::vector<Line> lines;
	std::vector<std::uint8_t> state;
	template <class V>
	static bool better(V a, V b) {
		if constexpr (maximize) return a > b;
		else return a < b;
	}
	template <class V>
	void insert(int k, int l, int width, Line f) {
		for (;;) {
			Line &g = lines[k];
			if (!(state[k] & 1)) {
				g = f;
				state[k] = 3;
				return;
			}
			if (f.a == g.a) {
				if (better(f.b, g.b)) g = f;
				return;
			}
			int half = width >> 1, m = l + half;
			if (better(f.template eval<V>(xs[m]), g.template eval<V>(xs[m]))) std::swap(f, g);
			if (!half) return;
			bool right = better(f.a, g.a);
			int edge = right ? l + width - 1 : l;
			if (!better(f.template eval<V>(xs[edge]), g.template eval<V>(xs[edge]))) return;
			k = k * 2 + right;
			if (right) l = m;
			width = half;
		}
	}
	template <class V>
	void insert_segment(int k, int l, int r, int ql, int qr, Line f) {
		if (ql <= l && r <= qr) {
			insert<V>(k, l, r - l, f);
			return;
		}
		if (state[k] & 1) {
			const Line &g = lines[k];
			if (f.a == g.a) {
				if (!better(f.b, g.b)) return;
			} else {
				int edge = better(f.a, g.a) ? std::min(r, qr) - 1 : std::max(l, ql);
				if (!better(f.template eval<V>(xs[edge]), g.template eval<V>(xs[edge]))) return;
			}
		}
		state[k] |= 2;
		int m = (l + r) >> 1;
		if (ql < m) insert_segment<V>(k * 2, l, m, ql, qr, f);
		if (m < qr) insert_segment<V>(k * 2 + 1, m, r, ql, qr, f);
	}

	void check(Line f, int l, int r) {
		if constexpr (std::is_integral_v<T> && std::is_same_v<R, __int128_t>) {
			if (!wide) {
				R a = f.template eval<R>(xs[l]), b = f.template eval<R>(xs[r - 1]);
				wide = a != R(std::int64_t(a)) || b != R(std::int64_t(b));
			}
		}
	}
	template <class F>
	decltype(auto) dispatch(F f) const {
		if constexpr (std::is_integral_v<T> && std::is_same_v<R, __int128_t>) {
			if (!wide) return f.template operator()<std::int64_t>();
		}
		return f.template operator()<R>();
	}

	int lower(T x) const {
		if (n <= 1) return n && xs[0] < x;
		if constexpr (std::is_integral_v<T>)
			if (dense) return int(std::clamp<__int128_t>(__int128_t(x) - xs[0], 0, n));
		int i = 0;
		for (int step = base >> 1; step; step >>= 1) i += int(xs[i + step - 1] < x) * step;
		return std::min(n, i + int(xs[i] < x));
	}

	template <class V>
	std::optional<R> query_impl(int i) const {
		assert(0 <= i && i < n);
		V result = maximize ? std::numeric_limits<V>::lowest() : std::numeric_limits<V>::max();
		bool found = false;
		for (int k = 1, bit = base >> 1;; bit >>= 1) {
			if (!state[k]) break;
			if (state[k] & 1) {
				V value = lines[k].template eval<V>(xs[i]);
				if ((std::is_floating_point_v<V> && !found) || better(value, result)) result = value;
				found = true;
			}
			if (!bit) break;
			k = k * 2 + bool(i & bit);
		}
		if (found) return result;
		return std::nullopt;
	}

  public:
	using value_type = R;
	explicit LiChaoTree(std::vector<T> coordinates = {}) { build(std::move(coordinates)); }
	void build(std::vector<T> coordinates) {
		if (!std::is_sorted(coordinates.begin(), coordinates.end())) std::sort(coordinates.begin(), coordinates.end());
		coordinates.erase(std::unique(coordinates.begin(), coordinates.end()), coordinates.end());
		assert(coordinates.size() <= std::size_t(1) << 29);
		n = static_cast<int>(coordinates.size());
		if constexpr (std::is_integral_v<T>) dense = n && __int128_t(coordinates.back()) - coordinates[0] == n - 1;
		base = int(std::bit_ceil(unsigned(std::max(1, n))));
		xs = std::move(coordinates);
		if (n) xs.resize(base, xs.back());
		lines.resize(base * 2);
		state.assign(base * 2, 0);
		wide = false;
	}
	int size() const { return n; }
	void clear() {
		std::fill(state.begin(), state.end(), 0);
		wide = false;
	}
	T coordinate(int i) const {
		assert(0 <= i && i < n);
		return xs[i];
	}
	int index(T x) const {
		int i = lower(x);
		assert(i < n && xs[i] == x);
		return i;
	}
	void add_line(T a, T b) {
		if (n) {
			Line f{a, b};
			check(f, 0, n);
			dispatch([&]<class V>() { insert<V>(1, 0, base, f); });
		}
	}
	void add_segment_index(int l, int r, T a, T b) {
		assert(0 <= l && l <= r && r <= n);
		if (l < r) {
			Line f{a, b};
			check(f, l, r);
			dispatch([&]<class V>() { insert_segment<V>(1, 0, base, l, r == n ? base : r, f); });
		}
	}
	void add_segment(T l, T r, T a, T b) {
		assert(l <= r);
		int left = lower(l);
		int right = lower(r);
		add_segment_index(left, right, a, b);
	}
	std::optional<R> query_index(int i) const {
		return dispatch([&]<class V>() { return query_impl<V>(i); });
	}
	std::optional<R> query(T x) const { return query_index(index(x)); }
};


#line 1 "components/radix_sort.hpp"



#line 5 "components/radix_sort.hpp"
#include <array>
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


#line 7 "test/li_chao_tree_line.test.cpp"
#include <tuple>
#line 10 "test/li_chao_tree_line.test.cpp"

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	vector<pair<int64_t, int64_t>> initial(n);
	for (auto &[a, b] : initial) input.read(a), input.read(b);
	struct Query {
		int type, index;
		int64_t a, b;
	};
	vector<Query> queries(q);
	vector<uint64_t> points;
	points.reserve(q);
	for (auto &query : queries) {
		input.read(query.type), input.read(query.a);
		if (query.type == 0) input.read(query.b);
		else points.push_back((uint64_t(uint32_t(query.a) ^ (1U << 31)) << 32) | uint32_t(&query - queries.data()));
	}
	if (points.empty()) return 0;
	stable_radix_sort<11>(points, [](uint64_t point) { return uint32_t(point >> 32); });
	vector<int64_t> xs;
	xs.reserve(points.size());
	for (auto point : points) {
		int64_t value = bit_cast<int32_t>(uint32_t(point >> 32) ^ (1U << 31));
		if (xs.empty() || xs.back() != value) xs.push_back(value);
		queries[uint32_t(point)].index = int(xs.size()) - 1;
	}
	LiChaoTree<> tree(std::move(xs));
	for (auto [a, b] : initial) tree.add_line(a, b);
	for (auto query : queries) {
		if (query.type == 0) tree.add_line(query.a, query.b);
		else output.println(*tree.query_index(query.index));
	}
}
