#ifndef CP_COMPONENTS_POLYNOMIAL_GCD_BLOCKED_HPP
#define CP_COMPONENTS_POLYNOMIAL_GCD_BLOCKED_HPP

#include "fps_division_blocked.hpp"
#include <limits>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

namespace gcd_detail {
using namespace detail;
using division_detail::fold;
using division_detail::subtract;
using division_detail::montgomery_reduce;
using division_detail::kLongDivision;
class Stack {
public:
	explicit Stack(std::span<std::uint32_t> memory) : base_(memory.data()), size_(memory.size()) {}

	std::size_t mark() const { return top_; }
	void release(std::size_t mark) { top_ = mark; }

	std::uint32_t* take(std::size_t n) {
		const std::size_t need = Arena::footprint(n);
		if (need > size_ - top_) throw std::bad_alloc();
		std::uint32_t* const p = base_ + top_;
		top_ += need;
		return p;
	}

private:
	std::uint32_t* base_;
	std::size_t size_, top_ = 0;
};

inline std::uint32_t mod_mul(std::uint32_t x, std::uint32_t y) { return std::uint32_t(std::uint64_t(x) * y % kP); }
inline std::uint32_t mod_sub(std::uint32_t x, std::uint32_t y) { return x >= y ? x - y : x + kP - y; }
inline std::uint32_t mod_negate(std::uint32_t x) { return x ? kP - x : 0; }
inline std::uint32_t mod_inverse(std::uint32_t x) { return blocked_ntt::detail::power(x, kP - 2); }

inline void combine3(const std::uint32_t* u, const std::uint32_t* v, std::uint32_t* out, std::ptrdiff_t from,
					 std::ptrdiff_t to, std::uint32_t x, std::uint32_t y, std::uint32_t z) {
	const Vec xs = broadcast(x), ys = broadcast(y), zs = broadcast(z);
	for (std::ptrdiff_t j = from & ~std::ptrdiff_t(7); j < to; j += 8) {
		const Vec a = load_unaligned(u + j), b = load_unaligned(v + j - 1), c = load_unaligned(v + j);
		const Vec even = _mm256_add_epi64(_mm256_add_epi64(_mm256_mul_epu32(a, xs), _mm256_mul_epu32(b, ys)),
										  _mm256_mul_epu32(c, zs));
		const Vec odd = _mm256_add_epi64(
			_mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(a, 32), xs), _mm256_mul_epu32(_mm256_srli_epi64(b, 32), ys)),
			_mm256_mul_epu32(_mm256_srli_epi64(c, 32), zs));
		store_unaligned(out + j, reduce(montgomery_reduce(even, odd), kP));
	}
}

[[gnu::always_inline]] inline void fill_pair(const std::uint32_t* roots, const std::uint32_t* x0, const std::uint32_t* x1,
											 std::size_t g, Window (&window)[2][4]) {
	const std::size_t at = 32 * g;
	const Vec a0[4] = {load(x0 + at), load(x0 + at + 8), load(x0 + at + 16), load(x0 + at + 24)};
	const Vec a1[4] = {load(x1 + at), load(x1 + at + 8), load(x1 + at + 16), load(x1 + at + 24)};
	fill_windows(window[0], a0, roots, g);
	fill_windows(window[1], a1, roots, g);
}

template <std::size_t K>
inline void products(const std::uint32_t* roots, const std::uint32_t* x0, const std::uint32_t* x1,
					 const std::uint32_t* const (&y_in)[K][2], std::uint32_t* const (&out_in)[K], std::size_t n) {
	const std::uint32_t* y[K][2];
	std::uint32_t* out[K];
	for (std::size_t k = 0; k < K; ++k) y[k][0] = y_in[k][0], y[k][1] = y_in[k][1], out[k] = out_in[k];
	const Factor undo_montgomery(kR);
	Window window[2][2][4];
	fill_pair(roots, x0, x1, 0, window[0]);
	for (std::size_t g = 0; g < n / 32; ++g) {
		if (g + 1 < n / 32) fill_pair(roots, x0, x1, g + 1, window[(g + 1) & 1]);
		const Window(&w)[2][4] = window[g & 1];
		const std::size_t at = 32 * g;
#pragma GCC unroll 4
		for (std::size_t t = 0; t < 4; ++t)
			for (std::size_t k = 0; k < K; ++k) {
				const Vec s = low(add(leaf_product(w[0][t], y[k][0] + at + 8 * t), leaf_product(w[1][t], y[k][1] + at + 8 * t)));
				store(out[k] + at + 8 * t, reduce(times(s, undo_montgomery), kP));
			}
	}
}

struct Matrix {
	std::uint32_t* entry[4];
	std::size_t size[4];
	std::uint32_t at0[4];
	std::uint32_t* transform[4];
	std::size_t length[4];
	std::size_t room, progress;
};

class HalfGcd {
public:

	static constexpr std::ptrdiff_t kDirect = 64;

	static int log_length(std::size_t n_max) {
		return std::max(Transform::kMinLog, int(std::bit_width(std::max<std::size_t>(n_max, 2) - 1)));
	}

	static std::size_t scratch_words(std::size_t n_max) { return 48 * (std::size_t(1) << log_length(n_max)) + 4096; }

	static std::size_t words(std::size_t n_max) {
		return Transform::words(log_length(n_max)) + Arena::footprint(scratch_words(n_max));
	}

	HalfGcd(Arena& arena, std::size_t n_max, std::ptrdiff_t direct = kDirect)
		: transform_(arena, log_length(n_max)), stack_(arena.take(scratch_words(n_max))), direct_(direct) {}

	Matrix matrix(std::size_t k, std::size_t room) {
		Matrix r;
		for (int i = 0; i < 4; ++i) {
			r.entry[i] = take(k + 1).data();
			r.transform[i] = room ? take(room).data() : nullptr;
			r.length[i] = 0;
		}
		r.room = room;
		return r;
	}

	void jump(const std::uint32_t* a, std::ptrdiff_t n, const std::uint32_t* b, std::ptrdiff_t m, std::ptrdiff_t k,
			  Matrix& out, unsigned need = 0xF) {
		const std::ptrdiff_t target = n - k;
		if (m < target) return identity(out);
		if (n > 2 * k) {
			const std::ptrdiff_t s = n - 2 * k;
			return jump(a + s, n - s, b + s, m - s, k, out, need);
		}
		if (k <= direct_) return euclid(a, n, b, m, k, out);
		const std::size_t mark = stack_.mark(), room = std::max<std::size_t>(64, std::bit_ceil(std::size_t(k)));
		const std::ptrdiff_t k1 = first_jump(k), s1 = std::max<std::ptrdiff_t>(0, n - 2 * k1);
		Matrix r1 = matrix(std::size_t(k1), room);
		jump(a + s1, n - s1, b + s1, m - s1, k1, r1);
		std::ptrdiff_t c_deg = 0;
		const std::uint32_t *c = nullptr, *d = nullptr;
		if (r1.progress == 0) {
			r1 = matrix(std::size_t(n - m), room);
			std::uint32_t* const rest = take(std::size_t(n + 1)).data();
			divide_step(a, n, b, m, r1, rest);
			c = b, d = rest, c_deg = m;
		} else {
			c_deg = n - std::ptrdiff_t(r1.progress);
			const std::ptrdiff_t s2 = std::max<std::ptrdiff_t>(0, 2 * target - c_deg);
			std::uint32_t *const cs = take(std::size_t(c_deg + 1)).data(), *const ds = take(std::size_t(c_deg + 1)).data();
			apply(a, n, b, m, r1, c_deg, s2, cs, ds);
			c = cs, d = ds;
		}
		std::ptrdiff_t d_deg = c_deg - 1;
		while (d_deg >= target && d[d_deg] == 0) --d_deg;
		if (d_deg < target) {
			copy(r1, out);
			return stack_.release(mark);
		}
		const std::ptrdiff_t k2 = c_deg - target, s2 = std::max<std::ptrdiff_t>(0, c_deg - 2 * k2);
		Matrix r2 = matrix(std::size_t(k2), room);
		jump(c + s2, c_deg - s2, d + s2, d_deg - s2, k2, r2, (need & 3 ? 3u : 0u) | (need & 12 ? 12u : 0u));
		combine(r2, r1, out, need);
		stack_.release(mark);
	}

private:

	static std::ptrdiff_t first_jump(std::ptrdiff_t k) {
		const std::size_t u = std::size_t(k);
		return std::ptrdiff_t(std::has_single_bit(u) ? u / 2 : std::bit_floor(u));
	}

	std::span<std::uint32_t> take(std::size_t n) { return {stack_.take(n), n}; }

	std::uint32_t* take_padded(std::size_t n) {
		std::uint32_t* const p = stack_.take(n + 16);
		std::fill_n(p, n + 16, 0);
		return p + 8;
	}

	static void identity(Matrix& out) {
		out.entry[0][0] = out.entry[3][0] = 1;
		out.size[0] = out.size[3] = 1;
		out.size[1] = out.size[2] = 0;
		out.at0[0] = out.at0[3] = 1, out.at0[1] = out.at0[2] = 0;
		for (auto& l : out.length) l = 0;
		out.progress = 0;
	}

	static void copy(const Matrix& from, Matrix& to) {
		for (int i = 0; i < 4; ++i) {
			std::copy_n(from.entry[i], from.size[i], to.entry[i]);
			to.size[i] = from.size[i];
			to.at0[i] = from.at0[i];
			to.length[i] = 0;
		}
		to.progress = from.progress;
	}

	void euclid(const std::uint32_t* a, std::ptrdiff_t n, const std::uint32_t* b, std::ptrdiff_t m, std::ptrdiff_t k,
				Matrix& out) {
		const std::size_t mark = stack_.mark();
		const std::ptrdiff_t target = n - k;
		std::ptrdiff_t floor = n - 2 * k, lo = std::max<std::ptrdiff_t>(0, floor);
		std::uint32_t* p[3];
		for (auto& x : p) x = take_padded(std::size_t(n + 1));
		std::copy(a + lo, a + n + 1, p[0] + lo);
		std::copy(b + lo, b + m + 1, p[1] + lo);

		const std::size_t cap = std::size_t(k) + 8;
		std::uint32_t* row[3][2];
		std::size_t row_size[3][2] = {{1, 0}, {0, 1}, {0, 0}};
		for (auto& r : row)
			for (auto& e : r) {
				e = take_padded(cap);
				std::fill_n(e, cap, 0);
			}
		row[0][0][0] = 1, row[1][1][0] = 1;
		std::uint32_t *c = p[0], *d = p[1], *e = p[2];
		std::ptrdiff_t c_deg = n, d_deg = m;
		while (d_deg >= target) {
			const std::ptrdiff_t delta = c_deg - d_deg;
			const std::uint32_t alpha = d[d_deg];
			floor += delta;
			lo = std::max<std::ptrdiff_t>(0, floor);
			if (delta == 1) {

				const std::uint32_t beta = c[c_deg];
				const std::uint32_t gamma = mod_sub(mod_mul(alpha, c[c_deg - 1]), mod_mul(beta, d[d_deg - 1]));

				const std::uint32_t alpha_r = mod_mul(alpha, kR);
				const std::uint32_t x = mod_mul(alpha, alpha_r), y = kP - mod_mul(beta, alpha_r), z = mod_negate(mod_mul(gamma, kR));
				combine3(c, d, e, lo, d_deg, x, y, z);
				for (int i = 0; i < 2; ++i) {
					const std::size_t sc = row_size[0][i], sd = row_size[1][i];
					const std::size_t se = std::max(sc, sd ? sd + 1 : 0);
					combine3(row[0][i], row[1][i], row[2][i], 0, std::ptrdiff_t(se), x, y, z);
					if (row_size[2][i] > se) std::fill(row[2][i] + se, row[2][i] + row_size[2][i], 0);
					row_size[2][i] = se;
				}
			} else {
				pseudo_divide(c, c_deg, d, d_deg, e, lo, alpha, row, row_size);
			}
			std::uint32_t* const old = c;
			c = d, d = e, e = old;
			for (int i = 0; i < 2; ++i) {
				std::swap(row[0][i], row[1][i]);
				std::swap(row[1][i], row[2][i]);
				std::swap(row_size[0][i], row_size[1][i]);
				std::swap(row_size[1][i], row_size[2][i]);
			}
			c_deg = d_deg;
			d_deg = c_deg - 1;
			while (d_deg >= target && d[d_deg] == 0) --d_deg;
		}
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j) {
				std::copy_n(row[i][j], row_size[i][j], out.entry[2 * i + j]);
				out.size[2 * i + j] = row_size[i][j];
				out.at0[2 * i + j] = row_size[i][j] ? row[i][j][0] : 0;
				out.length[2 * i + j] = 0;
			}
		out.progress = std::size_t(n - c_deg);
		stack_.release(mark);
	}

	static void pseudo_divide(const std::uint32_t* c, std::ptrdiff_t c_deg, const std::uint32_t* d, std::ptrdiff_t d_deg,
							  std::uint32_t* e, std::ptrdiff_t lo, std::uint32_t alpha, std::uint32_t* (&row)[3][2],
							  std::size_t (&row_size)[3][2]) {
		std::copy(c + lo, c + c_deg + 1, e + lo);
		for (int i = 0; i < 2; ++i) {
			std::copy_n(row[0][i], row_size[0][i], row[2][i]);
			if (row_size[2][i] > row_size[0][i]) std::fill(row[2][i] + row_size[0][i], row[2][i] + row_size[2][i], 0);
			row_size[2][i] = row_size[0][i];
		}
		for (std::ptrdiff_t t = c_deg - d_deg; t >= 0; --t) {
			const std::uint32_t minus_tau = mod_negate(e[d_deg + t]);
			for (std::ptrdiff_t j = lo; j <= d_deg + t; ++j) {
				const std::uint32_t dj = j - t >= 0 ? d[j - t] : 0;
				e[j] = std::uint32_t((std::uint64_t(alpha) * e[j] + std::uint64_t(minus_tau) * dj) % kP);
			}
			for (int i = 0; i < 2; ++i) {
				std::uint32_t* const re = row[2][i];
				const std::uint32_t* const rd = row[1][i];
				const std::size_t sd = row_size[1][i], shift = std::size_t(t);
				const std::size_t se = std::max(row_size[2][i], sd ? sd + shift : 0);
				for (std::size_t j = 0; j < se; ++j) {
					const std::uint32_t dj = j >= shift ? rd[j - shift] : 0;
					re[j] = std::uint32_t((std::uint64_t(alpha) * re[j] + std::uint64_t(minus_tau) * dj) % kP);
				}
				row_size[2][i] = se;
			}
		}
	}

	void divide_step(const std::uint32_t* a, std::ptrdiff_t n, const std::uint32_t* b, std::ptrdiff_t m, Matrix& r,
					 std::uint32_t* rest) {
		const std::size_t delta = std::size_t(n - m);
		std::uint32_t* q = r.entry[3];
		if ((delta + 1) * std::size_t(m + 1) <= kLongDivision) {
			std::copy(a, a + n + 1, rest);
			const std::uint32_t lead = mod_inverse(b[m]);
			for (std::size_t i = delta + 1; i-- > 0;) {
				const std::uint32_t qi = mod_mul(rest[std::size_t(m) + i], lead);
				q[i] = qi;
				for (std::ptrdiff_t t = 0; t <= m; ++t)
					rest[i + std::size_t(t)] = mod_sub(rest[i + std::size_t(t)], mod_mul(qi, b[t]));
			}
		} else {
			const std::span<const std::uint32_t> f(a, std::size_t(n + 1)), g(b, std::size_t(m + 1));
			Arena arena(divide_words(f.size(), g.size()));
			divide(arena, f, g, std::span<std::uint32_t>(q, delta + 1), std::span<std::uint32_t>(rest, std::size_t(m)));
			std::fill(rest + m, rest + n + 1, 0);
		}
		for (std::size_t i = 0; i <= delta; ++i) q[i] = mod_negate(q[i]);
		r.entry[1][0] = r.entry[2][0] = 1;
		r.size[0] = 0, r.size[1] = r.size[2] = 1, r.size[3] = delta + 1;
		r.at0[0] = 0, r.at0[1] = r.at0[2] = 1, r.at0[3] = q[0];
		for (auto& l : r.length) l = 0;
		r.progress = delta;
	}

	void transform_of(const std::uint32_t* c, std::size_t size, std::span<std::uint32_t> out) const {
		const std::size_t len = out.size();
		if (size <= len) return transform_.forward(std::span<const std::uint32_t>(c, size), 0, out);
		fold(std::span<const std::uint32_t>(c, size), out);
		transform_.forward(out);
	}

	void extend(Matrix& r, int i, std::size_t len) const {
		if (r.length[i] == len) return;
		const std::span<std::uint32_t> t(r.transform[i], len);
		const std::size_t half = len / 2, size = r.size[i];
		if (r.length[i] == half && size <= half + 1) {
			const std::span<std::uint32_t> upper = t.subspan(half);
			if (size <= half) {
				transform_.forward_upper(std::span<const std::uint32_t>(r.entry[i], size), 0, upper);
			} else {
				std::copy_n(r.entry[i], half, upper.data());
				upper[0] = mod_sub(upper[0], r.entry[i][half]);
				transform_.forward_upper(upper, 0, upper);
			}
		} else {
			transform_of(r.entry[i], size, t);
		}
		r.length[i] = len;
	}

	void apply(const std::uint32_t* a, std::ptrdiff_t n, const std::uint32_t* b, std::ptrdiff_t m, Matrix& r1,
			   std::ptrdiff_t c_deg, std::ptrdiff_t s2, std::uint32_t* c, std::uint32_t* d) {
		const std::size_t mark = stack_.mark();
		const std::size_t len = std::max<std::size_t>(64, std::bit_ceil(std::size_t(n + 1) / 2));
		for (int i = 0; i < 4; ++i) extend(r1, i, len);
		const std::span<std::uint32_t> ta = take(len), tb = take(len);
		transform_of(a, std::size_t(n + 1), ta);
		transform_of(b, std::size_t(m + 1), tb);
		const std::span<std::uint32_t> pc = take(len), pd = take(len);
		apply_products(r1, ta, tb, pc, pd);
		const std::ptrdiff_t l = std::ptrdiff_t(len), mixed = c_deg - l;
		if (mixed < 0) {
			std::copy(pc.begin() + s2, pc.begin() + c_deg + 1, c + s2);
			std::copy(pd.begin() + s2, pd.begin() + c_deg + 1, d + s2);
			return stack_.release(mark);
		}
		const std::size_t low = std::size_t(mixed + 1);
		const std::span<std::uint32_t> lc = take(len), ld = take(len);
		const std::size_t short_len = std::max<std::size_t>(64, std::bit_ceil(2 * low - 1));
		if (short_len < len) {

			Matrix r = r1;
			for (int i = 0; i < 4; ++i) {
				r.transform[i] = take(short_len).data();
				transform_.forward(std::span<const std::uint32_t>(r1.entry[i], std::min(low, r1.size[i])), 0,
								   std::span<std::uint32_t>(r.transform[i], short_len));
			}
			const std::span<std::uint32_t> sa = ta.first(short_len), sb = tb.first(short_len);
			transform_.forward(std::span<const std::uint32_t>(a, low), 0, sa);
			transform_.forward(std::span<const std::uint32_t>(b, std::min(low, std::size_t(m + 1))), 0, sb);
			apply_products(r, sa, sb, lc.first(short_len), ld.first(short_len));
		} else {
			transform_.forward(std::span<const std::uint32_t>(a, low), 0, ta);
			transform_.forward(std::span<const std::uint32_t>(b, std::min(low, std::size_t(m + 1))), 0, tb);
			apply_products(r1, ta, tb, lc, ld);

			lc[0] = std::uint32_t((std::uint64_t(r1.at0[0]) * a[0] + std::uint64_t(r1.at0[1]) * b[0]) % kP);
			ld[0] = std::uint32_t((std::uint64_t(r1.at0[2]) * a[0] + std::uint64_t(r1.at0[3]) * b[0]) % kP);
		}

		const std::ptrdiff_t first = std::min(s2, mixed + 1);
		std::copy(lc.begin() + first, lc.begin() + mixed + 1, c + first);
		std::copy(ld.begin() + first, ld.begin() + mixed + 1, d + first);
		const std::ptrdiff_t from = std::max(s2, mixed + 1);
		std::copy(pc.begin() + from, pc.begin() + l, c + from);
		std::copy(pd.begin() + from, pd.begin() + l, d + from);
		subtract(pc.data(), lc.data(), c + l, std::size_t(mixed + 1));
		subtract(pd.data(), ld.data(), d + l, std::size_t(mixed + 1));
		stack_.release(mark);
	}

	void apply_products(const Matrix& r1, std::span<const std::uint32_t> ta, std::span<const std::uint32_t> tb,
						std::span<std::uint32_t> c, std::span<std::uint32_t> d) const {
		const std::size_t len = c.size();
		const auto t = [&](int i) { return std::span<const std::uint32_t>(r1.transform[i], len); };
		const Transform::Pair pc[2] = {{t(0), ta}, {t(1), tb}}, pd[2] = {{t(2), ta}, {t(3), tb}};
		transform_.inverse_product_sum(pc, c);
		transform_.inverse_product_sum(pd, d);
	}

	void combine(Matrix& r2, Matrix& r1, Matrix& out, unsigned need) {
		const std::size_t mark = stack_.mark();
		const std::size_t p = r2.progress + r1.progress, len = std::max<std::size_t>(64, std::bit_ceil(p));
		for (int i = 0; i < 2; ++i)
			if (need & (3u << 2 * i)) extend(r2, 2 * i, len), extend(r2, 2 * i + 1, len);
		for (int j = 0; j < 2; ++j)
			if (need & (5u << j)) extend(r1, j, len), extend(r1, 2 + j, len);
		const std::span<std::uint32_t> buffer = take(len);
		const bool keep = out.room >= len;
		std::uint32_t* t[4];
		for (int i = 0; i < 4; ++i) t[i] = keep ? out.transform[i] : take(len).data();
		for (int i = 0; i < 2; ++i) {

			const std::uint32_t *x0 = r2.transform[2 * i], *x1 = r2.transform[2 * i + 1];
			const std::uint32_t* const y[2][2] = {{r1.transform[0], r1.transform[2]}, {r1.transform[1], r1.transform[3]}};
			const unsigned row = need >> 2 * i & 3;
			if (row == 3) {
				std::uint32_t* const to[2] = {t[2 * i], t[2 * i + 1]};
				products<2>(transform_.roots(), x0, x1, y, to, len);
			} else if (row) {
				const int j = row == 1 ? 0 : 1;
				const std::uint32_t* const yj[1][2] = {{y[j][0], y[j][1]}};
				std::uint32_t* const to[1] = {t[2 * i + j]};
				products<1>(transform_.roots(), x0, x1, yj, to, len);
			}
		}
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j) {
				const int ij = 2 * i + j;
				out.length[ij] = 0;
				const std::uint32_t x0 = std::uint32_t(
					(std::uint64_t(r2.at0[2 * i]) * r1.at0[j] + std::uint64_t(r2.at0[2 * i + 1]) * r1.at0[2 + j]) % kP);
				out.at0[ij] = x0;
				if (!(need >> ij & 1)) continue;
				if (keep) out.length[ij] = len;
				transform_.inverse(std::span<const std::uint32_t>(t[ij], len), buffer);
				std::uint32_t* const e = out.entry[ij];
				std::copy_n(buffer.data(), std::min(p + 1, len), e);
				if (p == len) e[len] = mod_sub(e[0], x0), e[0] = x0;
				std::size_t size = p + 1;
				while (size > 0 && e[size - 1] == 0) --size;
				out.size[ij] = size;
			}
		out.progress = p;
		stack_.release(mark);
	}

	Transform transform_;
	Stack stack_;
	std::ptrdiff_t direct_;
};

}

inline std::size_t inverse_mod_words(std::size_t n, std::size_t m) {
	const std::size_t d = m - 1;
	return divide_words(n, m) + Arena::footprint(d) + Arena::footprint(quotient_size(n, m)) +
		   gcd_detail::HalfGcd::words(std::max<std::size_t>(d, 1));
}

inline std::ptrdiff_t inverse_mod(Arena& arena, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
								  std::span<std::uint32_t> h) {
	using namespace gcd_detail;
	const std::ptrdiff_t d = std::ptrdiff_t(g.size()) - 1;
	if (d == 0) return 0;
	const std::span<std::uint32_t> b = arena.take(std::size_t(d));
	const std::size_t rs = remainder_size(f.size(), g.size());
	divide(arena, f, g, arena.take(quotient_size(f.size(), g.size())), b.first(rs));
	std::ptrdiff_t m = std::ptrdiff_t(rs) - 1;
	while (m >= 0 && b[std::size_t(m)] == 0) --m;
	if (m < 0) return -1;
	HalfGcd gcd(arena, std::size_t(d));
	Matrix r = gcd.matrix(std::size_t(d), 0);
	gcd.jump(g.data(), d, b.data(), m, d, r, 0x2);
	if (std::ptrdiff_t(r.progress) != d) return -1;

	const std::uint32_t c = std::uint32_t((std::uint64_t(r.at0[0]) * g[0] + std::uint64_t(r.at0[1]) * b[0]) % kP);
	const std::uint32_t scale = mod_inverse(c);
	std::size_t t = r.size[1];
	while (t > 0 && r.entry[1][t - 1] == 0) --t;
	for (std::size_t i = 0; i < t; ++i) h[i] = mod_mul(r.entry[1][i], scale);
	return std::ptrdiff_t(t);
}

}


namespace fps_detail::blocked_poly {
inline bool native_inv_mod(const Mint* a, std::size_t n, const Mint* b, std::size_t m, std::vector<Mint>& out) {
	Arena arena(inverse_mod_words(n, m) + Arena::footprint(n) + Arena::footprint(m) + Arena::footprint(m));
	auto f = arena.take(n), g = arena.take(m), h = arena.take(m);
	for (std::size_t i = 0; i < n; ++i) f[i] = std::min(a[i].v, a[i].v - Mint::mod());
	for (std::size_t i = 0; i < m; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	const auto size = inverse_mod(arena, f, g, h);
	if (size < 0) return false;
	prepare_fps_output(out, std::size_t(size));
	division_detail::append_words(out, h.data(), std::size_t(size), 932051910);
	return true;
}

template<class Sink>
inline void native_gcd_matrix(const Mint* a, std::size_t n, const Mint* b, std::size_t m, unsigned need, Sink sink) {
	Arena arena(gcd_detail::HalfGcd::words(n - 1) + Arena::footprint(n) + Arena::footprint(m));
	auto f = arena.take(n), g = arena.take(m);
	for (std::size_t i = 0; i < n; ++i) f[i] = std::min(a[i].v, a[i].v - Mint::mod());
	for (std::size_t i = 0; i < m; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	gcd_detail::HalfGcd gcd(arena, n - 1);
	auto matrix = gcd.matrix(n - 1, 0);
	gcd.jump(f.data(), std::ptrdiff_t(n) - 1, g.data(), std::ptrdiff_t(m) - 1, std::ptrdiff_t(n) - 1, matrix, need);
	sink(matrix);
}
}

#pragma GCC pop_options
#endif
#endif
