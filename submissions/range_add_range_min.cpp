#line 1 "test/kinetic_tournament.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#line 1 "data_structure/advanced/kinetic_tournament.hpp"



#line 1 "data_structure/advanced/generic_segtree_beats.hpp"



#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class GenericSegtreeBeats {
	int n_ = 0, base_ = 1, log_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const {
		assert(0 <= l && l <= r && r <= n_);
	}
	void pull(int k) { data_[k] = op(data_[2 * k], data_[2 * k + 1]); }
	void all_apply(int k, const F &f) {
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = 1;
		}
		if (mapping(f, data_[k])) return;
		assert(k < base_);
		push(k);
		pull(k);
	}
	void push(int k) {
		if (!dirty_[k]) return;
		all_apply(2 * k, lazy_[k]);
		all_apply(2 * k + 1, lazy_[k]);
		dirty_[k] = 0;
	}
	void push_path(int k) {
		for (int h = log_; h; --h) push(k >> h);
	}
	void push_range(int l, int r) {
		for (int h = log_; h; --h) {
			if (((l >> h) << h) != l) push(l >> h);
			if (((r >> h) << h) != r) push((r - 1) >> h);
		}
	}
	void pull_range(int l, int r) {
		for (int h = 1; h <= log_; ++h) {
			if (((l >> h) << h) != l) pull(l >> h);
			if (((r >> h) << h) != r) pull((r - 1) >> h);
		}
	}

  public:
	GenericSegtreeBeats() : GenericSegtreeBeats(0) {}
	explicit GenericSegtreeBeats(int n, const S &value = e()) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 4);
		build(std::vector<S>(n, value));
	}
	explicit GenericSegtreeBeats(const std::vector<S> &a) { build(a); }
	void build(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		n_ = static_cast<int>(a.size());
		base_ = static_cast<int>(std::bit_ceil(static_cast<unsigned>(std::max(1, n_))));
		log_ = std::countr_zero(static_cast<unsigned>(base_));
		data_.assign(2 * base_, e());
		lazy_.assign(base_, id());
		dirty_.assign(base_, 0);
		std::copy(a.begin(), a.end(), data_.begin() + base_);
		for (int k = base_ - 1; k; --k) pull(k);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	S all_prod() const { return data_[1]; }
	S get(int p) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		return data_[k];
	}
	void set(int p, const S &value) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		data_[k] = value;
		while (k >>= 1) pull(k);
	}
	S prod(int l, int r) {
		check(l, r);
		if (l == r) return e();
		l += base_, r += base_;
		push_range(l, r);
		S left = e(), right = e();
		while (l < r) {
			if (l & 1) left = op(std::move(left), data_[l++]);
			if (r & 1) right = op(data_[--r], std::move(right));
			l >>= 1, r >>= 1;
		}
		return op(std::move(left), std::move(right));
	}
	void apply(const F &f) { if (n_) all_apply(1, f); }
	void apply(int p, const F &f) {
		assert(0 <= p && p < n_);
		int k = p + base_;
		push_path(k);
		all_apply(k, f);
		while (k >>= 1) pull(k);
	}
	void apply(int l, int r, const F &f) {
		check(l, r);
		if (l == r) return;
		l += base_, r += base_;
		push_range(l, r);
		int first = l, last = r;
		while (l < r) {
			if (l & 1) all_apply(l++, f);
			if (r & 1) all_apply(--r, f);
			l >>= 1, r >>= 1;
		}
		pull_range(first, last);
	}
	template <class Predicate>
	int max_right(int l, Predicate predicate) {
		assert(0 <= l && l <= n_ && predicate(e()));
		if (l == n_) return n_;
		int k = l + base_;
		push_path(k);
		S value = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(value, data_[k]);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k *= 2;
					next = op(value, data_[k]);
					if (predicate(next)) value = std::move(next), ++k;
				}
				return std::min(k - base_, n_);
			}
			value = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template <class Predicate>
	int min_left(int r, Predicate predicate) {
		assert(0 <= r && r <= n_ && predicate(e()));
		if (!r) return 0;
		int k = r + base_;
		push_path(k - 1);
		S value = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], value);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k = 2 * k + 1;
					next = op(data_[k], value);
					if (predicate(next)) value = std::move(next), --k;
				}
				return k + 1 - base_;
			}
			value = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
	std::vector<S> values() {
		for (int k = 1; k < base_; ++k) push(k);
		return std::vector<S>(data_.begin() + base_, data_.begin() + base_ + n_);
	}
};


#line 1 "components/kinetic_tournament_monoid.hpp"



#line 6 "components/kinetic_tournament_monoid.hpp"
#include <cstdint>
#line 8 "components/kinetic_tournament_monoid.hpp"
#include <type_traits>

namespace kinetic_tournament_detail {
template<class T, bool maximize> struct Monoid {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using W = __int128_t;
	using U = std::uint64_t;
	static constexpr U never = std::numeric_limits<U>::max();
	struct Node { T value = 0, slope = 0; U remaining = never; int index = -1; };
	struct Tag { W steps = 0, add = 0; };
	static Node identity() { return {}; }
	static Tag tag_identity() { return {}; }
	static bool better(const Node &a, const Node &b) {
		if (a.index == -1) return false;
		if (b.index == -1) return true;
		if (a.value != b.value) return maximize ? a.value > b.value : a.value < b.value;
		return maximize ? a.index > b.index : a.index < b.index;
	}
	static Node op(Node a, Node b) {
		if (a.index == -1) return b;
		if (b.index == -1) return a;
		bool left = better(a, b);
		Node result = left ? a : b;
		result.remaining = std::min(a.remaining, b.remaining);
		if (left == maximize && a.slope < b.slope) {
			U gap = U(a.value) - U(b.value);
			U difference = U(b.slope) - U(a.slope);
			assert(gap > 0);
			result.remaining = std::min(result.remaining, (gap - 1) / difference);
		}
		return result;
	}
	static bool mapping(Tag tag, Node &node) {
		if (node.index == -1) return true;
		if (node.remaining != never && tag.steps > W(node.remaining)) return false;
		W value = W(node.value) + tag.steps * node.slope + tag.add;
		assert(std::numeric_limits<T>::lowest() <= value && value <= std::numeric_limits<T>::max());
		node.value = T(value);
		if (node.remaining != never) node.remaining -= U(tag.steps);
		return true;
	}
	static Tag composition(Tag a, Tag b) { return {a.steps + b.steps, a.add + b.add}; }
};
}


#line 6 "data_structure/advanced/kinetic_tournament.hpp"
#include <optional>

template<class T = std::int64_t, bool maximize = true> class KineticTournament {
	using M = kinetic_tournament_detail::Monoid<T, maximize>;
	using Node = typename M::Node;
	GenericSegtreeBeats<Node, M::op, M::identity, typename M::Tag, M::mapping, M::composition, M::tag_identity> tree_;
	std::vector<T> slopes_;
	std::vector<unsigned char> active_;
	int count_ = 0;
	void check([[maybe_unused]] int i) const { assert(0 <= i && i < size()); }
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }

  public:
	struct Result { int index; T value; };
	using value_type = T;
	KineticTournament() = default;
	explicit KineticTournament(const std::vector<T> &slopes, T value = 0) { build(slopes, value); }
	KineticTournament(const std::vector<T> &slopes, const std::vector<T> &values) { build(slopes, values); }
	KineticTournament(const std::vector<T> &slopes, const std::vector<std::optional<T>> &values) { build(slopes, values); }
	KineticTournament(const std::vector<T> &slopes, std::nullopt_t) { build(slopes, std::nullopt); }
	KineticTournament(const KineticTournament &) = default;
	KineticTournament &operator=(const KineticTournament &) = default;
	KineticTournament(KineticTournament &&other) : KineticTournament() { swap(other); }
	KineticTournament &operator=(KineticTournament &&other) {
		if (this != &other) { KineticTournament next(std::move(other)); swap(next); }
		return *this;
	}
	void swap(KineticTournament &other) noexcept {
		std::swap(tree_, other.tree_);
		slopes_.swap(other.slopes_); active_.swap(other.active_); std::swap(count_, other.count_);
	}
	void build(const std::vector<T> &slopes, const std::vector<std::optional<T>> &values) {
		assert(slopes.size() == values.size() && slopes.size() <= std::size_t(std::numeric_limits<int>::max() / 4));
		assert(std::is_sorted(slopes.begin(), slopes.end()));
		slopes_ = slopes;
		active_.assign(slopes.size(), 0);
		count_ = 0;
		std::vector<Node> nodes(slopes.size());
		for (int i = 0; i < size(); ++i) if (values[i]) {
			nodes[i] = {*values[i], slopes_[i], M::never, i};
			active_[i] = 1; ++count_;
		}
		tree_.build(nodes);
	}
	void build(const std::vector<T> &slopes, const std::vector<T> &values) { build(slopes, std::vector<std::optional<T>>(values.begin(), values.end())); }
	void build(const std::vector<T> &slopes, T value = 0) { build(slopes, std::vector<std::optional<T>>(slopes.size(), value)); }
	void build(const std::vector<T> &slopes, std::nullopt_t) { build(slopes, std::vector<std::optional<T>>(slopes.size())); }
	int size() const { return static_cast<int>(slopes_.size()); }
	int active_size() const { return count_; }
	bool empty() const { return count_ == 0; }
	const std::vector<T> &slopes() const { return slopes_; }
	void clear() { tree_.build(std::vector<Node>(size())); std::fill(active_.begin(), active_.end(), 0); count_ = 0; }
	void set(int i, T value) {
		check(i);
		tree_.set(i, Node{value, slopes_[i], M::never, i});
		count_ += !active_[i]; active_[i] = 1;
	}
	bool erase(int i) {
		check(i);
		if (!active_[i]) return false;
		tree_.set(i, M::identity());
		active_[i] = 0; --count_;
		return true;
	}
	std::optional<T> get(int i) { check(i); return active_[i] ? std::optional<T>(tree_.get(i).value) : std::nullopt; }
	void apply(int l, int r, T a, T b = 0) {
		check(l, r); assert(a >= 0);
		if (l == r || (!a && !b)) return;
		if (!l && r == size()) tree_.apply({a, b});
		else tree_.apply(l, r, {a, b});
	}
	void add(int l, int r, T value) { apply(l, r, 0, value); }
	void advance(int l, int r, T steps = 1) { apply(l, r, steps, 0); }
	void all_apply(T a, T b = 0) { apply(0, size(), a, b); }
	std::optional<Result> prod(int l, int r) {
		check(l, r);
		if (!l && r == size()) return all_prod();
		auto node = tree_.prod(l, r);
		return node.index == -1 ? std::nullopt : std::optional<Result>(Result{node.index, node.value});
	}
	std::optional<Result> all_prod() const {
		auto node = tree_.all_prod();
		return node.index == -1 ? std::nullopt : std::optional<Result>(Result{node.index, node.value});
	}
	std::vector<std::optional<T>> values() {
		auto nodes = tree_.values();
		std::vector<std::optional<T>> result(size());
		for (int i = 0; i < size(); ++i) if (active_[i]) result[i] = nodes[i].value;
		return result;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
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


#line 4 "test/kinetic_tournament.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<std::int64_t> values(n), slopes(n);
	for (auto &value : values) in.read(value);
	KineticTournament<std::int64_t, false> tree(slopes, values);
	for (int i = 0; i < q; ++i) {
		int type = 0, l = 0, r = 0;
		in.read(type); in.read(l); in.read(r);
		if (!type) { std::int64_t value = 0; in.read(value); tree.add(l, r, value); }
		else out.println(tree.prod(l, r)->value);
	}
}
