#line 1 "test/gcd_convolution.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/gcd_convolution"
#line 1 "convolution/gcd_lcm_convolution.hpp"



#line 1 "components/gcd_convolution_mint.hpp"



#line 1 "components/divisor_zeta.hpp"



#line 1 "math/prime_sieve.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <utility>
#include <type_traits>
#if defined(__BMI2__) && defined(__x86_64__)
#include <immintrin.h>
#endif
#include <vector>

namespace prime_sieve_detail {
struct Sample {
	std::size_t skip, stride;
	std::size_t &count;
	std::vector<std::size_t> &values;
	void operator()(std::size_t p) {
		if (skip) --skip;
		else values.push_back(p), skip = stride - 1;
		++count;
	}
	template<class At> void word(std::uint64_t word, At at) {
		unsigned remaining = std::popcount(word);
		count += remaining;
		if (skip >= remaining) { skip -= remaining; return; }
		do {
			const unsigned rank = unsigned(skip);
			if (rank) {
#if defined(__BMI2__) && defined(__x86_64__)
				word &= -_pdep_u64(std::uint64_t(1) << rank, word);
#else
				for (unsigned i = 0; i < rank; ++i) word &= word - 1;
#endif
			}
			const unsigned bit = std::countr_zero(word);
			values.push_back(at(bit));
			word &= word - 1;
			remaining -= rank + 1;
			skip = stride - 1;
		} while (skip < remaining);
		skip -= remaining;
	}
};
template<class F> inline constexpr bool sampled = std::is_same_v<F, Sample>;
}

inline std::vector<std::size_t> prime_sieve(std::size_t n);

template<class F> void for_each_prime(std::size_t n, F emit) {
	if (n <= 2) return;
	emit(std::size_t(2));
	if (n <= 65537) {
		const std::size_t size = n / 2;
		std::vector<std::uint8_t> prime(size, 1);
		prime[0] = 0;
		for (std::size_t p = 3; p <= (n - 1) / p; p += 2) {
			if (!prime[p / 2]) continue;
			for (std::size_t j = p * p / 2; j < size; j += p) prime[j] = 0;
		}
		std::size_t i = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for (; i + 8 <= size; i += 8) {
			std::uint64_t word;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			if constexpr (prime_sieve_detail::sampled<F>) emit.word(word, [&](unsigned bit) { return 2 * (i + bit / 8) + 1; });
			else {
				while (word) {
					emit(2 * (i + (std::countr_zero(word) >> 3)) + 1);
					word &= word - 1;
				}
			}
		}
#endif
		for (; i < size; ++i) if (prime[i]) emit(2 * i + 1);
		return;
	}
	emit(3), emit(5);
	constexpr std::array<unsigned, 8> residues{1, 7, 11, 13, 17, 19, 23, 29};
	constexpr auto index = [&] {
		std::array<unsigned, 30> index{};
		for (unsigned i = 0; i < 8; ++i) index[residues[i]] = i;
		return index;
	}();
	constexpr std::size_t period = 7 * 11 * 13;
	static const auto pattern = [&] {
		std::array<std::uint8_t, period> pattern{};
		for (std::size_t i = 0; i < period; ++i)
			for (unsigned d = 0; d < 8; ++d) {
				const auto x = 30 * i + residues[d];
				pattern[i] |= std::uint8_t(bool(x % 7 && x % 11 && x % 13) << d);
			}
		return pattern;
	}();
	std::size_t root = std::sqrt(static_cast<long double>(n - 1));
	while (root > (n - 1) / root) --root;
	while (root + 1 <= (n - 1) / (root + 1)) ++root;
	const std::size_t block_size = (std::max(period * 32, root) + period - 1) / period * period;
	const auto primes = prime_sieve(root + 1);
	struct State {
		std::size_t p;
		std::array<std::size_t, 8> next;
		std::array<std::uint8_t, 8> mask;
	};
	std::vector<State> state;
	state.reserve(primes.size());
	for (const auto p : primes) {
		if (p < 17) continue;
		State current;
		current.p = p;
		for (unsigned d = 0; d < 8; ++d) {
			const std::size_t k = p + (residues[d] + 30 - p % 30) % 30;
			const __uint128_t x = __uint128_t(p) * k;
			current.next[d] = x / 30;
			current.mask[d] = std::uint8_t(~(1u << index[x % 30]));
		}
		state.push_back(current);
	}
	const std::size_t size = (n - 1) / 30 + 1;
	std::vector<std::uint8_t> prime((block_size + 7) / 8 * 8);
	constexpr auto offsets = [&] {
		std::array<unsigned, 64> offsets{};
		for (unsigned i = 0; i < 64; ++i) offsets[i] = 30 * (i / 8) + residues[i % 8];
		return offsets;
	}();
	for (std::size_t base = 0; base < size;) {
		const std::size_t length = std::min(block_size, size - base);
		for (std::size_t i = 0; i < length; i += period)
			std::copy_n(pattern.data(), std::min(period, length - i), prime.data() + i);
		if (base == 0) {
			prime[0] &= ~1u;
			prime[0] |= (1u << 1) | (1u << 2) | (1u << 3);
		}
		for (auto &current : state) {
			const auto p = current.p;
			for (unsigned d = 0; d < 8; ++d) {
				std::size_t j = current.next[d];
				const auto mask = current.mask[d];
				for (; j + 3 * p < length; j += 4 * p) {
					prime[j] &= mask;
					prime[j + p] &= mask;
					prime[j + 2 * p] &= mask;
					prime[j + 3 * p] &= mask;
				}
				for (; j < length; j += p) prime[j] &= mask;
				current.next[d] = j - length;
			}
		}
		if (base + length == size) {
			const auto remaining = n - 30 * (size - 1);
			for (unsigned d = 0; d < 8; ++d) if (residues[d] >= remaining) prime[length - 1] &= ~(1u << d);
		}
		std::fill(prime.begin() + length, prime.begin() + ((length + 7) / 8 * 8), 0);
		for (std::size_t i = 0; i < length; i += 8) {
			std::uint64_t word = 0;
			std::memcpy(&word, prime.data() + i, sizeof(word));
			if constexpr (std::endian::native == std::endian::big) word = std::byteswap(word);
			else if constexpr (std::endian::native != std::endian::little) {
				word = 0;
				for (unsigned d = 0; d < 8; ++d) word |= std::uint64_t(prime[i + d]) << (8 * d);
			}
			if constexpr (prime_sieve_detail::sampled<F>) emit.word(word, [&](unsigned bit) { return 30 * (base + i) + offsets[bit]; });
			else {
				while (word) {
					const auto x = 30 * (base + i) + offsets[std::countr_zero(word)];
					emit(x);
					word &= word - 1;
				}
			}
		}
		base += length;
	}
}

inline std::vector<std::size_t> prime_sieve(std::size_t n) {
	std::vector<std::size_t> result;
	if (n >= 1024) result.reserve(std::size_t(double(n) / (std::log(double(n)) - 1.1)));
	for_each_prime(n, [&](std::size_t p) { result.push_back(p); });
	return result;
}

inline std::pair<std::size_t, std::vector<std::size_t>> prime_sieve_sample(std::size_t n, std::size_t stride, std::size_t offset = 0) {
	assert(stride > 0);
	std::pair<std::size_t, std::vector<std::size_t>> result;
	if (stride <= 4) {
		std::size_t skip = offset;
		for_each_prime(n, [&](std::size_t p) {
			if (skip) --skip;
			else result.second.push_back(p), skip = stride - 1;
			++result.first;
		});
		return result;
	}
	for_each_prime(n, prime_sieve_detail::Sample{offset, stride, result.first, result.second});
	return result;
}


#line 9 "components/divisor_zeta.hpp"
#include <span>
#line 11 "components/divisor_zeta.hpp"

namespace divisor_zeta_detail {

struct Stage {
	std::vector<std::uint32_t> divisors;
	Stage(std::uint32_t n, std::span<const std::size_t> primes) {
		std::vector<std::uint64_t> marked(std::size_t(n) / 64 + 1);
		auto visit = [&](auto &&self, std::uint32_t value, std::size_t first) -> void {
			std::uint32_t limit = n / value;
			for (std::size_t j = first; j < primes.size() && primes[j] <= limit; ++j) {
				std::uint32_t next = value * std::uint32_t(primes[j]);
				marked[next >> 6] |= std::uint64_t(1) << (next & 63);
				self(self, next, j);
			}
		};
		visit(visit, 1, 0);
		for (std::size_t i = 0; i < marked.size(); ++i)
			for (auto bits = marked[i]; bits; bits &= bits - 1)
				divisors.push_back(std::uint32_t(i * 64 + std::countr_zero(bits)));
	}
};

inline std::vector<Stage> make_plan(std::uint32_t n, const std::vector<std::size_t> &primes, std::size_t &small) {
	auto first = std::upper_bound(primes.begin(), primes.end(), 31);
	small = std::size_t(first - primes.begin());
	std::vector<Stage> stages;
	auto append = [&](std::size_t bound) {
		auto last = std::upper_bound(first, primes.end(), bound);
		if (first != last) stages.emplace_back(n, std::span(first, last));
		first = last;
	};
	std::uint64_t bound = 199;
	append(bound);
	while (bound * bound <= (n - 1) / (bound * bound)) {
		bound *= bound;
		append(std::size_t(bound));
	}
	if (first != primes.end()) stages.emplace_back(n, std::span(first, primes.end()));
	return stages;
}

template <bool inverse, bool multiples, class F>
void for_each(std::size_t n, const std::vector<std::size_t> &ps, std::size_t count, F update) {
	if (!n) return;
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto apply = [&](std::size_t i) { update(multiples ? i : i * p, multiples ? i * p : i); };
		if constexpr (inverse == multiples) {
			std::size_t i = 1;
			for (; i + 3 <= end; i += 4) apply(i), apply(i + 1), apply(i + 2), apply(i + 3);
			for (; i <= end; ++i) apply(i);
		} else {
			std::size_t i = end;
			for (; i >= 4; i -= 4) apply(i), apply(i - 1), apply(i - 2), apply(i - 3);
			for (; i; --i) apply(i);
		}
	}
}

}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a, const std::vector<std::size_t> &ps) {
	divisor_zeta_detail::for_each<inverse, multiples>(a.size(), ps, ps.size(), [&](std::size_t to, std::size_t from) {
		if constexpr (inverse) a[to] -= a[from];
		else a[to] += a[from];
	});
}

template <bool inverse = false, bool multiples = false, class T>
void divisor_zeta(std::vector<T> &a) { divisor_zeta<inverse, multiples>(a, prime_sieve(a.size())); }


#line 11 "components/gcd_convolution_mint.hpp"

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace mint_detail { struct Mint; }

namespace gcd_convolution_mint_detail {

#if defined(__x86_64__) || defined(__i386__)
template <bool inverse, class T>
__attribute__((target("avx2"), always_inline)) inline
void update8(T *a, T *b, std::size_t i, std::size_t p) {
	const __m256i mod = _mm256_set1_epi32(2 * T::mod());
	auto gather = [&](const T *s) __attribute__((target("avx2"))) {
		return _mm256_setr_epi32(s[0].v, s[p].v, s[2 * p].v, s[3 * p].v,
								s[4 * p].v, s[5 * p].v, s[6 * p].v, s[7 * p].v);
	};
	__m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)), y = gather(a + i * p);
	if constexpr (inverse) {
		x = _mm256_sub_epi32(x, y);
		x = _mm256_min_epu32(x, _mm256_add_epi32(x, mod));
	} else {
		x = _mm256_add_epi32(x, y);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, mod));
	}
	_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), x);
	if constexpr (!inverse) {
		x = _mm256_add_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), gather(b + i * p));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, mod));
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(b + i), x);
	}
}

template <bool inverse, class T>
__attribute__((target("avx2")))
void prime_pass(T *a, T *b, std::size_t n, const std::vector<std::size_t> &ps, std::size_t count) {
	for (std::size_t j = 0; j < count; ++j) {
		const std::size_t p = ps[j], end = (n - 1) / p;
		auto scalar = [&](std::size_t i) {
			if constexpr (inverse) a[i] -= a[i * p];
			else a[i] += a[i * p], b[i] += b[i * p];
		};
		if constexpr (inverse) {
			std::size_t i = 1;
			for (; i <= end && i < 8; ++i) scalar(i);
			for (; i + 7 <= end; i += 8) update8<inverse>(a, b, i, p);
			for (; i <= end; ++i) scalar(i);
		} else {
			std::size_t i = end;
			for (; i >= 15; i -= 8) update8<inverse>(a, b, i - 7, p);
			for (; i; --i) scalar(i);
		}
	}
}
#endif

template <bool inverse, class T>
void transform(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps, std::size_t count) {
#if defined(__x86_64__) || defined(__i386__)
	if (__builtin_cpu_supports("avx2")) {
		prime_pass<inverse>(a.data(), b.data(), a.size(), ps, count);
		return;
	}
#endif
	divisor_zeta_detail::for_each<inverse, true>(a.size(), ps, count, [&](std::size_t to, std::size_t from) {
		if constexpr (inverse) a[to] -= a[from];
		else a[to] += a[from], b[to] += b[from];
	});
}

template <bool inverse, class T>
void large_multiples(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps,
					 std::size_t first) {
	if (first == ps.size()) return;
	constexpr std::uint64_t twice = 2 * T::mod();
	const std::size_t n = a.size() - 1;
	for (std::size_t i = 1; i <= n / ps[first]; ++i) {
		const auto end = std::upper_bound(ps.begin() + first, ps.end(), n / i);
		std::uint64_t x = inverse ? 0 : a[i].v, y = inverse ? a[i].v : b[i].v;
		for (auto p = ps.begin() + first; p != end; ++p) {
			x += a[i * *p].v;
			if constexpr (!inverse) y += b[i * *p].v;
		}
		if constexpr (inverse) a[i] = T::word(y % twice) - T::word(x % twice);
		else {
			a[i] = T::word(x % twice), b[i] = T::word(y % twice);
		}
	}
}

#if defined(__x86_64__) || defined(__i386__)
template <bool inverse, class T>
void partial_transform(std::vector<T> &a, std::vector<T> &b, const divisor_zeta_detail::Stage &stage, std::uint32_t last) {
	constexpr std::uint64_t twice = 2 * T::mod();
	const auto &ds = stage.divisors;
	auto evaluate = [&](std::uint32_t i) {
		std::uint32_t limit = last / i;
		std::uint64_t x = 0, y = 0, xx = 0, yy = 0;
		std::size_t j = 0;
		for (; j + 1 < ds.size() && ds[j + 1] <= limit; j += 2) {
			std::size_t p = std::size_t(i) * ds[j], q = std::size_t(i) * ds[j + 1];
			x += a[p].v, xx += a[q].v;
			if constexpr (!inverse) y += b[p].v, yy += b[q].v;
		}
		if (j < ds.size() && ds[j] <= limit) {
			std::size_t p = std::size_t(i) * ds[j];
			x += a[p].v;
			if constexpr (!inverse) y += b[p].v;
		}
		if constexpr (inverse) a[i] -= T::word(std::uint32_t((x + xx) % twice));
		else {
			a[i] = T::word(std::uint32_t((x + xx + a[i].v) % twice));
			b[i] = T::word(std::uint32_t((y + yy + b[i].v) % twice));
		}
	};
	std::uint32_t end = last / ds[0];
	if constexpr (inverse) for (std::uint32_t i = end; i; --i) evaluate(i);
	else for (std::uint32_t i = 1; i <= end; ++i) evaluate(i);
}

template <bool inverse, class T>
__attribute__((target("avx2")))
void stage_pass(T *a, T *b, std::uint32_t first, std::uint32_t last, std::uint32_t p) {
	std::uint32_t i = first;
	for (; i + 7 <= last; i += 8) update8<inverse>(a, b, i, p);
	for (; i <= last; ++i) {
		if constexpr (inverse) a[i] -= a[std::size_t(i) * p];
		else a[i] += a[std::size_t(i) * p], b[i] += b[std::size_t(i) * p];
	}
}

template <bool inverse, class T>
void stage_transform(std::vector<T> &a, std::vector<T> &b, const divisor_zeta_detail::Stage &stage,
					 std::uint32_t block, std::uint32_t split) {
	constexpr std::uint64_t twice = 2 * T::mod();
	const auto &ds = stage.divisors;
	if (ds.empty()) return;
	std::uint32_t n = std::uint32_t(a.size() - 1);
	std::size_t large = std::size_t(std::upper_bound(ds.begin(), ds.end(), split) - ds.begin());
	std::uint64_t large_first = large < ds.size() ? ds[large] : std::uint64_t(n) + 1;
	std::vector<std::size_t> cursor(std::size_t(n / large_first) + 1, large);
	std::vector<std::uint32_t> next(large);
	if constexpr (!inverse) {
		std::uint32_t first_last = std::min(n, block);
		partial_transform<false>(a, b, stage, first_last);
		for (std::size_t j = 0; j < large; ++j) next[j] = first_last / ds[j] + 1;
		for (std::uint32_t i = 1; i <= first_last / large_first; ++i)
			cursor[i] = std::size_t(std::upper_bound(ds.begin() + large, ds.end(), first_last / i) - ds.begin());
		for (std::uint64_t first = std::uint64_t(first_last) + 1; first <= n; first += block) {
			std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
			for (std::size_t j = 0; j < large; ++j) {
				std::uint32_t end = last / ds[j];
				stage_pass<false>(a.data(), b.data(), next[j], end, ds[j]);
				next[j] = end + 1;
			}
			for (std::uint32_t i = 1; i <= last / large_first; ++i) {
				std::uint32_t limit = last / i;
				std::size_t j = cursor[i];
				std::uint64_t x = a[i].v, y = b[i].v, xx = 0, yy = 0;
				for (; j + 1 < ds.size() && ds[j + 1] <= limit; j += 2) {
					std::size_t p = std::size_t(i) * ds[j], q = std::size_t(i) * ds[j + 1];
					x += a[p].v, y += b[p].v, xx += a[q].v, yy += b[q].v;
				}
				if (j < ds.size() && ds[j] <= limit) {
					std::size_t p = std::size_t(i) * ds[j++];
					x += a[p].v, y += b[p].v;
				}
				a[i] = T::word(std::uint32_t((x + xx) % twice));
				b[i] = T::word(std::uint32_t((y + yy) % twice));
				cursor[i] = j;
			}
		}
	} else {
		for (std::size_t j = 0; j < large; ++j) next[j] = n / ds[j];
		for (std::uint32_t i = 1; i < cursor.size(); ++i)
			cursor[i] = std::size_t(std::upper_bound(ds.begin() + large, ds.end(), n / i) - ds.begin());
		for (std::uint64_t first = std::uint64_t(n / block) * block + 1; first > block; first -= block) {
			if (first > n) continue;
			std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
			for (std::size_t j = 0; j < large; ++j) {
				std::uint32_t start = std::uint32_t((first + ds[j] - 1) / ds[j]);
				stage_pass<true>(a.data(), b.data(), start, next[j], ds[j]);
				next[j] = start - 1;
			}
			for (std::uint32_t i = 1; i <= last / large_first; ++i) {
				std::uint32_t limit = std::uint32_t((first + i - 1) / i);
				std::size_t j = cursor[i];
				std::uint64_t x = 0, xx = 0;
				for (; j >= large + 2 && ds[j - 2] >= limit; j -= 2)
					x += a[std::size_t(i) * ds[j - 1]].v, xx += a[std::size_t(i) * ds[j - 2]].v;
				if (j > large && ds[j - 1] >= limit) x += a[std::size_t(i) * ds[--j]].v;
				a[i] -= T::word(std::uint32_t((x + xx) % twice));
				cursor[i] = j;
			}
		}
		partial_transform<true>(a, b, stage, std::min(n, block));
	}
}

template <class T>
void blocked_multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	std::uint32_t n = std::uint32_t(a.size() - 1);
	if (!n) return;
	std::size_t small;
	auto stages = divisor_zeta_detail::make_plan(n, ps, small);
	std::uint32_t block = std::max(32768u, std::uint32_t(1) << ((std::bit_width(n) + 1) / 2));
	std::uint32_t split = std::max(4096u, std::uint32_t((std::uint64_t(n) + block - 1) / block));
	transform<false>(a, b, ps, small);
	for (const auto &stage : stages) stage_transform<false>(a, b, stage, block, split);
	for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
	for (auto it = stages.rbegin(); it != stages.rend(); ++it) stage_transform<true>(a, b, *it, block, split);
	transform<true>(a, b, ps, small);
}
#endif

template <class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &ps) {
	const std::size_t n = a.size() - 1;
#if defined(__x86_64__) || defined(__i386__)
	if (n >= (std::size_t(1) << 19) && __builtin_cpu_supports("avx2")) {
		blocked_multiply(a, b, ps);
		return;
	}
#endif
	std::size_t cutoff = std::sqrt(n);
	while (cutoff + 1 <= n / (cutoff + 1)) ++cutoff;
	while (cutoff && cutoff > n / cutoff) --cutoff;
	const std::size_t first = std::upper_bound(ps.begin(), ps.end(), cutoff) - ps.begin();
	transform<false>(a, b, ps, first);
	large_multiples<false>(a, b, ps, first);
	for (std::size_t i = 1; i <= n; ++i) a[i] *= b[i];
	transform<true>(a, b, ps, first);
	large_multiples<true>(a, b, ps, first);
}

}


#line 1 "components/lcm_convolution_mint.hpp"



#line 11 "components/lcm_convolution_mint.hpp"

namespace lcm_convolution_mint_detail {
using divisor_zeta_detail::Stage;
using divisor_zeta_detail::make_plan;

template<bool inverse, class T>
void transform(std::vector<T> &a, std::vector<T> &b, const Stage &stage, std::uint32_t block, std::uint32_t split) {
	const auto &divisors = stage.divisors;
	if (divisors.empty()) return;
	std::uint32_t n = std::uint32_t(a.size() - 1), end = n / divisors[0];
	std::size_t large = std::size_t(std::upper_bound(divisors.begin(), divisors.end(), split) - divisors.begin());
	std::uint64_t large_first = large < divisors.size() ? divisors[large] : std::uint64_t(n) + 1;
	std::vector<std::size_t> cursor(std::size_t(n / large_first) + 1, large);
	std::vector<std::uint64_t> next(large);
	std::vector<std::uint32_t> source(large, 1);
	std::vector<T> x, y;
	if constexpr (!inverse) {
		x.assign(a.begin(), a.begin() + std::size_t(end) + 1);
		y.assign(b.begin(), b.begin() + std::size_t(end) + 1);
	}
	for (std::size_t j = 0; j < large; ++j) next[j] = divisors[j];
	std::uint64_t first = 1;
	if constexpr (inverse) {
		std::uint32_t last = std::min(n, block);
		for (std::uint32_t i = 1; i <= last / divisors[0]; ++i) {
			std::uint32_t limit = last / i;
			std::size_t j = 0;
			T value = a[i];
			while (j < divisors.size() && divisors[j] <= limit) a[std::size_t(i) * divisors[j++]] -= value;
			if (i < cursor.size()) cursor[i] = std::max(j, large);
		}
		for (std::size_t j = 0; j < large; ++j) {
			std::uint32_t i = last / divisors[j] + 1;
			source[j] = i;
			next[j] = std::uint64_t(i) * divisors[j];
		}
		first = std::uint64_t(last) + 1;
	}
	for (; first <= n; first += block) {
		std::uint32_t last = std::uint32_t(std::min<std::uint64_t>(n, first + block - 1));
		for (std::size_t j = 0; j < large; ++j) {
			std::uint64_t at = next[j];
			std::uint32_t i = source[j], d = divisors[j];
			if constexpr (inverse) {
				for (; at <= last; at += d, ++i) a[std::size_t(at)] -= a[i];
			} else {
				for (; at <= last; at += d, ++i) {
					a[std::size_t(at)] += x[i];
					b[std::size_t(at)] += y[i];
				}
			}
			next[j] = at;
			source[j] = i;
		}
		for (std::uint32_t i = 1; i <= last / large_first; ++i) {
			std::uint32_t limit = last / i;
			std::size_t j = cursor[i];
			T value = inverse ? a[i] : x[i];
			while (j < divisors.size() && divisors[j] <= limit) {
				std::size_t to = std::size_t(i) * divisors[j++];
				if constexpr (inverse) a[to] -= value;
				else a[to] += value, b[to] += y[i];
			}
			cursor[i] = j;
		}
	}
}

template<class T>
void multiply(std::vector<T> &a, std::vector<T> &b, const std::vector<std::size_t> &primes) {
	std::uint32_t n = std::uint32_t(a.size() - 1);
	if (!n) return;
	std::size_t small;
	auto stages = make_plan(n, primes, small);
	std::uint32_t block = std::max(8192u, std::uint32_t(1) << ((std::bit_width(n) + 1) / 2));
	std::uint32_t split = std::max(4096u, std::uint32_t((std::uint64_t(n) + block - 1) / block));
	divisor_zeta_detail::for_each<false, false>(a.size(), primes, small, [&](std::size_t to, std::size_t from) {
		a[to] += a[from], b[to] += b[from];
	});
	for (const auto &stage : stages) transform<false>(a, b, stage, block, split);
	for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
	for (auto it = stages.rbegin(); it != stages.rend(); ++it) transform<true>(a, b, *it, block, split);
	divisor_zeta_detail::for_each<true, false>(a.size(), primes, small, [&](std::size_t to, std::size_t from) { a[to] -= a[from]; });
}
}


#line 13 "convolution/gcd_lcm_convolution.hpp"

namespace gcd_lcm_convolution_detail {

template <bool gcd, class T>
std::vector<T> multiply(std::vector<T> a, std::vector<T> b) {
	if (a.empty() || b.empty()) return {};
	assert(a[0] == T(0) && b[0] == T(0));
	std::size_t n = std::max(a.size(), b.size());
	a.resize(n, T(0));
	b.resize(n, T(0));
	auto ps = prime_sieve(n);
	if constexpr (gcd && std::is_same_v<T, mint_detail::Mint>) {
		if (n >= 4096 && n - 1 <= UINT32_MAX) {
			gcd_convolution_mint_detail::multiply(a, b, ps);
			return a;
		}
	}
	if constexpr (!gcd && std::is_same_v<T, mint_detail::Mint>) {
		if (n > (std::size_t(1) << 19) && n - 1 <= UINT32_MAX) {
			lcm_convolution_mint_detail::multiply(a, b, ps);
			return a;
		}
	}
	divisor_zeta_detail::for_each<false, gcd>(n, ps, ps.size(), [&](std::size_t to, std::size_t from) {
		a[to] += a[from], b[to] += b[from];
	});
	for (std::size_t i = 1; i < n; ++i) a[i] *= b[i];
	divisor_zeta<true, gcd>(a, ps);
	return a;
}
}

template <class T>
std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<true>(std::move(a), std::move(b));
}
template <class T>
std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b) {
	return gcd_lcm_convolution_detail::multiply<false>(std::move(a), std::move(b));
}


#line 1 "test/fixtures/mint_io.hpp"



#line 1 "components/fast_io_bulk.hpp"



#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 5 "components/fast_io_bulk.hpp"

#line 1 "components/fast_io_output.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 13 "components/fast_io.hpp"
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


#line 5 "components/fast_io_output.hpp"
#include <limits>
#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 10 "components/fast_io_bulk.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	template <bool Signed = false>
	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid, unsigned *signs = nullptr) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		unsigned n0 = 0, n1 = 0;
		if constexpr (Signed) {
			if (a < 1 || a > 11 || b <= a + 1 || b - a - 1 > 11) [[unlikely]] {
				invalid = _mm256_set1_epi32(-1);
				p += std::min(b + 1, 32u);
				*signs = 0;
				return _mm256_setzero_si256();
			}
			n0 = p[0] == '-', n1 = p[a + 1] == '-';
			*signs = n0 | (n1 << 1);
		}
		if (a < n0 + 1 || a - n0 > 10 || b <= a + n1 + 1 || b - a - n1 - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		auto window = bytes;
		if constexpr (Signed) window = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1 + n1)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + n0)));
		else window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - n1 - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a - n0].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	template <bool Signed = false>
	static std::uint32_t one(const char *&p, bool &bad) {
		bool negative = false;
		if constexpr (Signed) negative = *p == '-', p += negative;
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		if constexpr (Signed) bad |= value > 2147483647ULL + negative;
		else bad |= value > 0xffffffffULL;
		p += n + 1;
		return negative ? 0u - std::uint32_t(value) : std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	template <bool Signed = false>
	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				unsigned signs0 = 0, signs1 = 0, signs2 = 0, signs3 = 0;
				const auto g0 = two<Signed>(p[0], invalid, &signs0), g1 = two<Signed>(p[1], invalid, &signs1), g2 = two<Signed>(p[2], invalid, &signs2), g3 = two<Signed>(p[3], invalid, &signs3);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				auto max_high = _mm256_set1_epi32(42), max_low = _mm256_set1_epi32(94967295);
				auto signs = _mm256_setzero_si256();
				if constexpr (Signed) {
					signs = _mm256_setr_epi32(signs0 & 1, signs1 & 1, signs2 & 1, signs3 & 1, signs0 >> 1, signs1 >> 1, signs2 >> 1, signs3 >> 1);
					max_high = _mm256_set1_epi32(21), max_low = _mm256_add_epi32(_mm256_set1_epi32(47483647), signs);
				}
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, max_high),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, max_high), _mm256_cmpgt_epi32(low, max_low)));
				invalid = _mm256_or_si256(invalid, overflow);
				auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				if constexpr (Signed) {
					const auto mask = _mm256_sub_epi32(_mm256_setzero_si256(), signs);
					value = _mm256_sub_epi32(_mm256_xor_si256(value, mask), mask);
				}
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <bool Signed = false, class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep<Signed>(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one<Signed>(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <bool Signed = false, class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks<Signed>(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (!Signed && done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				std::conditional_t<Signed, std::int32_t, unsigned> value = 0;
				in.read(value);
				emit(reinterpret_cast<unsigned *>(&value), 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, class T, std::size_t... I>
void emit_record(Emit &emit, const T *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template <class Emit>
void read_signed_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks<true>(input, count, [&](const unsigned *values, std::size_t n) {
			emit(reinterpret_cast<const std::int32_t *>(values), n);
		});
		return;
	}
#endif
	std::int32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

template <std::size_t Fields, class Emit>
void read_signed_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::int32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_signed_blocks(input, count * Fields, [&](const std::int32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 1 "math/mint.hpp"


#line 5 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 8 "math/mint.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct [[gnu::warn_unused]] Mint {
	using mint = Mint;
	unsigned v = 0;
	[[nodiscard]] static constexpr unsigned mod() { return 998244353; }
	[[nodiscard, gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[nodiscard, gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[nodiscard, gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[nodiscard, gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[nodiscard, gnu::always_inline]] Mint operator+() const { return *this; }
	[[nodiscard, gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[nodiscard, gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[nodiscard, gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[nodiscard, gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	[[nodiscard]] Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[nodiscard, gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[nodiscard, gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	[[nodiscard]] Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[nodiscard, gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 10 "test/fixtures/mint_io.hpp"

inline void read_mints(FastInput &input, std::span<mint> values) {
	std::size_t offset = 0;
	read_unsigned_blocks(input, values.size(), [&](const std::uint32_t *block, std::size_t count) {
		for (std::size_t i = 0; i < count; ++i) values[offset + i] = mint::raw(block[i]);
		offset += count;
	});
}

inline void write_mints(FastOutput &output, std::span<const mint> values) {
	write_unsigned_records<1>(output, values.size(), [&](std::size_t i) {
		return std::array{values[i].val()};
	});
}


#line 6 "test/gcd_convolution.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0;
	input.read(n);
	std::vector<mint> a(n + 1), b(n + 1);
	read_mints(input, std::span(a).subspan(1));
	read_mints(input, std::span(b).subspan(1));
	auto result = gcd_convolution(a, b);
	write_mints(output, std::span(result).subspan(1));
}
