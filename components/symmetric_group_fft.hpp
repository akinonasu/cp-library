#ifndef SYMMETRIC_GROUP_FFT_HPP
#define SYMMETRIC_GROUP_FFT_HPP

#include "../math/mint.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <map>
#include <numeric>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

template <class Mint = mint>
class SymmetricGroupFFT {
  public:
	struct Block {
		std::vector<int> partition;
		int dimension = 0, offset = 0;
	};
  private:
	struct Shape {
		Block block;
		int first = 0;
		std::map<int, int> branches;
	};
	struct Tableau {
		std::uint64_t code;
		int shape, parent, content;
	};
	struct Level {
		std::vector<Shape> shapes;
		std::vector<Tableau> tableaux;
		std::vector<std::vector<int>> children;
		std::map<std::vector<int>, int> shapes_by_parts;
		std::map<std::uint64_t, int> tableaux_by_code;
	};
	struct Copy { int from, to, width; };
	struct Term { int from, to, width; Mint coefficient; };
	struct Step {
		std::vector<Copy> copies;
		std::vector<Term> terms;
	};
	struct Row { int shape; std::uint64_t suffix; int beta, cut, offset; };
	int n;
	std::uint64_t modulus;
	std::vector<int> factorial, order, inverse_order;
	std::vector<Block> block_data;
	std::vector<std::vector<Step>> plans;
	static void partitions(int left, int maximum, std::vector<int> &parts, std::vector<std::vector<int>> &out) {
		if (!left) { out.push_back(parts); return; }
		for (int x = std::min(left, maximum); x; --x) {
			parts.push_back(x);
			partitions(left - x, x, parts, out);
			parts.pop_back();
		}
	}
	void build_plans() {
		std::vector<Mint> reciprocals(2 * n + 1);
		for (int i = 1; i <= n; ++i) {
			reciprocals[n + i] = Mint(1) / Mint(i);
			reciprocals[n - i] = -reciprocals[n + i];
		}
		std::vector<Level> levels(n + 1);
		for (int k = 0; k <= n; ++k) {
			auto &level = levels[k];
			std::vector<std::vector<int>> parts;
			std::vector<int> path;
			partitions(k, k, path, parts);
			int offset = 0;
			for (const auto &p : parts) {
				int id = int(level.shapes.size());
				Shape shape;
				shape.block.partition = p;
				shape.first = int(level.tableaux.size());
				if (!k) level.tableaux.push_back({0, id, 0, 0});
				for (int r = 0; r < int(p.size()); ++r) {
					if (r + 1 < int(p.size()) && p[r] == p[r + 1]) continue;
					auto previous = p;
					if (!--previous[r]) previous.pop_back();
					const auto &lower = levels[k - 1];
					int mu = lower.shapes_by_parts.at(previous);
					shape.branches.emplace(mu, int(level.tableaux.size()) - shape.first);
					const auto &parent = lower.shapes[mu];
					for (int j = 0; j < parent.block.dimension; ++j) {
						int t = parent.first + j;
						level.tableaux.push_back({lower.tableaux[t].code | (std::uint64_t(r) << (4 * (k - 1))), id, t, p[r] - 1 - r});
					}
				}
				shape.block.dimension = int(level.tableaux.size()) - shape.first;
				shape.block.offset = offset;
				offset += shape.block.dimension * shape.block.dimension;
				level.shapes_by_parts.emplace(p, id);
				level.shapes.push_back(std::move(shape));
			}
			assert(offset == factorial[k]);
			if (k) levels[k - 1].children.resize(levels[k - 1].tableaux.size());
			for (int t = 0; t < int(level.tableaux.size()); ++t) {
				level.tableaux_by_code.emplace(level.tableaux[t].code, t);
				if (k) levels[k - 1].children[level.tableaux[t].parent].push_back(t);
			}
		}
		for (const auto &shape : levels[n].shapes) block_data.push_back(shape.block);
		for (int m = 2; m <= n; ++m) {
			const auto &previous = levels[m - 1];
			std::vector<Row> rows;
			for (int mu = 0; mu < int(previous.shapes.size()); ++mu) {
				const auto &s = previous.shapes[mu];
				for (int j = 0; j < s.block.dimension; ++j)
					rows.push_back({mu, previous.tableaux[s.first + j].code, 0, 0, s.block.offset + j * s.block.dimension});
			}
			for (int i = 2; i <= m; ++i) {
				Step step;
				std::vector<Row> next;
				std::map<std::tuple<int, std::uint64_t, int>, int> ids;
				int size = 0;
				for (const auto &row : rows) {
					auto a = levels[i - 2].shapes[row.cut].block.partition;
					int r = int(row.suffix & 15);
					if (r == int(a.size())) a.push_back(1);
					else ++a[r];
					int cut = levels[i - 1].shapes_by_parts.at(a);
					int bp = levels[i - 1].tableaux[row.beta].shape;
					int width = previous.shapes[row.shape].block.dimension;
					for (int beta : levels[i - 1].children[row.beta]) {
						const auto &tableau = levels[i].tableaux[beta];
						const auto &shape = levels[i].shapes[tableau.shape];
						if (!shape.branches.contains(cut)) continue;
						auto key = std::tuple(row.shape, row.suffix >> 4, beta);
						auto [it, added] = ids.emplace(key, int(next.size()));
						if (added) {
							int offset = i == m ? shape.block.offset + (beta - shape.first) * shape.block.dimension + shape.branches.at(row.shape) : size;
							next.push_back({row.shape, row.suffix >> 4, beta, cut, offset});
							size += width;
						}
						const auto &before = levels[i - 2].shapes[row.cut].block.partition;
						const auto &after = levels[i - 1].shapes[bp].block.partition;
						int corner = 0;
						while (corner < int(before.size()) && before[corner] == after[corner]) ++corner;
						int d = tableau.content - (after[corner] - 1 - corner);
						assert(d && -n <= d && d <= n);
						Mint coefficient = reciprocals[n + d] + Mint(bp != cut);
						step.terms.push_back({row.offset, next[it->second].offset, width, coefficient});
					}
				}
				assert(size == i * factorial[m - 1]);
				for (const auto &row : next) {
					const auto &parent = levels[i - 1].tableaux[levels[i].tableaux[row.beta].parent];
					if (parent.shape != row.cut) continue;
					std::uint64_t code = (row.suffix << (4 * (i - 1))) | parent.code;
					int t = previous.tableaux_by_code.at(code);
					const auto &shape = previous.shapes[row.shape];
					step.copies.push_back({shape.block.offset + (t - shape.first) * shape.block.dimension, row.offset, shape.block.dimension});
				}
				plans[m].push_back(std::move(step));
				rows.swap(next);
			}
		}
	}
	int rank(const std::vector<int> &p, bool reversed) const {
		unsigned used = 0;
		int result = 0;
		for (int i = 0; i < n; ++i) {
			int x = p[reversed ? n - 1 - i : i];
			result = result * (n - i) + x - std::popcount(used & ((1u << x) - 1));
			used |= 1u << x;
		}
		return result;
	}
	void transform(int m, Mint *a, std::vector<Mint> &x, std::vector<Mint> &y, bool transpose) const {
		if (m <= 1) return;
		int sub = factorial[m - 1];
		if (!transpose) {
			for (int i = 0; i < m; ++i) transform(m - 1, a + i * sub, x, y, false);
			x.assign(a, a + sub);
			for (int i = 2; i <= m; ++i) {
				const auto &step = plans[m][i - 2];
				y.assign(i * sub, Mint(0));
				for (auto copy : step.copies) std::copy_n(a + (i - 1) * sub + copy.from, copy.width, y.data() + copy.to);
				for (const auto &term : step.terms)
					for (int j = 0; j < term.width; ++j) y[term.to + j] += term.coefficient * x[term.from + j];
				x.swap(y);
			}
			std::copy(x.begin(), x.end(), a);
		} else {
			x.assign(a, a + factorial[m]);
			std::fill_n(a, factorial[m], Mint(0));
			for (int i = m; i >= 2; --i) {
				const auto &step = plans[m][i - 2];
				y.assign((i - 1) * sub, Mint(0));
				for (auto copy : step.copies)
					for (int j = 0; j < copy.width; ++j) a[(i - 1) * sub + copy.from + j] += x[copy.to + j];
				for (const auto &term : step.terms)
					for (int j = 0; j < term.width; ++j) y[term.from + j] += term.coefficient * x[term.to + j];
				x.swap(y);
			}
			std::copy(x.begin(), x.end(), a);
			for (int i = 0; i < m; ++i) transform(m - 1, a + i * sub, x, y, true);
		}
	}
	void check(std::size_t size) const {
		assert(modulus == std::uint64_t(Mint::mod()));
		if (size != std::size_t(factorial[n])) throw std::invalid_argument("SymmetricGroupFFT size");
	}
  public:
	explicit SymmetricGroupFFT(int degree = 0) : n(degree), modulus(Mint::mod()) {
		if (n < 0 || n > 10 || modulus <= unsigned(n)) throw std::invalid_argument("SymmetricGroupFFT degree or modulus");
		factorial.assign(n + 1, 1);
		for (int i = 1; i <= n; ++i) factorial[i] = factorial[i - 1] * i;
		plans.resize(n + 1);
		build_plans();
		order.resize(factorial[n]);
		inverse_order.resize(factorial[n]);
		std::vector<int> p(n), inverse(n);
		std::iota(p.begin(), p.end(), 0);
		int index = 0;
		do {
			int internal = rank(p, true);
			order[index++] = internal;
			for (int i = 0; i < n; ++i) inverse[p[i]] = i;
			inverse_order[internal] = rank(inverse, false);
		} while (std::next_permutation(p.begin(), p.end()));
	}
	int degree() const { return n; }
	int size() const { return factorial[n]; }
	const std::vector<Block> &blocks() const { return block_data; }
	std::vector<Mint> forward(std::span<const Mint> a) const {
		check(a.size());
		std::vector<Mint> result(a.size()), x, y;
		for (int i = 0; i < size(); ++i) result[order[i]] = a[i];
		transform(n, result.data(), x, y, false);
		return result;
	}
	std::vector<Mint> inverse(std::span<const Mint> a) const {
		check(a.size());
		std::vector<Mint> result(a.size()), work(a.size()), x, y;
		Mint scale = Mint(1) / Mint(size());
		for (const auto &block : block_data) {
			int d = block.dimension, offset = block.offset;
			Mint weight = scale * Mint(d);
			for (int i = 0; i < d; ++i) for (int j = 0; j < d; ++j) work[offset + i * d + j] = a[offset + j * d + i] * weight;
		}
		transform(n, work.data(), x, y, true);
		for (int i = 0; i < size(); ++i) result[inverse_order[i]] = work[i];
		return result;
	}
};

#endif
