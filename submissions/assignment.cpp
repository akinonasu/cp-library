#line 1 "test/hungarian.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/assignment"
#line 1 "graph/hungarian.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

template <class Cost = long long>
struct Hungarian {
	static_assert(std::numeric_limits<Cost>::is_integer && std::numeric_limits<Cost>::is_signed);
	Cost cost = 0;
	std::vector<int> row_to_col, col_to_row;
	template <class Matrix>
	explicit Hungarian(const Matrix &a, bool maximize = false) {
		int n = int(a.size()), m = n ? int(a[0].size()) : 0;
		using T = std::remove_cvref_t<decltype(a[0][0])>;
		std::vector<const T *> rows(n);
		for (int i = 0; i < n; ++i) {
			assert(int(a[i].size()) == m);
			rows[i] = std::data(a[i]);
		}
		if (maximize) solve<true>(n, m, rows);
		else solve<false>(n, m, rows);
	}
	template <class T>
	Hungarian(int n, int m, const T *a, int stride = -1, bool maximize = false) {
		assert(0 <= n && n <= m);
		if (stride == -1) stride = m;
		assert(stride >= m);
		std::vector<const T *> rows(n);
		for (int i = 0; i < n; ++i) rows[i] = a + std::size_t(i) * stride;
		if (maximize) solve<true>(n, m, rows);
		else solve<false>(n, m, rows);
	}
private:
	template <bool maximize, class T>
	void solve(int n, int m, const std::vector<const T *> &a) {
		assert(n <= m);
		row_to_col.assign(n, -1);
		col_to_row.assign(m, -1);
		if (!n) return;
		auto value = [&](int i, int j) -> Cost {
			if constexpr (maximize) return -Cost(a[i][j]);
			else return Cost(a[i][j]);
		};
		auto finish = [&] {
			for (int i = 0; i < n; ++i) cost += Cost(a[i][row_to_col[i]]);
		};
		if (n == m) {
			using Wide = std::conditional_t<(sizeof(T) < 8), long long, __int128_t>;
			int order = 3;
			for (int i = 1; i < n && order; ++i)
				for (int j = 1; j < m; ++j) {
					Wide x = Wide(a[i - 1][j - 1]) + Wide(a[i][j]);
					Wide y = Wide(a[i - 1][j]) + Wide(a[i][j - 1]);
					if (x > y) order &= 2;
					if (x < y) order &= 1;
					if (!order) break;
				}
			if (order) {
				bool reverse = maximize ? bool(order & 1) : !(order & 1);
				for (int i = 0; i < n; ++i) {
					int j = reverse ? n - 1 - i : i;
					row_to_col[i] = j;
					col_to_row[j] = i;
				}
				finish();
				return;
			}
		}
		std::vector<Cost> price(m), dist(m);
		std::vector<int> columns(m), pred(m);
		if (n == m) {
			std::vector<T> minimum(a[0], a[0] + m);
			std::vector<int> owner(m), count(n);
			for (int i = 1; i < n; ++i)
				for (int j = 0; j < m; ++j) {
					T x = a[i][j];
					bool better = maximize ? x > minimum[j] : x < minimum[j];
					minimum[j] = better ? x : minimum[j];
					owner[j] = better ? i : owner[j];
				}
			for (int j = 0; j < m; ++j) price[j] = maximize ? -Cost(minimum[j]) : Cost(minimum[j]);
			for (int j = m - 1; j >= 0; --j) {
				int i = owner[j];
				if (!count[i]++) row_to_col[i] = j, col_to_row[j] = i;
			}
			for (int i = 0; i < n; ++i) if (count[i] == 1) {
				Cost delta = std::numeric_limits<Cost>::max();
				int matched = row_to_col[i];
				for (int j = 0; j < m; ++j)
					if (j != matched) delta = std::min(delta, Cost(value(i, j) - price[j]));
				price[matched] -= delta;
			}
			for (int pass = 0; pass < 2; ++pass)
				for (int i = 0; i < n; ++i) if (row_to_col[i] == -1) {
					int best_col = 0;
					Cost best = value(i, 0) - price[0], second = std::numeric_limits<Cost>::max();
					for (int j = 1; j < m; ++j) {
						Cost x = value(i, j) - price[j];
						if (x < best) second = best, best = x, best_col = j;
						else if (x == best && col_to_row[best_col] != -1 && col_to_row[j] == -1) second = best, best_col = j;
						else second = std::min(second, x);
					}
					if (best < second) price[best_col] -= second - best;
					int old = col_to_row[best_col];
					if (old != -1) row_to_col[old] = -1;
					row_to_col[i] = best_col;
					col_to_row[best_col] = i;
				}
		} else {
			for (int i = 0; i < n; ++i) {
				int best_col = 0;
				Cost best = value(i, 0);
				for (int j = 1; j < m; ++j) {
					Cost x = value(i, j);
					if (x < best || (x == best && col_to_row[best_col] != -1 && col_to_row[j] == -1)) best = x, best_col = j;
				}
				if (col_to_row[best_col] == -1) row_to_col[i] = best_col, col_to_row[best_col] = i;
			}
		}
		std::iota(columns.begin(), columns.end(), 0);
		for (int root = 0; root < n; ++root) if (row_to_col[root] == -1) {
			for (int j = 0; j < m; ++j) dist[j] = value(root, j) - price[j], pred[j] = root;
			int scanned = 0, queued = 0, target = -1;
			Cost level = 0;
			while (target == -1) {
				if (scanned == queued) {
					level = dist[columns[scanned]];
					for (int k = scanned; k < m; ++k) {
						int j = columns[k];
						if (dist[j] > level) continue;
						if (dist[j] < level) level = dist[j], queued = scanned, target = -1;
						std::swap(columns[k], columns[queued++]);
						if (col_to_row[j] == -1) target = j;
					}
					if (target != -1) break;
				}
				int j0 = columns[scanned++], i = col_to_row[j0];
				Cost offset = value(i, j0) - price[j0] - level;
				for (int k = queued; k < m; ++k) {
					int j = columns[k];
					Cost candidate = value(i, j) - price[j] - offset;
					if (candidate >= dist[j]) continue;
					dist[j] = candidate;
					pred[j] = i;
					if (candidate == level) {
						if (col_to_row[j] == -1) { target = j; break; }
						std::swap(columns[k], columns[queued++]);
					}
				}
			}
			for (int k = 0; k < scanned; ++k) {
				int j = columns[k];
				price[j] += dist[j] - level;
			}
			while (target != -1) {
				int i = pred[target], next = row_to_col[i];
				row_to_col[i] = target;
				col_to_row[target] = i;
				target = next;
			}
		}
		finish();
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
#line 8 "components/fast_io.hpp"
#include <charconv>
#line 10 "components/fast_io.hpp"
#include <cstdint>
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


#line 5 "test/hungarian.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n;
	in.read(n);
	std::vector<int> a(std::size_t(n) * n);
	for (auto &x : a) in.read(x);
	Hungarian res(n, n, a.data());
	out.write(res.cost);
	out.write('\n');
	for (int i = 0; i < n; ++i) {
		if (i) out.write(' ');
		out.write(res.row_to_col[i]);
	}
	out.write('\n');
}
