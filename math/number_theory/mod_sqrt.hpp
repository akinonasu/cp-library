#ifndef MOD_SQRT_HPP
#define MOD_SQRT_HPP

#include "../../components/montgomery.hpp"
#include "../../components/montgomery32.hpp"
#include "../../components/splitmix64.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>

namespace mod_sqrt_detail {
template<class M> struct ChunkLog {
	using U = typename M::U;
	U modulus = 0;
	static constexpr unsigned bits = 10, size = 1U << bits, slots = 2 * size;
	unsigned uses = 0;
	bool ready = false;
	std::array<U, slots> keys{};
	std::array<std::uint16_t, slots> values{};
	std::array<std::array<U, size>, (std::numeric_limits<U>::digits + bits - 1) / bits> correction;
	static unsigned hash(U value) { return std::uint64_t(value) * UINT64_C(11995408973635179863) >> (63 - bits); }
	void build(const M &m, const std::array<U, std::numeric_limits<U>::digits> &powers, int s) {
		keys.fill(0);
		U value = m.one;
		for (unsigned i = 0; i < size; ++i) {
			unsigned h = hash(value);
			while (keys[h]) h = (h + 1) & (slots - 1);
			keys[h] = value;
			values[h] = i;
			value = m.mul(value, powers[s - bits]);
		}
		std::array<U, std::numeric_limits<U>::digits> inverse;
		inverse[s - 1] = powers[s - 1];
		for (int i = s - 1; i--;) inverse[i] = m.mul(powers[i], inverse[i + 1]);
		for (int offset = 0; offset < s; offset += bits) {
			value = m.one;
			for (unsigned i = 0; i < size; ++i) {
				correction[offset / bits][i] = value;
				value = m.mul(value, inverse[offset]);
			}
		}
		ready = true;
	}
	std::uint64_t logarithm(const M &m, U value, int s) const {
		std::uint64_t result = 0;
		for (int offset = 0; offset < s && value != m.one; offset += bits) {
			const int width = std::min<int>(bits, s - offset);
			U high = value;
			for (int j = offset + width; j < s; ++j) high = m.mul(high, high);
			unsigned h = hash(high);
			while (keys[h] != high) h = (h + 1) & (slots - 1);
			unsigned digit = values[h] >> (bits - width);
			if (!offset && (digit & 1)) return UINT64_MAX;
			result |= std::uint64_t(digit) << offset;
			value = m.mul(value, correction[offset / bits][digit]);
		}
		return result;
	}
	U correct(const M &m, U root, std::uint64_t exponent) const {
		for (unsigned offset = 0; exponent; exponent >>= bits, ++offset)
			root = m.mul(root, correction[offset][exponent & (size - 1)]);
		return root;
	}
};

template <class M>
std::optional<std::uint64_t> sqrt(std::uint64_t a, const M &m) {
	using U = typename M::U;
	U p = m.mod, value = m.in(U(a)), root;
	if ((p & 3) == 3) {
		root = m.pow(value, (p >> 2) + 1);
		if (m.mul(root, root) != value) return std::nullopt;
	} else {
		int s = std::countr_zero(U(p - 1));
		U q = (p - 1) >> s;
		U t = m.pow(value, q >> 1);
		root = m.mul(value, t);
		U b = m.mul(root, t), v = b;
		static thread_local ChunkLog<M> cache;
		if (s >= 12) {
			if (cache.modulus != p) { cache.modulus = p; cache.uses = 0; cache.ready = false; }
			if (cache.ready) {
				const auto exponent = cache.logarithm(m, b, s);
				if (exponent == UINT64_MAX) return std::nullopt;
				root = cache.correct(m, root, exponent >> 1);
				const std::uint64_t result = m.reduce(root);
				return std::min(result, m.mod - result);
			}
		}
		int k = 0;
		while (v != m.one && k < s) v = m.mul(v, v), ++k;
		if (k == s) return std::nullopt;
		if (k) {
			static thread_local std::array<U, std::numeric_limits<U>::digits> powers;
			static thread_local U cached_modulus = 0;
			if (cached_modulus != p) {
				SplitMix64 random(a ^ p ^ 0x9e3779b97f4a7c15ULL);
				U nonresidue = 0;
				if ((p & 7) == 5) nonresidue = 2;
				else if (p % 3 == 2) nonresidue = 3;
				else if (p % 5 == 2 || p % 5 == 3) nonresidue = 5;
				else if (p % 7 == 3 || p % 7 == 5 || p % 7 == 6) nonresidue = 7;
				for (;;) {
					U z = nonresidue ? nonresidue : U(random() % (p - 2) + 2);
					powers[0] = m.pow(m.in(z), q);
					for (int i = 1; i < s; ++i) powers[i] = m.mul(powers[i - 1], powers[i - 1]);
					if (powers[s - 1] == p - m.one) break;
				}
				cached_modulus = p;
			}
			if (s >= 12 && ++cache.uses == (1U << (ChunkLog<M>::bits - 4))) cache.build(m, powers, s);
			if (k >= 24) {
				std::array<U, std::numeric_limits<U>::digits> inverse;
				inverse[s - 1] = powers[s - 1];
				for (int i = s - 1; i--;) inverse[i] = m.mul(powers[i], inverse[i + 1]);
				auto logarithm = [&](auto &&self, U value, int offset, int bits) -> std::uint64_t {
					if (value == m.one) return 0;
					if (bits <= 4) {
						std::uint64_t result = 0;
						for (int i = 0; i < bits; ++i) {
							U high = value;
							for (int j = i + 1; j < bits; ++j) high = m.mul(high, high);
							if (high != m.one) result |= std::uint64_t(1) << i, value = m.mul(value, inverse[offset + i]);
						}
						return result;
					}
					int half = bits / 2;
					U high = value;
					for (int i = half; i < bits; ++i) high = m.mul(high, high);
					std::uint64_t low = self(self, high, offset + bits - half, half);
					for (auto mask = low; mask; mask &= mask - 1) value = m.mul(value, inverse[offset + std::countr_zero(mask)]);
					return low | (self(self, value, offset + half, bits - half) << half);
				};
				auto exponent = logarithm(logarithm, b, s - k, k);
				for (; exponent; exponent &= exponent - 1) root = m.mul(root, inverse[s - k - 1 + std::countr_zero(exponent)]);
			} else {
				while (k) {
					root = m.mul(root, powers[s - k - 1]);
					b = m.mul(b, powers[s - k]);
					int limit = k;
					v = b; k = 0;
					while (v != m.one && k < limit) v = m.mul(v, v), ++k;
					assert(k < limit);
				}
			}
		}
	}
	std::uint64_t result = m.reduce(root);
	return std::min(result, m.mod - result);
}
}

inline std::optional<std::uint64_t> mod_sqrt(std::uint64_t a, std::uint64_t prime) {
	assert(prime >= 2 && (prime == 2 || (prime & 1)));
	if (prime == 2) return a & 1;
	a %= prime;
	if (a <= 1) return a;
	if (prime <= std::numeric_limits<std::uint32_t>::max())
		return mod_sqrt_detail::sqrt(a, Montgomery32(std::uint32_t(prime)));
	return mod_sqrt_detail::sqrt(a, Montgomery64(prime));
}

#endif
