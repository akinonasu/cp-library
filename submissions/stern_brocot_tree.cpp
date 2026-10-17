#line 1 "test/stern_brocot_tree.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/stern_brocot_tree"
#line 1 "math/stern_brocot_tree.hpp"



#line 1 "components/stern_brocot_path.hpp"



#include <cassert>
#include <concepts>

namespace stern_brocot_detail {
template <std::unsigned_integral U>
class Path {
	U a, b;
public:
	Path(U numerator, U denominator) : a(numerator), b(denominator) { assert(a && b); }
	bool empty() const { return a == b; }
	bool right() const { return a > b; }
	U pop() {
		assert(!empty());
		if (right()) { U k = (a - 1) / b; a -= k * b; return k; }
		U k = (b - 1) / a; b -= k * a; return k;
	}
};
}


#line 5 "math/stern_brocot_tree.hpp"
#include <algorithm>
#line 8 "math/stern_brocot_tree.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template <std::integral Int = std::uint64_t>
requires (sizeof(Int) <= 8 && !std::is_same_v<Int, bool>)
class SternBrocotTree {
public:
	using value_type = std::pair<Int, Int>;
	using count_type = std::make_unsigned_t<Int>;
	using distance_type = std::conditional_t<(sizeof(Int) <= 4), std::uint64_t, __uint128_t>;
	struct Run {
		char direction;
		count_type count;
		bool operator==(const Run &) const = default;
	};
	using path_type = std::vector<Run>;
private:
	using U = count_type;
	using W = distance_type;
	using Path = stern_brocot_detail::Path<U>;
	Int ln = 0, ld = 1, rn = 1, rd = 0;
	U level = 0;
	void advance(bool right, U k) {
		if (right) { ln += k * U(rn); ld += k * U(rd); }
		else { rn += k * U(ln); rd += k * U(ld); }
		level += k;
	}
	bool move(bool right, U k) {
		W n = W(numerator()) + W(k) * U(right ? rn : ln);
		W d = W(denominator()) + W(k) * U(right ? rd : ld);
		if (n > W(std::numeric_limits<Int>::max()) || d > W(std::numeric_limits<Int>::max())) return false;
		advance(right, k); return true;
	}
public:
	SternBrocotTree() = default;
	SternBrocotTree(Int numerator, Int denominator) {
		assert(numerator > 0 && denominator > 0);
		Path path{U(numerator), U(denominator)};
		while (!path.empty()) { bool right = path.right(); advance(right, path.pop()); }
	}
	explicit SternBrocotTree(value_type value) : SternBrocotTree(value.first, value.second) {}
	Int numerator() const { return ln + rn; }
	Int denominator() const { return ld + rd; }
	value_type value() const { return {numerator(), denominator()}; }
	value_type lower_bound() const { return {ln, ld}; }
	value_type upper_bound() const { return {rn, rd}; }
	std::pair<value_type, value_type> range() const { return {lower_bound(), upper_bound()}; }
	U depth() const { return level; }
	bool is_root() const { return level == 0; }
	bool operator==(const SternBrocotTree &) const = default;
	bool move_left(U k = 1) { return move(false, k); }
	bool move_right(U k = 1) { return move(true, k); }
	bool move_up(U k = 1) {
		if (k > level) return false;
		level -= k;
		while (k) {
			bool right = ln >= rn;
			U step = k == 1 ? 1 : std::min<U>(k, right ? U(ln) / U(rn) : U(rd) / U(ld));
			if (right) { ln -= step * U(rn); ld -= step * U(rd); }
			else { rn -= step * U(ln); rd -= step * U(ld); }
			k -= step;
		}
		return true;
	}
	std::optional<SternBrocotTree> left(U k = 1) const {
		auto result = *this;
		if (!result.move_left(k)) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> right(U k = 1) const {
		auto result = *this;
		if (!result.move_right(k)) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> parent() const {
		auto result = *this;
		if (!result.move_up()) return std::nullopt;
		return result;
	}
	bool contains(value_type x) const {
		assert(x.first > 0 && x.second > 0);
		return W(ln) * U(x.second) < W(ld) * U(x.first) && W(x.first) * U(rd) < W(x.second) * U(rn);
	}
	bool is_ancestor(const SternBrocotTree &other) const { return contains(other.value()); }
	path_type path() const { return encode(numerator(), denominator()); }
	template <class Emit>
	static void encode(Int numerator, Int denominator, Emit &&emit) {
		assert(numerator > 0 && denominator > 0);
		Path path{U(numerator), U(denominator)};
		while (!path.empty()) { char direction = path.right() ? 'R' : 'L'; emit(direction, path.pop()); }
	}
	static path_type encode(Int numerator, Int denominator) {
		path_type result;
		encode(numerator, denominator, [&](char direction, U count) { result.push_back({direction, count}); });
		return result;
	}
	static std::optional<SternBrocotTree> decode(std::span<const Run> path) {
		SternBrocotTree result;
		for (auto [direction, count] : path) {
			if (direction != 'L' && direction != 'R') return std::nullopt;
			if (!result.move(direction == 'R', count)) return std::nullopt;
		}
		return result;
	}
	static std::optional<SternBrocotTree> ancestor(value_type x, U depth) {
		assert(x.first > 0 && x.second > 0);
		SternBrocotTree result;
		Path path{U(x.first), U(x.second)};
		while (depth && !path.empty()) {
			bool right = path.right();
			U step = std::min(depth, path.pop());
			result.advance(right, step); depth -= step;
		}
		if (depth) return std::nullopt;
		return result;
	}
	std::optional<SternBrocotTree> ancestor(U depth) const {
		if (depth > level) return std::nullopt;
		return ancestor(value(), depth);
	}
	static SternBrocotTree lca(value_type x, value_type y) {
		assert(x.first > 0 && x.second > 0 && y.first > 0 && y.second > 0);
		SternBrocotTree result;
		Path a{U(x.first), U(x.second)}, b{U(y.first), U(y.second)};
		while (!a.empty() && !b.empty() && a.right() == b.right()) {
			bool right = a.right();
			U k = a.pop(), l = b.pop();
			result.advance(right, std::min(k, l));
			if (k != l) break;
		}
		return result;
	}
	SternBrocotTree lca(const SternBrocotTree &other) const { return lca(value(), other.value()); }
	W distance(const SternBrocotTree &other) const { return W(level) + other.level - 2 * W(lca(other).level); }
};


#line 1 "components/fast_io.hpp"



#line 6 "components/fast_io.hpp"
#include <charconv>
#include <cstddef>
#line 9 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 16 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

class FastInput {
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
		else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/stern_brocot_tree.test.cpp"
#include <array>
#line 6 "test/stern_brocot_tree.test.cpp"

int main() {
#if defined(BENCH_STERN_BROCOT_64)
	using Tree = SternBrocotTree<std::uint64_t>;
#else
	using Tree = SternBrocotTree<std::uint32_t>;
#endif
	FastInput in;
	FastOutput out;
	int q = 0;
	if (!in.read(q)) return 0;
	auto read = [&] { Tree::count_type x = 0; in.read(x); return x; };
	auto print = [&](Tree node) { auto [a, b] = node.value(); out.write(a); out.write(' '); out.println(b); };
	while (q--) {
		char command = in.token()[0];
		if (command == 'E') {
			auto a = read(), b = read();
			std::array<Tree::Run, 128> path;
			int n = 0;
			Tree::encode(a, b, [&](char direction, auto count) { path[n++] = {direction, count}; });
			out.write(n);
			for (int i = 0; i < n; ++i) { out.write(' '); out.write(path[i].direction); out.write(' '); out.write(path[i].count); }
			out.write('\n');
		} else if (command == 'D') {
			auto n = read();
			Tree node;
			while (n--) {
				bool right = in.token()[0] == 'R';
				auto count = read();
				[[maybe_unused]] bool ok = right ? node.move_right(count) : node.move_left(count);
				assert(ok);
			}
			print(node);
		} else if (command == 'L') {
			auto a = read(), b = read(), c = read(), d = read();
			print(Tree::lca({a, b}, {c, d}));
		} else if (command == 'A') {
			auto depth = read(), a = read(), b = read();
			auto node = Tree::ancestor({a, b}, depth);
			if (node) print(*node);
			else out.println(-1);
		} else {
			auto a = read(), b = read();
			auto [lower, upper] = Tree(a, b).range();
			out.write(lower.first); out.write(' '); out.write(lower.second); out.write(' ');
			out.write(upper.first); out.write(' '); out.println(upper.second);
		}
	}
}
