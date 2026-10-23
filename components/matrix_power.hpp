#ifndef MATRIX_POWER_HPP
#define MATRIX_POWER_HPP

#include "matrix_arithmetic.hpp"
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
	std::uint64_t random = 0x243f6a8885a308d3ULL;
	explicit FrobeniusPower(const std::vector<T> &a, int size) : n(size), transpose(a.size()), upper(a.size()), inverses(size) {
		for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) transpose[std::size_t(j) * n + i] = a[std::size_t(i) * n + j].val();
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
		for (int i = 0; i < n; ++i) if (x[i]) Ops::axpy(result.data(), transpose.data() + std::size_t(i) * n, n, x[i]);
		return result;
	}
	std::optional<Row> chain(Row x) {
		while (++work <= 4 * n) {
			const int m = int(basis.size());
			Row y = x, factors(m);
			for (int j = 0; j < m; ++j) {
				int p = pivots[j];
				V factor = factors[j] = y[p];
				if (factor) Ops::axpy(y.data() + p, reduced[j].data() + p, n - p, Ops::neg(factor));
			}
			int p = 0;
			while (p < n && !y[p]) ++p;
			if (p == n) {
				for (int i = m - 1; i >= 0; --i) {
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
	std::optional<std::vector<T>> run(std::uint64_t exponent) {
		for (int attempt = 0; attempt < 3; ++attempt) {
			basis.clear(), reduced.clear(), pivots.clear();
			work = 0;
			std::vector<Row> relations;
			bool valid = true;
			while (int(basis.size()) < n) {
				const int start = int(basis.size());
				auto rec = chain(seed());
				if (!rec) { valid = false; break; }
				if (int(basis.size()) == start) {
					Row x(n);
					int p = 0;
					while (std::find(pivots.begin(), pivots.end(), p) != pivots.end()) ++p;
					x[p] = 1;
					rec = chain(std::move(x));
					if (!rec) { valid = false; break; }
				}
				const int d = int(basis.size()) - start;
				if (std::any_of(rec->begin(), rec->begin() + start, [](V v) { return v != 0; })) {
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
		return std::nullopt;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
