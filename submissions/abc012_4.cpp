#line 1 "graph/warshall_floyd.hpp"



#line 1 "components/warshall_floyd_kernel.hpp"



#line 1 "components/aligned_allocator.hpp"



#include <cstddef>
#include <limits>
#include <new>

template<class T, std::size_t Alignment = 64> struct AlignedAllocator {
	static_assert(Alignment >= alignof(T) && !(Alignment & (Alignment - 1)));
	using value_type = T;
	template<class U> struct rebind { using other = AlignedAllocator<U, Alignment>; };
	AlignedAllocator() = default;
	template<class U> AlignedAllocator(const AlignedAllocator<U, Alignment> &) {}
	T *allocate(std::size_t n) {
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
		return static_cast<T *>(::operator new(n * sizeof(T), std::align_val_t(Alignment)));
	}
	void deallocate(T *p, std::size_t) { ::operator delete(p, std::align_val_t(Alignment)); }
	template<class U> bool operator==(const AlignedAllocator<U, Alignment> &) const { return true; }
};


#line 5 "components/warshall_floyd_kernel.hpp"
#include <algorithm>
#include <cassert>
#line 8 "components/warshall_floyd_kernel.hpp"
#include <cstdint>
#line 10 "components/warshall_floyd_kernel.hpp"
#include <type_traits>
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace warshall_floyd_detail {
template<class T> using Storage = std::vector<T, AlignedAllocator<T>>;

template<class T> struct Unsigned { using type = std::make_unsigned_t<T>; };
template<> struct Unsigned<__int128_t> { using type = __uint128_t; };

inline int backend() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(WARSHALL_FLOYD_DISABLE_SIMD)
#ifndef WARSHALL_FLOYD_DISABLE_AVX512
	if (__builtin_cpu_supports("avx512f")) return 2;
#endif
	if (__builtin_cpu_supports("avx2")) return 1;
#endif
	return 0;
}

template<class T, int B, bool Negative, bool Diagonal>
bool scalar(T* out, const T* a, const T* b, T inf) {
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if (x == inf) continue;
			for (int j = 0; j < B; ++j) {
				if constexpr (Negative) {
					if (b[k * B + j] != inf) out[i * B + j] = std::min(out[i * B + j], T(x + b[k * B + j]));
				} else {
					using U = typename Unsigned<T>::type;
					out[i * B + j] = T(std::min(U(out[i * B + j]), U(x) + U(b[k * B + j])));
				}
			}
		}
	}
	return true;
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template<class T, int B, bool Negative, bool Diagonal>
__attribute__((target("avx512f")))
bool simple512(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 64 / sizeof(T);
	[[maybe_unused]] __m512i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm512_set1_epi32(inf);
	else infinity = _mm512_set1_epi64(inf);
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if constexpr (Negative) if (x == inf) continue;
			__m512i alpha;
			if constexpr (sizeof(T) == 4) alpha = _mm512_set1_epi32(x);
			else alpha = _mm512_set1_epi64(x);
			for (int j = 0; j < B; j += lanes) {
				auto value = _mm512_loadu_si512(out + i * B + j);
				auto beta = _mm512_loadu_si512(b + k * B + j);
				[[maybe_unused]] __mmask16 valid = 0;
				if constexpr (Negative && sizeof(T) == 4) valid = _mm512_cmpneq_epi32_mask(beta, infinity);
				if constexpr (Negative && sizeof(T) == 8) valid = _mm512_cmpneq_epi64_mask(beta, infinity);
				if constexpr (sizeof(T) == 4) {
					auto sum = _mm512_add_epi32(alpha, beta);
					if constexpr (Negative) value = _mm512_mask_min_epi32(value, valid, value, sum);
					else value = _mm512_min_epu32(value, sum);
				} else {
					auto sum = _mm512_add_epi64(alpha, beta);
					if constexpr (Negative) value = _mm512_mask_min_epi64(value, __mmask8(valid), value, sum);
					else value = _mm512_min_epu64(value, sum);
				}
				_mm512_storeu_si512(out + i * B + j, value);
			}
		}
	}
	return true;
}

template<class T, int B, bool Negative>
__attribute__((target("avx512f")))
void product512(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 64 / sizeof(T), rows = 4, columns = 2;
	[[maybe_unused]] __m512i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm512_set1_epi32(inf);
	else infinity = _mm512_set1_epi64(inf);
	for (int i = 0; i < B; i += rows) for (int j = 0; j < B; j += lanes * columns) {
		__m512i value[rows][columns];
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			value[r][q] = _mm512_loadu_si512(out + (i + r) * B + j + q * lanes);
		for (int k = 0; k < B; ++k) {
			__m512i beta[columns];
			[[maybe_unused]] __mmask16 valid[columns]{};
			for (int q = 0; q < columns; ++q) {
				beta[q] = _mm512_loadu_si512(b + k * B + j + q * lanes);
				__mmask16 mask = 0;
				if constexpr (Negative && sizeof(T) == 4) mask = _mm512_cmpneq_epi32_mask(beta[q], infinity);
				if constexpr (Negative && sizeof(T) == 8) mask = _mm512_cmpneq_epi64_mask(beta[q], infinity);
				valid[q] = mask;
			}
			for (int r = 0; r < rows; ++r) {
				T x = a[(i + r) * B + k];
				if constexpr (Negative) if (x == inf) continue;
				__m512i alpha;
				if constexpr (sizeof(T) == 4) alpha = _mm512_set1_epi32(x);
				else alpha = _mm512_set1_epi64(x);
				for (int q = 0; q < columns; ++q) {
					if constexpr (sizeof(T) == 4) {
						auto sum = _mm512_add_epi32(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm512_mask_min_epi32(value[r][q], valid[q], value[r][q], sum);
						else value[r][q] = _mm512_min_epu32(value[r][q], sum);
					} else {
						auto sum = _mm512_add_epi64(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm512_mask_min_epi64(value[r][q], __mmask8(valid[q]), value[r][q], sum);
						else value[r][q] = _mm512_min_epu64(value[r][q], sum);
					}
				}
			}
		}
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			_mm512_storeu_si512(out + (i + r) * B + j + q * lanes, value[r][q]);
	}
}

template<class T, int B, bool Negative, bool Diagonal>
__attribute__((target("avx2")))
bool simple256(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 32 / sizeof(T);
	[[maybe_unused]] __m256i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm256_set1_epi32(inf);
	else infinity = _mm256_set1_epi64x(inf);
	for (int k = 0; k < B; ++k) {
		if constexpr (Diagonal && Negative) if (out[k * B + k] < 0) return false;
		for (int i = 0; i < B; ++i) {
			T x = a[i * B + k];
			if constexpr (Negative) if (x == inf) continue;
			__m256i alpha;
			if constexpr (sizeof(T) == 4) alpha = _mm256_set1_epi32(x);
			else alpha = _mm256_set1_epi64x(x);
			for (int j = 0; j < B; j += lanes) {
				auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out + i * B + j));
				auto beta = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + k * B + j));
				[[maybe_unused]] __m256i valid = _mm256_setzero_si256();
				if constexpr (Negative && sizeof(T) == 4) valid = _mm256_cmpeq_epi32(beta, infinity);
				if constexpr (Negative && sizeof(T) == 8) valid = _mm256_cmpeq_epi64(beta, infinity);
				if constexpr (sizeof(T) == 4) {
					auto sum = _mm256_add_epi32(alpha, beta);
					if constexpr (Negative) value = _mm256_min_epi32(value, _mm256_blendv_epi8(sum, infinity, valid));
					else value = _mm256_min_epu32(value, sum);
				} else {
					auto sum = _mm256_add_epi64(alpha, beta);
					if constexpr (Negative) {
						sum = _mm256_blendv_epi8(sum, infinity, valid);
						value = _mm256_blendv_epi8(value, sum, _mm256_cmpgt_epi64(value, sum));
					} else {
						auto choose = _mm256_andnot_si256(_mm256_cmpgt_epi64(_mm256_setzero_si256(), sum), _mm256_cmpgt_epi64(value, sum));
						value = _mm256_blendv_epi8(value, sum, choose);
					}
				}
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(out + i * B + j), value);
			}
		}
	}
	return true;
}

template<class T, int B, bool Negative>
__attribute__((target("avx2")))
void product256(T* out, const T* a, const T* b, T inf) {
	constexpr int lanes = 32 / sizeof(T), rows = 4, columns = 2;
	[[maybe_unused]] __m256i infinity;
	if constexpr (sizeof(T) == 4) infinity = _mm256_set1_epi32(inf);
	else infinity = _mm256_set1_epi64x(inf);
	for (int i = 0; i < B; i += rows) for (int j = 0; j < B; j += lanes * columns) {
		__m256i value[rows][columns];
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			value[r][q] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out + (i + r) * B + j + q * lanes));
		for (int k = 0; k < B; ++k) {
			__m256i beta[columns];
			[[maybe_unused]] __m256i valid[columns]{};
			for (int q = 0; q < columns; ++q) {
				beta[q] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + k * B + j + q * lanes));
				__m256i mask = _mm256_setzero_si256();
				if constexpr (Negative && sizeof(T) == 4) mask = _mm256_cmpeq_epi32(beta[q], infinity);
				if constexpr (Negative && sizeof(T) == 8) mask = _mm256_cmpeq_epi64(beta[q], infinity);
				valid[q] = mask;
			}
			for (int r = 0; r < rows; ++r) {
				T x = a[(i + r) * B + k];
				if constexpr (Negative) if (x == inf) continue;
				__m256i alpha;
				if constexpr (sizeof(T) == 4) alpha = _mm256_set1_epi32(x);
				else alpha = _mm256_set1_epi64x(x);
				for (int q = 0; q < columns; ++q) {
					if constexpr (sizeof(T) == 4) {
						auto sum = _mm256_add_epi32(alpha, beta[q]);
						if constexpr (Negative) value[r][q] = _mm256_min_epi32(value[r][q], _mm256_blendv_epi8(sum, infinity, valid[q]));
						else value[r][q] = _mm256_min_epu32(value[r][q], sum);
					} else {
						auto sum = _mm256_add_epi64(alpha, beta[q]);
						if constexpr (Negative) {
							sum = _mm256_blendv_epi8(sum, infinity, valid[q]);
							value[r][q] = _mm256_blendv_epi8(value[r][q], sum, _mm256_cmpgt_epi64(value[r][q], sum));
						} else {
							auto choose = _mm256_andnot_si256(_mm256_cmpgt_epi64(_mm256_setzero_si256(), sum), _mm256_cmpgt_epi64(value[r][q], sum));
							value[r][q] = _mm256_blendv_epi8(value[r][q], sum, choose);
						}
					}
				}
			}
		}
		for (int r = 0; r < rows; ++r) for (int q = 0; q < columns; ++q)
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(out + (i + r) * B + j + q * lanes), value[r][q]);
	}
}

#endif

template<class T, int B, bool Negative, int Backend>
struct Kernel {
	static bool close(T* out, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) return simple512<T, B, Negative, true>(out, out, out, inf);
		if constexpr (Backend == 1) return simple256<T, B, Negative, true>(out, out, out, inf);
#endif
		return scalar<T, B, Negative, true>(out, out, out, inf);
	}
	static void edge(T* out, const T* a, const T* b, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) { simple512<T, B, Negative, false>(out, a, b, inf); return; }
		if constexpr (Backend == 1) { simple256<T, B, Negative, false>(out, a, b, inf); return; }
#endif
		scalar<T, B, Negative, false>(out, a, b, inf);
	}
	static void product(T* out, const T* a, const T* b, T inf) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Backend == 2) { product512<T, B, Negative>(out, a, b, inf); return; }
		if constexpr (Backend == 1) { product256<T, B, Negative>(out, a, b, inf); return; }
#endif
		scalar<T, B, Negative, false>(out, a, b, inf);
	}
};

template<class T, int B, bool Negative, int Backend>
bool solve(T* data, int blocks, T inf) {
	using K = Kernel<T, B, Negative, Backend>;
	auto tile = [&](int i, int j) { return data + (std::size_t(i) * blocks + j) * B * B; };
	auto product = [&](int row, int col, int pivot, int rows, int cols, int inner) {
		for (int k = pivot; k < pivot + inner; ++k)
			for (int i = row; i < row + rows; ++i)
				for (int j = col; j < col + cols; ++j) K::product(tile(i, j), tile(i, k), tile(k, j), inf);
	};
	constexpr int macro = 16;
	for (int base = 0; base < blocks; base += macro) {
		int size = std::min(macro, blocks - base);
		for (int k = base; k < base + size; ++k) {
			if (!K::close(tile(k, k), inf)) return false;
			for (int j = base; j < base + size; ++j) if (j != k) K::edge(tile(k, j), tile(k, k), tile(k, j), inf);
			for (int i = base; i < base + size; ++i) if (i != k) K::edge(tile(i, k), tile(i, k), tile(k, k), inf);
			for (int i = base; i < base + size; ++i) if (i != k)
				for (int j = base; j < base + size; ++j) if (j != k) K::product(tile(i, j), tile(i, k), tile(k, j), inf);
		}
		for (int j = 0; j < blocks; j += macro) if (j != base)
			product(base, j, base, size, std::min(macro, blocks - j), size);
		for (int i = 0; i < blocks; i += macro) if (i != base)
			product(i, base, base, std::min(macro, blocks - i), size, size);
		for (int i = 0; i < blocks; i += macro) if (i != base)
			for (int j = 0; j < blocks; j += macro) if (j != base)
				product(i, j, base, std::min(macro, blocks - i), std::min(macro, blocks - j), size);
	}
	return true;
}

template<class T, int B, bool Negative>
bool run(T* data, int blocks, T inf, int selected = backend()) {
	if constexpr (sizeof(T) <= 8) {
		if (selected == 2) return solve<T, B, Negative, 2>(data, blocks, inf);
		if (selected == 1) return solve<T, B, Negative, 1>(data, blocks, inf);
	}
	return solve<T, B, Negative, 0>(data, blocks, inf);
}
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif


#line 5 "graph/warshall_floyd.hpp"
#include <stdexcept>

template<class T = std::int64_t>
class WarshallFloyd {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) == 4 || sizeof(T) == 8));
	static constexpr int block = 32;
	int n_ = 0, blocks_ = 0;
	T infinity_ = std::numeric_limits<T>::max();
	warshall_floyd_detail::Storage<T> data_;
	static int checked_size(std::size_t n) {
		if (n > std::size_t(std::numeric_limits<int>::max())) throw std::length_error("Warshall-Floyd size");
		return int(n);
	}
	std::size_t index(int i, int j) const {
		return (std::size_t(i / block) * blocks_ + j / block) * block * block + i % block * block + j % block;
	}
	template<class U> bool run_as(bool negative) {
		const U inf = std::numeric_limits<U>::max();
		warshall_floyd_detail::Storage<U> work(data_.size());
		for (std::size_t i = 0; i < data_.size(); ++i) work[i] = data_[i] == infinity_ ? inf : U(data_[i]);
		bool success = negative ? warshall_floyd_detail::run<U, block, true>(work.data(), blocks_, inf)
			: warshall_floyd_detail::run<U, block, false>(work.data(), blocks_, inf);
		if (success)
			for (U value : work) if (value != inf && (__int128_t(value) < std::numeric_limits<T>::lowest() || __int128_t(value) >= infinity_))
				throw std::overflow_error("Warshall-Floyd distance");
		for (std::size_t i = 0; i < data_.size(); ++i) {
			if (work[i] == inf) data_[i] = infinity_;
			else data_[i] = T(std::clamp(__int128_t(work[i]), __int128_t(std::numeric_limits<T>::lowest()), __int128_t(infinity_) - 1));
		}
		return success;
	}

  public:
	using value_type = T;
	explicit WarshallFloyd(int n = 0, T inf = std::numeric_limits<T>::max()) : n_(n), infinity_(inf) {
		if (n < 0 || inf <= 0) throw std::invalid_argument("Warshall-Floyd shape or infinity");
		blocks_ = n / block + (n % block != 0);
		std::size_t padded = std::size_t(blocks_) * block;
		data_.assign(padded * padded, infinity_);
		for (std::size_t i = 0; i < padded; ++i) data_[index(int(i), int(i))] = 0;
	}
	explicit WarshallFloyd(const std::vector<std::vector<T>>& distance, T inf = std::numeric_limits<T>::max())
		: WarshallFloyd(checked_size(distance.size()), inf) {
		for (int i = 0; i < n_; ++i) {
			if (distance[i].size() != std::size_t(n_)) throw std::invalid_argument("Warshall-Floyd matrix");
			for (int j = 0; j < n_; ++j) {
				if (distance[i][j] == infinity_) continue;
				add_edge(i, j, distance[i][j]);
			}
		}
	}
	int size() const { return n_; }
	T infinity() const { return infinity_; }
	T operator()(int i, int j) const {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		return data_[index(i, j)];
	}
	void add_edge(int from, int to, T weight) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		if (weight >= infinity_) throw std::invalid_argument("Warshall-Floyd edge");
		auto& value = data_[index(from, to)];
		value = std::min(value, weight);
	}
	bool solve() {
		__int128_t maximum = 0;
		std::size_t finite = 0;
		bool negative = false;
		for (T value : data_) if (value != infinity_) {
			++finite;
			__int128_t x = value;
			negative |= x < 0;
			maximum = std::max(maximum, x < 0 ? -x : x);
		}
		__int128_t bound = 2 * (__int128_t(n_) + 1) * maximum;
		bool complete = finite == std::size_t(n_) * n_ + std::size_t(blocks_) * block - n_;
		if constexpr (sizeof(T) == 8) {
#ifndef WARSHALL_FLOYD_DISABLE_NARROWING
			__int128_t limit = std::min(__int128_t(infinity_), __int128_t(std::numeric_limits<std::int32_t>::max()));
			if (negative ? bound < limit : maximum < limit && (complete || (__int128_t(n_) - 1) * maximum < limit))
				return run_as<std::int32_t>(negative);
#endif
		}
		if (negative ? bound >= infinity_ : !complete && (__int128_t(n_) - 1) * maximum >= infinity_) {
			if constexpr (sizeof(T) == 4) {
				if (bound < std::numeric_limits<std::int64_t>::max()) return run_as<std::int64_t>(negative);
			}
			return run_as<__int128_t>(negative);
		}
		return negative ? warshall_floyd_detail::run<T, block, true>(data_.data(), blocks_, infinity_)
			: warshall_floyd_detail::run<T, block, false>(data_.data(), blocks_, infinity_);
	}
	std::vector<std::vector<T>> distances() const {
		std::vector<std::vector<T>> result(n_, std::vector<T>(n_));
		for (int i = 0; i < n_; ++i) for (int j = 0; j < n_; ++j) result[i][j] = data_[index(i, j)];
		return result;
	}
};

template<class T>
bool warshall_floyd(std::vector<std::vector<T>>& distance, T inf = std::numeric_limits<T>::max()) {
	WarshallFloyd<T> work(distance, inf);
	bool success = work.solve();
	distance = work.distances();
	return success;
}


#line 1 "tree/tree_diameter.hpp"



#line 9 "tree/tree_diameter.hpp"
#include <utility>
#line 11 "tree/tree_diameter.hpp"

template<class T = std::int64_t> class TreeDiameter {
	int n_;

  public:
	struct Edge { int from, to; T weight; };
	struct Result { T distance{}; std::vector<int> path; };

  private:
	std::vector<Edge> edges_;

  public:
	explicit TreeDiameter(int n = 0) : n_(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges_.reserve(n ? n - 1 : 0);
	}
	TreeDiameter(const TreeDiameter &) = default;
	TreeDiameter &operator=(const TreeDiameter &other) {
		if (this != &other) { TreeDiameter copy(other); swap(copy); }
		return *this;
	}
	TreeDiameter(TreeDiameter &&other) noexcept : n_(std::exchange(other.n_, 0)), edges_(std::move(other.edges_)) {}
	TreeDiameter &operator=(TreeDiameter &&other) noexcept {
		if (this != &other) n_ = std::exchange(other.n_, 0), edges_ = std::move(other.edges_);
		return *this;
	}
	void swap(TreeDiameter &other) noexcept {
		std::swap(n_, other.n_);
		edges_.swap(other.edges_);
	}
	int size() const { return n_; }
	int edge_count() const { return static_cast<int>(edges_.size()); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(n_ ? n_ - 1 : 0));
		edges_.reserve(m);
	}
	int add_edge(int u, int v, T weight = T(1)) {
		assert(0 <= u && u < n_ && 0 <= v && v < n_ && u != v);
		assert(edges_.size() < std::size_t(n_ - 1));
		int id = edge_count();
		edges_.push_back({u, v, std::move(weight)});
		return id;
	}
	const std::vector<Edge> &edges() const { return edges_; }
	void clear() { edges_.clear(); }
	Result solve() const {
		assert(edges_.size() == std::size_t(n_ ? n_ - 1 : 0));
		Result result;
		if (!n_) return result;
		struct State { int degree = 0, link = 0, parent = -1, endpoint = 0; T best{}; };
		std::vector<State> state(n_);
		for (int id = 0; id < edge_count(); ++id) {
			const auto &e = edges_[id];
			++state[e.from].degree, ++state[e.to].degree;
			state[e.from].link ^= id, state[e.to].link ^= id;
		}
		std::vector<int> order;
		order.reserve(n_ - 1);
		for (int v = 0; v < n_; ++v) {
			state[v].endpoint = v;
			if (v && state[v].degree == 1) order.push_back(v);
		}
		int from = 0, to = 0, middle = 0;
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i], id = state[v].link;
			const auto &e = edges_[id];
			int p = e.from ^ e.to ^ v;
			state[v].parent = p;
			T candidate = state[v].best + e.weight;
			T joined = state[p].best + candidate;
			if (result.distance < joined) {
				result.distance = std::move(joined);
				from = state[v].endpoint, to = state[p].endpoint, middle = p;
			}
			if (state[p].best < candidate) state[p].best = std::move(candidate), state[p].endpoint = state[v].endpoint;
			state[p].link ^= id;
			if (--state[p].degree == 1 && p) order.push_back(p);
		}
		assert(order.size() == std::size_t(n_ - 1));
		for (int v = from; v != middle; v = state[v].parent) result.path.push_back(v);
		result.path.push_back(middle);
		std::size_t first = result.path.size();
		for (int v = to; v != middle; v = state[v].parent) result.path.push_back(v);
		std::reverse(result.path.begin() + first, result.path.end());
		return result;
	}
};


#line 1 "components/fast_io.hpp"



#line 5 "components/fast_io.hpp"
#include <array>
#include <bit>
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


#line 4 "abc012_4.cpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0;
	if (!in.read(n)) return 0;
	in.read(m);
	if (m == n - 1) {
		TreeDiameter<int> tree(n);
		std::vector<std::vector<std::pair<int, int>>> graph(n);
		for (int i = 0; i < m; ++i) {
			int a = 0, b = 0, t = 0;
			in.read(a); in.read(b); in.read(t);
			--a; --b;
			tree.add_edge(a, b, t);
			graph[a].emplace_back(b, t);
			graph[b].emplace_back(a, t);
		}
		auto result = tree.solve();
		int prefix = 0, answer = result.distance;
		for (std::size_t i = 0; i < result.path.size(); ++i) {
			answer = std::min(answer, std::max(prefix, result.distance - prefix));
			if (i + 1 < result.path.size())
				for (auto [to, weight] : graph[result.path[i]])
					if (to == result.path[i + 1]) { prefix += weight; break; }
		}
		out.println(answer);
	} else {
		WarshallFloyd<int> graph(n);
		for (int i = 0; i < m; ++i) {
			int a = 0, b = 0, t = 0;
			in.read(a); in.read(b); in.read(t);
			--a; --b;
			graph.add_edge(a, b, t);
			graph.add_edge(b, a, t);
		}
		graph.solve();
		int answer = graph.infinity();
		for (int i = 0; i < n; ++i) {
			int eccentricity = 0;
			for (int j = 0; j < n && eccentricity < answer; ++j)
				eccentricity = std::max(eccentricity, graph(i, j));
			answer = std::min(answer, eccentricity);
		}
		out.println(answer);
	}
}
