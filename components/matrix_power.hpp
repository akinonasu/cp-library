#ifndef MATRIX_POWER_HPP
#define MATRIX_POWER_HPP

#include "matrix_arithmetic.hpp"
#include "matrix_elimination_modular.hpp"
#include <bit>
#include <optional>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace matrix_detail {
constexpr bool prime_modulus(std::uint32_t p) {
	if (p < 2 || p % 2 == 0) return p == 2;
	if (p % 3 == 0) return p == 3;
	for (std::uint32_t d = 5; std::uint64_t(d) * d <= p; d += 6)
		if (p % d == 0 || p % (d + 2) == 0) return false;
	return true;
}

template <class T>
struct FrobeniusPower {
	using Ops = ModularField<T>;
	using V = std::uint32_t;
	using Row = std::vector<V>;
	int n, work = 0;
	std::vector<Row> basis, reduced;
	std::vector<int> pivots;
	Row transpose, upper, inverses;
	std::vector<std::int32_t> packed;
	int block = 0;
	std::uint64_t random = 0x243f6a8885a308d3ULL;
	explicit FrobeniusPower(const std::vector<T> &a, int size) : n(size), transpose(a.size()), upper(a.size()), inverses(size) {
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) transpose[std::size_t(j) * n + i] = a[std::size_t(i) * n + j].val();
	}
	static std::int32_t center(V x) {
		return x <= (Ops::mod - 1) / 2 ? std::int32_t(x) : std::int32_t(std::int64_t(x) - Ops::mod);
	}
	Row seed() {
		Row x(n);
		for (auto &v : x) {
			random ^= random << 7;
			random ^= random >> 9;
			v = random % Ops::mod;
		}
		return x;
	}
	Row apply(const Row &x) const {
		Row result(n);
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (block) {
			std::vector<std::int32_t> centered(n);
			for (int i = 0; i < n; ++i) centered[i] = center(x[i]);
			if (block == 16) product_packed_avx512<Ops::mod, 1>(centered.data(), packed.data(), result.data(), 1, n, n);
			else product_packed_avx2<Ops::mod, 1>(centered.data(), packed.data(), result.data(), 1, n, n);
			return result;
		}
#endif
		constexpr int count = ModularElimination<Ops>::batch;
		std::array<const V *, count> source;
		for (int i = 0; i < n; i += count) {
			int used = std::min(count, n - i);
			for (int j = 0; j < used; ++j) source[j] = transpose.data() + std::size_t(i + j) * n;
			ModularElimination<Ops>::update(result.data(), source.data(), x.data() + i, used, 0, n);
		}
		return result;
	}
	std::optional<Row> chain(Row x, int first = 0) {
		while (++work <= 4 * n) {
			const int m = int(basis.size());
			Row y = x, factors(m);
			constexpr int count = ModularElimination<Ops>::batch;
			std::array<const V *, count> source;
			std::array<V, count> coeff;
			for (int j = 0; j < m; j += count) {
				int used = std::min(count, m - j);
				for (int u = 0; u < used; ++u) {
					int p = pivots[j + u];
					std::uint64_t value = y[p];
					for (int k = 0; k < u; ++k) value += std::uint64_t(source[k][p]) * coeff[k];
					V factor = factors[j + u] = value % Ops::mod;
					coeff[u] = Ops::neg(factor);
					source[u] = reduced[j + u].data();
				}
				ModularElimination<Ops>::update(y.data(), source.data(), coeff.data(), used, 0, n);
			}
			int p = 0;
			while (p < n && !y[p]) ++p;
			if (p == n) {
				for (int i = m - 1; i >= first; --i) {
					V sum = Ops::dot(upper.data() + std::size_t(i) * n + i + 1, factors.data() + i + 1, m - i - 1);
					factors[i] = Ops::mul(Ops::add(factors[i], Ops::neg(sum)), inverses[i]);
				}
				for (auto &value : factors) value = Ops::neg(value);
				factors.push_back(1);
				return factors;
			}
			for (int j = 0; j < m; ++j) upper[std::size_t(j) * n + m] = factors[j];
			upper[std::size_t(m) * n + m] = y[p];
			inverses[m] = Ops::inv(y[p]);
			Ops::scale(y.data() + p, n - p, inverses[m]);
			pivots.push_back(p);
			reduced.push_back(std::move(y));
			basis.push_back(std::move(x));
			x = apply(basis.back());
		}
		return std::nullopt;
	}
	static Row power(const Row &relation, std::uint64_t k) {
		const int d = int(relation.size());
		Row p{1};
		for (int bit = int(std::bit_width(k)) - 1; bit >= 0; --bit) {
			const int m = int(p.size());
			Row reversed(p.rbegin(), p.rend()), square(2 * m - 1);
			for (int i = 0; i < int(square.size()); ++i) {
				int first = std::max(0, i - m + 1), last = std::min(m, i + 1);
				square[i] = Ops::dot(p.data() + first, reversed.data() + m - 1 - i + first, last - first);
			}
			for (int i = int(square.size()) - 1; i >= d; --i)
				if (square[i]) Ops::axpy(square.data() + i - d, relation.data(), d, square[i]);
			square.resize(std::min<int>(d, square.size()));
			p = std::move(square);
			if ((k >> bit) & 1) {
				p.insert(p.begin(), 0);
				if (int(p.size()) > d) {
					V lead = p.back();
					p.pop_back();
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
			}
		}
		p.resize(d);
		return p;
	}
	std::optional<std::vector<Row>> decompose(bool diagonal = true) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 32 && avx2()) {
			block = avx512_product() ? 16 : 8;
			packed.resize(transpose.size());
			for (int j = 0; j < n; j += block) {
				int width = std::min(block, n - j);
				for (int i = 0; i < n; ++i) for (int q = 0; q < width; ++q) packed[std::size_t(j) * n + std::size_t(i) * width + q] = center(transpose[std::size_t(i) * n + j + q]);
			}
		}
#endif
		for (int attempt = 0; attempt < 3; ++attempt) {
			basis.clear(), reduced.clear(), pivots.clear();
			work = 0;
			std::vector<Row> relations;
			bool valid = true;
			while (int(basis.size()) < n) {
				const int start = int(basis.size());
				Row x = seed();
				if (!diagonal) for (int p : pivots) x[p] = 0;
				auto rec = chain(std::move(x), diagonal ? 0 : start);
				if (!rec) { valid = false; break; }
				if (int(basis.size()) == start) {
					Row x(n);
					int p = 0;
					while (std::find(pivots.begin(), pivots.end(), p) != pivots.end()) ++p;
					x[p] = 1;
					rec = chain(std::move(x), diagonal ? 0 : start);
					if (!rec) { valid = false; break; }
				}
				const int d = int(basis.size()) - start;
				if (!diagonal && !start && d <= 2) return std::nullopt;
				if (diagonal && std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
					Row remainder = *rec, quotient(start + 1);
					for (int i = int(remainder.size()) - 1; i >= d; --i) {
						V factor = remainder[i];
						quotient[i - d] = factor;
						Ops::axpy(remainder.data() + i - d, rec->data() + start, d + 1, Ops::neg(factor));
					}
					Row x = basis[start];
					for (int j = 0; j < start; ++j) if (quotient[j]) Ops::axpy(x.data(), basis[j].data(), n, quotient[j]);
					basis.resize(start), reduced.resize(start), pivots.resize(start);
					rec = chain(std::move(x));
					if (!rec || int(basis.size()) != start + d || std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
						valid = false;
						break;
					}
				}
				Row relation(d);
				for (int i = 0; i < d; ++i) relation[i] = Ops::neg((*rec)[start + i]);
				relations.push_back(std::move(relation));
			}
			if (!valid) continue;
			return relations;
		}
		return std::nullopt;
	}
	std::optional<std::vector<T>> characteristic() {
		bool upper_triangular = true, lower_triangular = true;
		for (int i = 0; i < n && (upper_triangular || lower_triangular); ++i) for (int j = 0; j < i; ++j) {
			upper_triangular &= transpose[std::size_t(j) * n + i] == 0;
			lower_triangular &= transpose[std::size_t(i) * n + j] == 0;
		}
		if (upper_triangular || lower_triangular) {
			Row p{1};
			for (int i = 0; i < n; ++i) {
				Row next(p.size() + 1);
				std::copy(p.begin(), p.end(), next.begin() + 1);
				Ops::axpy(next.data(), p.data(), int(p.size()), Ops::neg(transpose[std::size_t(i) * n + i]));
				p = std::move(next);
			}
			std::vector<T> result(p.size());
			for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
			return result;
		}
		auto relations = decompose(false);
		if (!relations) return std::nullopt;
		Row p{1};
		for (const auto &relation : *relations) {
			const int d = int(relation.size());
			Row next(p.size() + d);
			for (int i = 0; i < int(p.size()); ++i) {
				next[i + d] = Ops::add(next[i + d], p[i]);
				if (p[i]) Ops::axpy(next.data() + i, relation.data(), d, Ops::neg(p[i]));
			}
			p = std::move(next);
		}
		std::vector<T> result(p.size());
		for (std::size_t i = 0; i < p.size(); ++i) result[i] = T::raw(p[i]);
		return result;
	}
	std::optional<std::vector<T>> run(std::uint64_t exponent) {
		auto relation_blocks = decompose();
		if (!relation_blocks) return std::nullopt;
		const auto &relations = *relation_blocks;
			std::vector<T> b(std::size_t(n) * n), companion(b.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				b[std::size_t(j) * n + i] = T::raw(basis[i][j]);
			int start = 0;
			for (const auto &relation : relations) {
				const int d = int(relation.size());
				Row p = power(relation, exponent);
				for (int j = 0; j < d; ++j) {
					for (int i = 0; i < d; ++i) companion[std::size_t(start + i) * n + start + j] = T::raw(p[i]);
					V lead = p.back();
					std::move_backward(p.begin(), p.end() - 1, p.end());
					p[0] = 0;
					if (lead) Ops::axpy(p.data(), relation.data(), d, lead);
				}
				start += d;
			}
			auto result = multiply(b, companion, n, n, n);
			Row columns(result.size());
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				columns[std::size_t(j) * n + i] = result[std::size_t(i) * n + j].val();
			for (int j = 0; j < n; ++j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = 0; i < j; ++i) if (upper[std::size_t(i) * n + j])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(upper[std::size_t(i) * n + j]));
				Ops::scale(column, n, inverses[j]);
			}
			for (int j = n - 1; j >= 0; --j) {
				auto *column = columns.data() + std::size_t(j) * n;
				for (int i = j + 1; i < n; ++i) if (reduced[j][pivots[i]])
					Ops::axpy(column, columns.data() + std::size_t(i) * n, n, Ops::neg(reduced[j][pivots[i]]));
			}
			for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
				result[std::size_t(i) * n + pivots[j]] = T::raw(columns[std::size_t(j) * n + i]);
		return result;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
