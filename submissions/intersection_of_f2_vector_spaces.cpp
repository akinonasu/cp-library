#line 1 "test/xor_basis_intersection.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/intersection_of_f2_vector_spaces"
#line 1 "math/linear_algebra/xor_basis.hpp"



#line 1 "components/xor_basis_core.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace xor_basis_detail {
template <class UInt, bool maximum = false>
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
inline UInt reduce_rows(UInt x, const UInt *row, int count) {
	int i = 0;
	auto step = [&](UInt value) {
		if constexpr (maximum) x = std::max(x, UInt(x ^ value));
		else x = std::min(x, UInt(x ^ value));
	};
	for (; i + 4 <= count; i += 4) {
		step(row[i]); step(row[i + 1]); step(row[i + 2]); step(row[i + 3]);
	}
	for (; i < count; ++i) step(row[i]);
	return x;
}

template <class UInt, int Bits>
struct Core {
	static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && !std::is_same_v<UInt, bool>);
	static constexpr int digits = std::numeric_limits<UInt>::digits;
	static_assert(0 < Bits && Bits <= digits && digits <= 64);
	static constexpr UInt mask = UInt(UInt(~UInt(0)) >> (digits - Bits));
	std::array<UInt, Bits> row{};
	std::array<std::uint8_t, Bits> at{};
	int dimension = 0;
	static bool valid(UInt x) { return (x & mask) == x; }
	static int pivot(UInt x) { return int(std::bit_width(std::uint64_t(x))) - 1; }
	int insert(UInt x) {
		assert(valid(x));
		if (dimension == Bits) return -1;
		x = reduce_rows(x, row.data(), dimension);
		if (!x) return -1;
		row[dimension] = x; at[pivot(x)] = std::uint8_t(dimension + 1);
		return dimension++;
	}
	UInt minimize(UInt x) const {
		if (dimension == Bits) return UInt(x & UInt(~mask));
		x = reduce_rows(x, row.data(), dimension);
		return x;
	}
	UInt maximize(UInt x) const {
		if (dimension == Bits) return UInt(x | mask);
		x = reduce_rows<UInt, true>(x, row.data(), dimension);
		return x;
	}
};
}


#line 9 "math/linear_algebra/xor_basis.hpp"
#include <cstddef>
#line 11 "math/linear_algebra/xor_basis.hpp"
#include <initializer_list>
#line 13 "math/linear_algebra/xor_basis.hpp"
#include <optional>
#include <span>
#include <utility>
#include <vector>

template <class UInt, int Bits> class RollbackXorBasis;
template <class UInt, int Bits> class PrefixXorBasis;
template <class UInt, int Bits> class XorBasisWithWitness;

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class XorBasis {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	Core core;
	friend class RollbackXorBasis<UInt, Bits>;
	friend class PrefixXorBasis<UInt, Bits>;
	friend class XorBasisWithWitness<UInt, Bits>;

	XorBasis intersection_range(std::span<const UInt> values) const {
		XorBasis result;
		if (empty()) return result;
		if (rank() == Bits) { for (UInt value : values) result.insert(value); return result; }
		if constexpr (Bits <= 32) {
			std::array<std::uint64_t, Bits> work;
			int n = rank();
			for (int i = 0; i < n; ++i) work[i] = std::uint64_t(core.row[i]) << 32;
			for (UInt value : values) {
				assert(Core::valid(value));
				std::uint64_t x = (std::uint64_t(value) << 32) | value;
				x = xor_basis_detail::reduce_rows(x, work.data(), n);
				if (x >> 32) work[n++] = x;
				else result.insert(UInt(x));
			}
			return result;
		}
		std::array<UInt, Bits> row{};
		for (int i = 0; i < rank(); ++i) row[Core::pivot(core.row[i])] = core.row[i];
		std::array<UInt, Bits> source{};
		for (UInt value : values) {
			assert(Core::valid(value));
			UInt x = value, y = value;
			while (x) {
				int i = Core::pivot(x);
				if (!row[i]) { row[i] = x; source[i] = y; break; }
				x ^= row[i]; y ^= source[i];
			}
			if (!x) result.insert(y);
		}
		return result;
	}

  public:
	using value_type = UInt;
	using count_type = __uint128_t;
	static constexpr int bits = Bits;
	XorBasis() = default;
	XorBasis(std::initializer_list<UInt> values) { for (UInt x : values) insert(x); }
	explicit XorBasis(const std::vector<UInt> &values) { for (UInt x : values) insert(x); }
	int rank() const { return core.dimension; }
	bool empty() const { return rank() == 0; }
	count_type count() const { return count_type(1) << rank(); }
	void clear() { core = Core{}; }
	bool insert(UInt x) { return core.insert(x) >= 0; }
	UInt reduce(UInt x) const { return core.minimize(x); }
	bool contains(UInt x) const { return reduce(x) == 0; }
	UInt min_xor(UInt x = 0) const { return core.minimize(x); }
	UInt max_xor(UInt x = 0) const { return core.maximize(x); }
	count_type mex(UInt x = 0) const {
		if (!contains(x)) return 0;
		if (rank() == Bits) return count_type(1) << Bits;
		for (int i = 0; i < Bits; ++i) if (!core.at[i]) return count_type(1) << i;
		return count_type(1) << Bits;
	}
	std::optional<UInt> min_nonzero() const {
		if (empty()) return std::nullopt;
		return *std::min_element(core.row.begin(), core.row.begin() + rank());
	}
	std::vector<UInt> vectors() const {
		return {core.row.begin(), core.row.begin() + rank()};
	}
	XorBasis canonical() const {
		XorBasis result = *this;
		std::sort(result.core.row.begin(), result.core.row.begin() + rank());
		result.core.at.fill(0);
		for (int i = 0; i < rank(); ++i) {
			int p = Core::pivot(result.core.row[i]);
			result.core.at[p] = std::uint8_t(i + 1);
			for (int j = i + 1; j < rank(); ++j)
				if ((result.core.row[j] >> p) & 1) result.core.row[j] ^= result.core.row[i];
		}
		return result;
	}
	std::optional<UInt> coordinates(UInt x) const {
		UInt result = 0;
		for (int i = 0; i < rank(); ++i) if (UInt(x ^ core.row[i]) < x) {
			x ^= core.row[i]; result |= UInt(UInt(1) << i);
		}
		if (x) return std::nullopt;
		return result;
	}
	UInt combine(UInt coefficients) const {
		UInt result = 0;
		for (int i = 0; i < rank(); ++i) {
			if (coefficients & 1) result ^= core.row[i];
			coefficients >>= 1;
		}
		assert(!coefficients);
		return result;
	}
	std::optional<UInt> kth(count_type k, UInt x = 0) const {
		if (k >= count()) return std::nullopt;
		if (rank() == Bits) return UInt((x & UInt(~Core::mask)) | UInt(k));
		int remaining = rank();
		for (int i = Bits - 1; i >= 0; --i) if (core.at[i]) {
			--remaining;
			if (((x >> i) & 1) != ((k >> remaining) & 1)) x ^= core.row[core.at[i] - 1];
		}
		return x;
	}
	count_type count_less(count_type value, UInt x = 0) const {
		if (value >= (count_type(1) << Core::digits)) return count();
		UInt bound = UInt(value);
		UInt high = UInt(x & UInt(~Core::mask)), bound_high = UInt(bound & UInt(~Core::mask));
		if (high != bound_high) return high < bound_high ? count() : 0;
		if (rank() == Bits) return UInt(bound & Core::mask);
		count_type result = 0;
		int remaining = rank();
		for (int i = Bits - 1; i >= 0; --i) {
			bool target = (bound >> i) & 1;
			if (core.at[i]) {
				--remaining;
				if (target) result += count_type(1) << remaining;
				if (bool((x >> i) & 1) != target) x ^= core.row[core.at[i] - 1];
			} else if (bool((x >> i) & 1) != target) return result + (target ? count_type(1) << remaining : 0);
		}
		return result;
	}
	count_type count_leq(count_type value, UInt x = 0) const {
		if (value >= std::numeric_limits<UInt>::max()) return count();
		return count_less(value + 1, x);
	}
	count_type count_range(count_type lower, count_type upper, UInt x = 0) const {
		assert(lower <= upper);
		return count_less(upper, x) - count_less(lower, x);
	}
	std::optional<count_type> index_of(count_type value, UInt x = 0) const {
		if (value > std::numeric_limits<UInt>::max() || !contains(UInt(UInt(value) ^ x))) return std::nullopt;
		return count_less(value, x);
	}
	std::optional<UInt> lower_bound(count_type value, UInt x = 0) const { return kth(count_less(value, x), x); }
	std::optional<UInt> upper_bound(count_type value, UInt x = 0) const { return kth(count_leq(value, x), x); }
	std::optional<UInt> predecessor(count_type value, UInt x = 0) const {
		count_type k = count_less(value, x);
		return k ? kth(k - 1, x) : std::nullopt;
	}
	int merge(const XorBasis &other) {
		int before = rank();
		if (this != &other) for (int i = 0; i < other.rank(); ++i) {
			insert(other.core.row[i]);
			if (rank() == Bits) break;
		}
		return rank() - before;
	}
	bool is_subspace_of(const XorBasis &other) const {
		if (rank() > other.rank()) return false;
		for (int i = 0; i < rank(); ++i) if (!other.contains(core.row[i])) return false;
		return true;
	}
	bool operator==(const XorBasis &other) const { return rank() == other.rank() && is_subspace_of(other); }
	XorBasis intersection(const XorBasis &other) const {
		if (core.dimension == Bits || this == &other) return other;
		if (other.core.dimension == Bits) return *this;
		if (rank() < other.rank()) return other.intersection(*this);
		return intersection_range({other.core.row.data(), std::size_t(other.rank())});
	}
	template <class Range>
	requires requires(const Range &values) { std::span<const UInt>(values); }
	XorBasis intersection(const Range &values) const { return intersection_range(std::span<const UInt>(values)); }

	XorBasis orthogonal_complement() const {
		auto reduced = canonical();
		XorBasis result;
		for (int j = 0; j < Bits; ++j) if (!reduced.core.at[j]) {
			UInt x = UInt(UInt(1) << j);
			for (int i = 0; i < rank(); ++i) if ((reduced.core.row[i] >> j) & 1) x |= UInt(UInt(1) << Core::pivot(reduced.core.row[i]));
			result.insert(x);
		}
		return result;
	}
	XorBasis &operator|=(const XorBasis &other) { merge(other); return *this; }
	XorBasis &operator&=(const XorBasis &other) { return *this = intersection(other); }
	friend XorBasis operator|(XorBasis a, const XorBasis &b) { a.merge(b); return a; }
	friend XorBasis operator&(const XorBasis &a, const XorBasis &b) { return a.intersection(b); }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class RollbackXorBasis {
	XorBasis<UInt, Bits> current;
	std::vector<int> history;

  public:
	const XorBasis<UInt, Bits> &basis() const { return current; }
	std::size_t snapshot() const { return history.size(); }
	bool insert(UInt x) {
		history.push_back(-1);
		int i = current.core.insert(x);
		history.back() = i;
		return i >= 0;
	}
	void undo() {
		assert(!history.empty());
		int i = history.back(); history.pop_back();
		if (i >= 0) {
			current.core.at[current.core.pivot(current.core.row[i])] = 0;
			current.core.row[i] = 0; --current.core.dimension;
		}
	}
	void rollback(std::size_t token) { assert(token <= history.size()); while (history.size() > token) undo(); }
	void clear() { current.clear(); history.clear(); }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class PrefixXorBasis {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	std::array<UInt, Bits> row{};
	std::array<std::size_t, Bits> position{};
	std::size_t length = 0;

  public:
	std::size_t size() const { return length; }
	void clear() { row.fill(0); position.fill(0); length = 0; }
	bool push_back(UInt x) {
		assert(Core::valid(x) && length < std::numeric_limits<std::size_t>::max());
		std::size_t p = ++length;
		while (x) {
			int i = Core::pivot(x);
			if (!row[i]) { row[i] = x; position[i] = p; return true; }
			if (position[i] < p) { std::swap(position[i], p); std::swap(row[i], x); }
			x ^= row[i];
		}
		return false;
	}
	XorBasis<UInt, Bits> basis(std::size_t lower = 0) const {
		assert(lower <= length);
		XorBasis<UInt, Bits> result;
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) {
			int k = result.core.dimension++;
			result.core.row[k] = row[i]; result.core.at[i] = std::uint8_t(k + 1);
		}
		return result;
	}
	int rank(std::size_t lower = 0) const {
		assert(lower <= length);
		int result = 0;
		for (std::size_t p : position) result += p > lower;
		return result;
	}
	UInt min_xor(std::size_t lower = 0, UInt x = 0) const {
		assert(lower <= length);
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) x = std::min(x, UInt(x ^ row[i]));
		return x;
	}
	UInt max_xor(std::size_t lower = 0, UInt x = 0) const {
		assert(lower <= length);
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) x = std::max(x, UInt(x ^ row[i]));
		return x;
	}
	bool contains(std::size_t lower, UInt x) const { return min_xor(lower, x) == 0; }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class XorBasisWithWitness {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	XorBasis<UInt, Bits> current;
	std::array<UInt, Bits> source{};
	std::array<std::size_t, Bits> index{};
	std::size_t length = 0;

  public:
	const XorBasis<UInt, Bits> &basis() const { return current; }
	std::size_t size() const { return length; }
	void clear() { current.clear(); source.fill(0); length = 0; }
	bool insert(UInt x) {
		assert(Core::valid(x) && length < std::numeric_limits<std::size_t>::max());
		std::size_t id = length++;
		if (current.rank() == Bits) return false;
		UInt mask = UInt(UInt(1) << current.rank());
		for (int i = 0; i < current.rank(); ++i) if (UInt(x ^ current.core.row[i]) < x) {
			x ^= current.core.row[i]; mask ^= source[i];
		}
		if (!x) return false;
		int k = current.core.dimension++;
		index[k] = id; source[k] = mask; current.core.row[k] = x;
		current.core.at[Core::pivot(x)] = std::uint8_t(k + 1);
		return true;
	}
	std::optional<std::vector<std::size_t>> represent(UInt x) const {
		if (!Core::valid(x)) return std::nullopt;
		UInt mask = 0;
		for (int i = 0; i < current.rank(); ++i) if (UInt(x ^ current.core.row[i]) < x) {
			x ^= current.core.row[i]; mask ^= source[i];
		}
		if (x) return std::nullopt;
		std::vector<std::size_t> result;
		while (mask) {
			int i = std::countr_zero(std::uint64_t(mask));
			result.push_back(index[i]); mask &= UInt(mask - 1);
		}
		return result;
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


#line 7 "test/xor_basis_intersection.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		XorBasis<std::uint32_t, 30> a;
		std::array<std::uint32_t, 30> b;
		int n = 0, m = 0; input.read(n);
		for (int i = 0; i < n; ++i) { std::uint32_t x = 0; input.read(x); a.insert(x); }
		input.read(m);
		for (int i = 0; i < m; ++i) input.read(b[i]);
		auto result = a.intersection(std::span<const std::uint32_t>(b.data(), m)).vectors();
		output.write(result.size());
		for (auto x : result) { output.write(' '); output.write(x); }
		output.write('\n');
	}
}
