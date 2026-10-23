#line 1 "test/offline_rectangle_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#line 1 "data_structure/offline_rectangle_sum.hpp"



#line 1 "components/fenwick_sum.hpp"



#include <cassert>
#include <bit>
#include <limits>
#include <vector>

template <class T>
class FenwickSum {
	std::vector<T> data;

  public:
	FenwickSum() = default;
	explicit FenwickSum(int n) { assert(n >= 0); data.resize(n); }
	explicit FenwickSum(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= static_cast<unsigned>(std::numeric_limits<int>::max()));
		data = a;
		for (unsigned i = 1; i <= data.size(); ++i) {
			unsigned p = i + (i & -i);
			if (p <= data.size()) data[p - 1] += data[i - 1];
		}
	}
	int size() const { return static_cast<int>(data.size()); }
	void add(int p, const T &x) {
		assert(0 <= p && p < size());
		for (unsigned i = unsigned(p) + 1; i <= data.size(); i += i & -i) data[i - 1] += x;
	}
	void add_pair(int p, const T &x, int q, const T &y) {
		assert(0 <= p && p < size() && 0 <= q && q < size());
		unsigned a = unsigned(p) + 1, b = unsigned(q) + 1;
		while (a != b && (a <= data.size() || b <= data.size())) {
			if (a < b) data[a - 1] += x, a += a & -a;
			else data[b - 1] += y, b += b & -b;
		}
		T both = x;
		both += y;
		for (; a <= data.size(); a += a & -a) data[a - 1] += both;
	}
	T prefix(int r) const {
		assert(0 <= r && r <= size());
		T result{};
		for (; r; r &= r - 1) result += data[r - 1];
		return result;
	}
	template <class Accum>
	Accum prod_as(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		Accum result{};
		if (l == r) return result;
		unsigned shared = unsigned(l) & (std::numeric_limits<unsigned>::max() << std::bit_width(unsigned(l ^ r)));
		for (; unsigned(l) != shared; l &= l - 1) result -= data[l - 1];
		for (; unsigned(r) != shared; r &= r - 1) result += data[r - 1];
		return result;
	}
	T prod(int l, int r) const { return prod_as<T>(l, r); }
};


#line 1 "components/radix_sort.hpp"



#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#line 11 "components/radix_sort.hpp"

template <class T, class Key>
void stable_radix_sort(std::vector<T> &values, Key key) {
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
	for (int shift = 0; shift < 64; shift += 8) {
		if (!((varying >> shift) & 255)) continue;
		std::array<std::size_t, 256> count{};
		for (const auto &value : values) ++count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255];
		std::size_t sum = 0;
		for (auto &x : count) {
			auto n = x;
			x = sum, sum += n;
		}
		for (auto &&value : values) scratch[count[(static_cast<std::uint64_t>(key(value)) >> shift) & 255]++] = std::move(value);
		values.swap(scratch);
	}
}


#line 12 "data_structure/offline_rectangle_sum.hpp"

template <class X = std::int64_t, class Y = X, class Sum = std::int64_t>
class OfflineRectangleSum {
	using Accum = std::conditional_t<std::is_integral_v<Sum> && sizeof(Sum) <= 8, __int128_t, Sum>;
	struct Point { X x; Y y; Sum weight; int rank = 0; };
	struct Query { X left, right; Y down, up; };
	struct Event { X x; int down, up, query; bool subtract; };
	std::vector<Point> points;
	std::vector<Query> queries;
	template <class V, class Key>
	static void sort(std::vector<V> &a, Key key) {
		using K = std::remove_cvref_t<decltype(key(a.front()))>;
		if constexpr (std::is_integral_v<K> && !std::is_same_v<K, bool> && sizeof(K) <= 8 && std::is_default_constructible_v<V>) {
			using U = std::make_unsigned_t<K>;
			constexpr U sign = std::is_signed_v<K> ? U(1) << (sizeof(K) * 8 - 1) : U(0);
			stable_radix_sort(a, [&](const V &v) { return U(U(key(v)) ^ sign); });
		} else std::sort(a.begin(), a.end(), [&](const V &a, const V &b) { return key(a) < key(b); });
	}
	template <class A>
	static void sweep(const std::vector<Point> &ordered, const std::vector<Event> &events, int size, std::vector<Accum> &answers) {
		FenwickSum<A> tree(size);
		std::size_t p = 0;
		for (const auto &event : events) {
			while (p < ordered.size() && ordered[p].x < event.x) {
				tree.add(ordered[p].rank, A(ordered[p].weight));
				++p;
			}
			auto value = tree.template prod_as<Accum>(event.down, event.up);
			if (event.subtract) answers[event.query] -= value;
			else answers[event.query] += value;
		}
	}

public:
	explicit OfflineRectangleSum(int point_capacity = 0, int query_capacity = 0) {
		assert(point_capacity >= 0 && query_capacity >= 0);
		points.reserve(point_capacity); queries.reserve(query_capacity);
	}
	void add_point(const X &x, const Y &y, const Sum &weight) {
		assert(points.size() < std::size_t(std::numeric_limits<int>::max()));
		points.push_back({x, y, weight});
	}
	int add_query(const X &left, const Y &down, const X &right, const Y &up) {
		assert(queries.size() < std::size_t(std::numeric_limits<int>::max()));
		queries.push_back({left, right, down, up});
		return int(queries.size() - 1);
	}
	std::vector<Sum> solve() const {
		if (points.empty() || queries.empty()) return std::vector<Sum>(queries.size());
		std::vector<Accum> answers(queries.size());
		auto ordered = points;
		sort(ordered, [](const Point &p) -> const Y & { return p.y; });
		std::vector<Y> ys;
		ys.reserve(points.size());
		for (auto &p : ordered) {
			if (ys.empty() || ys.back() < p.y) ys.push_back(p.y);
			p.rank = int(ys.size() - 1);
		}
		sort(ordered, [](const Point &p) -> const X & { return p.x; });
		std::vector<Event> events;
		events.reserve(2 * queries.size());
		for (int i = 0; i < int(queries.size()); ++i) {
			const auto &q = queries[i];
			if (!(q.left < q.right) || !(q.down < q.up)) continue;
			int down = std::lower_bound(ys.begin(), ys.end(), q.down) - ys.begin();
			int up = std::lower_bound(ys.begin(), ys.end(), q.up) - ys.begin();
			if (down == up) continue;
			events.push_back({q.left, down, up, i, true});
			events.push_back({q.right, down, up, i, false});
		}
		sort(events, [](const Event &e) -> const X & { return e.x; });
		bool narrow = false;
		if constexpr (std::is_integral_v<Sum> && sizeof(Sum) <= 8) {
			Accum positive = 0, negative = 0;
			for (const auto &point : points) {
				Accum value(point.weight);
				if (value > 0) positive += value;
				else negative += value;
			}
			narrow = positive <= INT64_MAX && negative >= INT64_MIN;
		}
		if (narrow) {
			if constexpr (std::is_integral_v<Sum> && sizeof(Sum) <= 8) sweep<std::int64_t>(ordered, events, int(ys.size()), answers);
		} else sweep<Accum>(ordered, events, int(ys.size()), answers);

		if constexpr (std::is_same_v<Accum, Sum>) return answers;
		else return std::vector<Sum>(answers.begin(), answers.end());
	}
};


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


#line 4 "test/offline_rectangle_sum.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	OfflineRectangleSum<int, int, std::int64_t> solver(n, q);
	for (int i = 0; i < n; ++i) {
		int x = 0, y = 0, w = 0;
		input.read(x); input.read(y); input.read(w);
		solver.add_point(x, y, w);
	}
	for (int i = 0; i < q; ++i) {
		int l = 0, d = 0, r = 0, u = 0;
		input.read(l); input.read(d); input.read(r); input.read(u);
		solver.add_query(l, d, r, u);
	}
	for (auto answer : solver.solve()) output.println(answer);
}
