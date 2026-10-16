#ifndef RUN_ENUMERATE_HPP
#define RUN_ENUMERATE_HPP

#include "../components/sequence_match.hpp"
#include "../components/default_init_allocator.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <compare>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

struct Run {
	int period, l, r;
	friend auto operator<=>(const Run &, const Run &) = default;
};

namespace run_enumerate_detail {
template <class Key>
void radix_sort(std::vector<Run> &runs, std::vector<Run> &scratch, int bits, Key key) {
	scratch.resize(runs.size());
	for (int shift = 0; shift < bits; shift += 11) {
		std::array<std::size_t, 2048> count{};
		for (auto run : runs) ++count[(key(run) >> shift) & 2047];
		std::size_t sum = 0;
		for (auto &position : count) {
			auto next = sum + position;
			position = sum;
			sum = next;
		}
		for (auto run : runs) scratch[count[(key(run) >> shift) & 2047]++] = run;
		runs.swap(scratch);
	}
}

inline void sort_runs(std::vector<Run> &runs, int n) {
	if (runs.size() < 512) std::sort(runs.begin(), runs.end());
	else {
		std::vector<Run> scratch;
		int bits = std::bit_width(unsigned(n));
		radix_sort(runs, scratch, 2 * bits, [bits](Run run) { return (std::uint64_t(run.l) << bits) | unsigned(run.r); });
		radix_sort(runs, scratch, bits, [](Run run) { return unsigned(run.period); });
	}
	runs.erase(std::unique(runs.begin(), runs.end()), runs.end());
}

template <bool Indexed, class T>
bool lyndon_impl(std::span<const T> s, std::vector<Run> &runs, std::uint64_t budget, sequence_match_detail::Matcher<T, false> &match) {
	int n = static_cast<int>(s.size());
	auto extend = [&](bool backward, int i, int j, int limit) {
		if constexpr (Indexed) {
			auto [length, work] = backward ? match.backward_counted(i, j, limit) : match.forward_counted(i, j, limit);
			if (std::uint64_t(work) > budget) return -1;
			budget -= work;
			return length;
		} else {
			int allowed = static_cast<int>(std::min(std::uint64_t(limit), budget));
			int length = backward ? match.backward(i, j, allowed) : match.forward(i, j, allowed);
			budget -= length;
			return length == allowed && allowed < limit ? -1 : length;
		}
	};
	std::vector<int, DefaultInitAllocator<int>> stack(n + 1);
	for (int opposite = 0; opposite < 2; ++opposite) {
		int top = 0;
		stack[0] = n;
		for (int i = n - 1; i >= 0; --i) {
			int common = 0;
			while (top) {
				int j = stack[top], left = j - i, right = stack[top - 1] - j;
				int limit = std::min(left, right);
				common = s[i] == s[j] ? extend(false, i, j, limit) : 0;
				if (common < 0) return false;
				if ((common == limit && left < right) ||
					(common < limit && (bool(s[i + common] < s[j + common]) != bool(opposite)))) {
					--top;
					common = 0;
				} else break;
			}
			int j = stack[top];
			stack[++top] = i;
			if (j == n) continue;
			int period = j - i;
			int back = i && s[i - 1] == s[j - 1] ? extend(true, i, j, std::min(i, period)) : 0;
			if (back < 0) return false;
			if (back == period) continue;
			int ahead = common;
			if (j + common < n && s[i + common] == s[j + common]) {
				int extra = extend(false, i + common, j + common, n - j - common);
				if (extra < 0) return false;
				ahead += extra;
			}
			if (back + ahead >= period) runs.push_back({period, i - back, j + ahead});
		}
	}
	return true;
}

template <class T>
bool lyndon(std::span<const T> s, std::vector<Run> &runs, std::uint64_t budget) {
	int n = static_cast<int>(s.size());
	sequence_match_detail::Matcher<T, false> match(s);
	if (s[0] == s[1]) {
		sequence_match_detail::Matcher<T> whole(s);
		int length = whole.forward(0, 1, n - 1);
		if (std::uint64_t(length) > budget) return false;
		budget -= length;
		if (length == n - 1) {
			runs.push_back({1, 0, n});
			return true;
		}
	}
	if (n >= 128) {
		int equal = 0;
		for (int k = 1; k <= 64; ++k) {
			int i = static_cast<int>(std::size_t(n - 1) * k / 64);
			equal += s[i - 1] == s[i];
		}
		if (equal >= 56) {
			match.index_runs(n);
			return lyndon_impl<true>(s, runs, budget, match);
		}
	}
	return lyndon_impl<false>(s, runs, budget, match);
}

template <class T>
void divide(std::span<const T> s, std::vector<Run> &runs) {
	int n = static_cast<int>(s.size());
	if (n < 2) return;
	sequence_match_detail::Matcher<T, false> match(s);
	std::vector<int, DefaultInitAllocator<int>> z(n);
	int *left_z = z.data(), *right_z = z.data() + n / 2;
	std::vector<Run> candidates, scratch;
	auto add = [&](int period, int l, int r, int middle) {
		if (l >= middle || r <= middle || (r - l) / 2 < period) return;
		if (l && s[l - 1] == s[l + period - 1]) return;
		if (r < n && s[r] == s[r - period]) return;
		candidates.push_back({period, l, r});
	};
	auto recurse = [&](auto &&self, int first, int last) -> void {
		if (last - first <= 16) {
			for (int period = 1; period <= (last - first) / 2; ++period) {
				int start = first;
				for (int i = first; i < last - period; ++i) {
					if (s[i] != s[i + period]) {
						if (i - start >= period) add(period, start, i + period, start + 1);
						start = i + 1;
					}
				}
				if (last - period - start >= period) add(period, start, last, start + 1);
			}
		} else {
			int middle = first + (last - first) / 2, left = middle - first, right = last - middle;
			self(self, first, middle);
			self(self, middle, last);
			sequence_match_detail::z_table(left, left_z, [&](int a, int b, int count) { return match.backward(middle - a, middle - b, count); });
			sequence_match_detail::z_table(right, right_z, [&](int a, int b, int count) { return match.forward(middle + a, middle + b, count); });
			sequence_match_detail::prefix_matches(left, right, right_z,
				[&](int a, int b, int count) { return match.forward(middle + a, first + b, count); },
				[&](int i, int length) {
					int period = left - i, back = period < left ? left_z[period] : 0;
					add(period, middle - period - back, middle + length, middle);
				});
			sequence_match_detail::prefix_matches(right, left, left_z,
				[&](int a, int b, int count) { return match.backward(middle - a, last - b, count); },
				[&](int i, int length) {
					int period = right - i, ahead = period < right ? right_z[period] : 0;
					add(period, middle - length, middle + period + ahead, middle);
				});
		}
		if (candidates.size() < 512) {
			std::sort(candidates.begin(), candidates.end(), [](Run a, Run b) {
				return std::tie(a.l, a.r, a.period) < std::tie(b.l, b.r, b.period);
			});
		} else {
			int bits = std::bit_width(unsigned(n));
			radix_sort(candidates, scratch, 2 * bits, [bits](Run run) { return (std::uint64_t(run.l) << bits) | unsigned(run.r); });
		}
		for (std::size_t i = 0; i < candidates.size();) {
			Run run = candidates[i++];
			while (i < candidates.size() && candidates[i].l == run.l && candidates[i].r == run.r)
				run.period = std::min(run.period, candidates[i++].period);
			runs.push_back(run);
		}
		candidates.clear();
	};
	recurse(recurse, 0, n);
}
}

template <class T>
std::vector<Run> run_enumerate(std::span<const T> s) {
	assert(s.size() < std::size_t(std::numeric_limits<int>::max()));
	std::vector<Run> runs;
	int n = static_cast<int>(s.size());
	if (n < 2) return runs;
	if (!run_enumerate_detail::lyndon(s, runs, std::uint64_t(n) * 64)) {
		runs.clear();
		run_enumerate_detail::divide(s, runs);
	}
	run_enumerate_detail::sort_runs(runs, n);
	return runs;
}

inline std::vector<Run> run_enumerate(std::string_view s) { return run_enumerate(std::span<const char>(s.data(), s.size())); }

template <class T, class Allocator>
std::vector<Run> run_enumerate(const std::vector<T, Allocator> &s) { return run_enumerate(std::span<const T>(s.data(), s.size())); }

#endif
