#line 1 "test/centroid_decomposition_contour.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree"
#line 1 "tree/contour_sum.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace tree_contour_sum_detail {
template<class T, bool = std::is_integral_v<T>> struct Storage { using type = T; };
template<class T> struct Storage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct Storage<__int128_t, false> { using type = __uint128_t; };
#endif
}

template<class T = std::int64_t> class TreeContourSum {
	static_assert(!std::is_same_v<T, bool>);
	using U = typename tree_contour_sum_detail::Storage<T>::type;
	struct Micro { std::size_t distances; int first; std::uint8_t count, maximum; };
	struct Bucket { std::size_t offset = 0; int length = 0; };
	struct Route { int centroid, branch, distance; };
	std::vector<int> position_;
	std::vector<T> values_;
	std::vector<Micro> micro_;
	std::vector<std::uint8_t> distances_;
	std::vector<Bucket> buckets_;
	std::vector<std::size_t> offsets_;
	std::vector<Route> routes_;
	std::vector<U> sums_;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	void add_bucket(int group, int distance, U value) {
		const auto& bucket = buckets_[group];
		for (unsigned i = unsigned(distance) + 1; i <= unsigned(bucket.length); i += i & -i)
			sums_[bucket.offset + i - 1] += value;
	}
	U sum_bucket(int group, int l, int r) const {
		const auto& bucket = buckets_[group];
		unsigned a = unsigned(std::clamp(l, 0, bucket.length)), b = unsigned(std::clamp(r, 0, bucket.length));
		U result{};
		while (b > a) result += sums_[bucket.offset + b - 1], b &= b - 1;
		while (a > b) result -= sums_[bucket.offset + a - 1], a &= a - 1;
		return result;
	}

  public:
	TreeContourSum() = default;
	TreeContourSum(int n, std::span<const std::pair<int, int>> edges, const std::vector<T>& values) { build(n, edges, values); }
	void build(int n, std::span<const std::pair<int, int>> edges, const std::vector<T>& values) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		assert(values.size() == std::size_t(n) && edges.size() == std::size_t(n ? n - 1 : 0));
		position_.assign(n, -1), micro_.resize(n);
		values_.clear(), values_.reserve(std::size_t(n) * 2), distances_.clear();
		buckets_.clear(), buckets_.reserve(std::size_t(n) * 2), sums_.clear();
		offsets_.assign(std::size_t(n) + 1, 0), routes_.clear();
		if (!n) return;
		std::vector<int> start(std::size_t(n) + 1), adjacent(edges.size() * 2);
		for (auto [u, v] : edges) {
			assert(0 <= u && u < n && 0 <= v && v < n && u != v);
			++start[u + 1], ++start[v + 1];
		}
		for (int v = 0; v < n; ++v) start[v + 1] += start[v];
		{
			auto cursor = start;
			for (auto [u, v] : edges) adjacent[cursor[u]++] = v, adjacent[cursor[v]++] = u;
		}
		unsigned stride = std::bit_width(unsigned(n));
		if (std::size_t(n) > std::numeric_limits<std::size_t>::max() / stride / sizeof(Route)) throw std::length_error("TreeContourSum routes");
		auto pending = std::make_unique_for_overwrite<Route[]>(std::size_t(n) * stride);
		std::vector<std::uint8_t> removed(n), levels(n);
		std::vector<int> previous(n, -1), sub(n), distance(n), work;
		work.reserve(n);
		std::vector<std::pair<int, unsigned>> tasks{{0, 0}};
		std::vector<U> full, branch;
		auto append_bucket = [&](int id, const std::vector<U>& raw) {
			if (raw.size() > sums_.max_size() - sums_.size()) throw std::length_error("TreeContourSum buckets");
			std::size_t offset = sums_.size();
			buckets_[id] = {offset, int(raw.size())};
			sums_.insert(sums_.end(), raw.begin(), raw.end());
			for (unsigned i = 1; i <= raw.size(); ++i) {
				unsigned j = i + (i & -i);
				if (j <= raw.size()) sums_[offset + j - 1] += sums_[offset + i - 1];
			}
		};
		while (!tasks.empty()) {
			auto [s, level] = tasks.back();
			tasks.pop_back();
			work.clear(), work.push_back(s), previous[s] = -2;
			for (std::size_t i = 0; i < work.size(); ++i) {
				int v = work[i];
				sub[v] = 1;
				for (int e = start[v]; e < start[v + 1]; ++e) {
					int u = adjacent[e];
					if (removed[u] || u == previous[v]) continue;
					assert(level || previous[u] == -1);
					previous[u] = v, work.push_back(u);
				}
			}
			assert(level || work.size() == std::size_t(n));
			int count = int(work.size());
			if (count <= 32) {
				int first = int(values_.size());
				U total{};
				for (int v : work) {
					position_[v] = int(values_.size());
					values_.push_back(values[v]), total += U(values[v]);
				}
				values_.push_back(T(total));
				std::size_t base = distances_.size();
				if (std::size_t(count) * count > distances_.max_size() - base) throw std::length_error("TreeContourSum distances");
				distances_.resize(base + std::size_t(count) * count);
				std::array<std::uint8_t, 32> maximum{};
				for (int j = 1; j < count; ++j) {
					int p = position_[previous[work[j]]] - first;
					auto* row = distances_.data() + base + std::size_t(j) * count;
					const auto* parent_row = distances_.data() + base + std::size_t(p) * count;
					std::uint8_t farthest = 0;
					for (int k = 0; k < j; ++k) {
						std::uint8_t d = parent_row[k] + 1;
						row[k] = d, distances_[base + std::size_t(k) * count + j] = d;
						farthest = std::max(farthest, d), maximum[k] = std::max(maximum[k], d);
					}
					maximum[j] = farthest;
				}
				for (int j = 0; j < count; ++j)
					micro_[work[j]] = {base + std::size_t(j) * count, first, std::uint8_t(count), maximum[j]};

				for (int v : work) removed[v] = 1;
				continue;
			}
			int c = -1;
			for (auto it = work.rbegin(); it != work.rend(); ++it) {
				int v = *it;
				if (c == -1 && sub[v] >= (count + 1) / 2) c = v;
				if (previous[v] >= 0) sub[previous[v]] += sub[v];
			}
			removed[c] = 1;
			position_[c] = int(values_.size());
			values_.push_back(values[c]), micro_[c] = {0, position_[c], 0, 0};
			int all = int(buckets_.size());
			buckets_.push_back({});
			assert(level < stride);
			pending[std::size_t(level) * n + c] = {all, -1, 0}, levels[c] = std::uint8_t(level + 1);
			full.assign(1, U(values[c]));
			for (int e = start[c]; e < start[c + 1]; ++e) {
				int root = adjacent[e];
				if (removed[root]) continue;
				int group = int(buckets_.size());
				buckets_.push_back({});
				branch.assign(2, U{});
				work.clear(), work.push_back(root), previous[root] = c, distance[root] = 1;
				for (std::size_t i = 0; i < work.size(); ++i) {
					int v = work[i], d = distance[v];
					pending[std::size_t(level) * n + v] = {all, group, d}, levels[v] = std::uint8_t(level + 1);
					if (std::size_t(d) >= full.size()) full.resize(std::size_t(d) + 1);
					if (std::size_t(d) >= branch.size()) branch.resize(std::size_t(d) + 1);
					full[d] += U(values[v]), branch[d] += U(values[v]);
					for (int j = start[v]; j < start[v + 1]; ++j) {
						int u = adjacent[j];
						if (removed[u] || u == previous[v]) continue;
						previous[u] = v, distance[u] = d + 1, work.push_back(u);
					}
				}
				append_bucket(group, branch);
				tasks.emplace_back(root, level + 1);
			}
			append_bucket(all, full);
		}
		for (int v = 0; v < n; ++v) offsets_[v + 1] = offsets_[v] + levels[v];
		routes_.reserve(offsets_.back());
		for (int v = 0; v < n; ++v)
			for (unsigned level = levels[v]; level-- > 0;) routes_.push_back(pending[std::size_t(level) * n + v]);
	}

	int size() const { return int(position_.size()); }
	bool empty() const { return position_.empty(); }
	const T& get(int v) const { check(v); return values_[position_[v]]; }
	void add(int v, T value) {
		check(v);
		values_[position_[v]] = T(U(values_[position_[v]]) + U(value));
		const auto& micro = micro_[v];
		if (micro.count) values_[micro.first + micro.count] = T(U(values_[micro.first + micro.count]) + U(value));
		for (std::size_t i = offsets_[v]; i < offsets_[v + 1]; ++i) {
			const auto& route = routes_[i];
			add_bucket(route.centroid, route.distance, U(value));
			if (route.branch != -1) add_bucket(route.branch, route.distance, U(value));
		}
	}
	void set(int v, T value) { add(v, T(U(value) - U(get(v)))); }
	T prod(int v, std::int64_t l, std::int64_t r) const {
		check(v);
		l = std::max<std::int64_t>(l, 0), r = std::min<std::int64_t>(r, size());
		if (l >= r) return T{};
		U answer{};
		const auto& micro = micro_[v];
		if (micro.count && l <= micro.maximum) {
			if (l == 0 && r > micro.maximum) answer += U(values_[micro.first + micro.count]);
			else {
				const auto* distances = distances_.data() + micro.distances;
				const auto* values = values_.data() + micro.first;
				for (int i = 0; i < micro.count; ++i)
					if (l <= distances[i] && distances[i] < r) answer += U(values[i]);
			}
		}
		for (std::size_t i = offsets_[v]; i < offsets_[v + 1]; ++i) {
			const auto& route = routes_[i];
			int low = int(l) - route.distance, high = int(r) - route.distance;
			answer += sum_bucket(route.centroid, low, high);
			if (route.branch != -1) answer -= sum_bucket(route.branch, low, high);
		}
		return T(answer);
	}
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
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


#line 4 "test/centroid_decomposition_contour.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n, q;
	input.read(n), input.read(q);
	TreeContourSum<> sums;
	{
		std::vector<std::int64_t> values(n);
		for (auto& x : values) input.read(x);
		std::vector<std::pair<int, int>> edges;
		edges.reserve(n - 1);
		for (int i = 1, u, v; i < n; ++i) {
			input.read(u), input.read(v);
			edges.emplace_back(u, v);
		}
		sums.build(n, edges, values);
	}
	while (q--) {
		int type, v;
		input.read(type), input.read(v);
		if (type == 0) {
			std::int64_t x;
			input.read(x);
			sums.add(v, x);
		} else {
			int l, r;
			input.read(l), input.read(r);
			output.println(sums.prod(v, l, r));
		}
	}
}
