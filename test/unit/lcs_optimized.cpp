#include "string/lcs.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

template <class Sequence>
int oracle(const Sequence &s, const Sequence &t) {
	std::vector<int> row(t.size() + 1);
	for (auto x : s) {
		int diagonal = 0;
		for (std::size_t j = 0; j < t.size(); ++j) {
			int previous = row[j + 1];
			row[j + 1] = x == t[j] ? diagonal + 1 : std::max(row[j], previous);
			diagonal = previous;
		}
	}
	return row.back();
}

template <class Sequence>
bool subsequence(const Sequence &s, const Sequence &t) {
	std::size_t position = 0;
	for (auto x : t)
		if (position < s.size() && s[position] == x) ++position;
	return position == s.size();
}

template <class Sequence>
auto view(const Sequence &s) {
	if constexpr (std::is_same_v<Sequence, std::string>) return std::string_view(s);
	else return std::span<const typename Sequence::value_type>(s);
}

template <class T>
void check_profiles(const std::vector<T> &s, const std::vector<T> &t, int expected) {
	sequence_bit_masks_detail::CompressedPair<T> input(s, t);
	sequence_bit_masks_detail::MaskWorkspace workspace(input.symbols.size());
	auto run = [&]<bool reverse, int backend>() {
		auto row = lcs_detail::profile<reverse, backend>(input.first, input.second, workspace);
		for (int x : workspace.head) assert(x == -1);
		for (int x : workspace.tail) assert(x == -1);
		assert(lcs_detail::count(row) == expected);
		return row;
	};
	auto forward = run.template operator()<false, 1>(), backward = run.template operator()<true, 1>();
	assert((run.template operator()<false, 0>() == forward));
	assert((run.template operator()<true, 0>() == backward));
#if defined(__x86_64__)
	if (__builtin_cpu_supports("avx2")) {
		assert((run.template operator()<false, 2>() == forward));
		assert((run.template operator()<true, 2>() == backward));
	}
#if CP_LIBRARY_AVX512
	if (__builtin_cpu_supports("avx512f")) {
		assert((run.template operator()<false, 3>() == forward));
		assert((run.template operator()<true, 3>() == backward));
	}
#endif
#endif
}

template <class Sequence>
void check(const Sequence &s, const Sequence &t, int expected = -1) {
	if (expected < 0) expected = oracle(s, t);
	assert(lcs_length(s, t) == expected);
	assert(lcs_length(t, s) == expected);
	for (auto result : {lcs(s, t), lcs(t, s)}) {
		static_assert(std::is_same_v<decltype(result), Sequence>);
		assert(int(result.size()) == expected);
		assert(subsequence(result, s) && subsequence(result, t));
	}
	Sequence prefix(2, typename Sequence::value_type()), restored = prefix;
	int near = lcs_detail::near(view(s), view(t), &restored);
	if (s.size() + t.size() - 2 * std::size_t(expected) <= 8) {
		assert(near == expected && restored.size() == prefix.size() + expected);
		assert(std::equal(prefix.begin(), prefix.end(), restored.begin()));
		Sequence result(restored.begin() + prefix.size(), restored.end());
		assert(subsequence(result, s) && subsequence(result, t));
	} else assert(near == -1 && restored == prefix);
	if constexpr (!std::is_same_v<Sequence, std::string>)
		if (std::min(s.size(), t.size()) >= 63 && std::max(s.size(), t.size()) <= 1100) check_profiles(s, t, expected);
}

template <class T>
int unique_oracle(const std::vector<T> &s, const std::vector<T> &t) {
	std::map<T, int> position;
	for (int i = 0; i < int(s.size()); ++i) assert(position.emplace(s[i], i).second);
	std::vector<int> tail;
	for (T x : t) {
		auto found = position.find(x);
		if (found == position.end()) continue;
		auto next = std::lower_bound(tail.begin(), tail.end(), found->second);
		if (next == tail.end()) tail.push_back(found->second);
		else *next = found->second;
	}
	return int(tail.size());
}

void check_masks() {
	std::mt19937_64 rng(104837);
	for (int n : {63, 64, 65, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025}) {
		for (int alphabet : {2, 26, 256, 257, 1000}) {
			std::vector<std::int64_t> s(n), t(n + 17);
			auto value = [&](int id) { return std::numeric_limits<std::int64_t>::lowest() + (std::int64_t(id) << 40); };
			for (auto &x : s) x = value(int(rng() % alphabet));
			for (auto &x : t) x = value(int(rng() % alphabet));
			check(s, t);
		}
	}
	for (int length : {65, 257, 513, 1025}) {
		std::vector<std::uint64_t> s(length), t(length + 63);
		for (int i = 0; i < int(s.size()); ++i) s[i] = std::uint64_t(i) << 32;
		for (int i = 0; i < int(t.size()); ++i) t[i] = (std::uint64_t(i) << 32) | 1;
		for (int i = 0; i < int(s.size()); i += 63) s[i] = UINT64_MAX;
		for (int i = 1; i < int(t.size()); i += 64) t[i] = UINT64_MAX;
		check(s, t);
		for (int i = 0; i < int(s.size()); ++i)
			if (i % 64) s[i] = i % 3;
		for (int i = 0; i < int(t.size()); ++i)
			if (i % 64) t[i] = (i + 1) % 3;
		check(s, t);
		for (int i = 0; i < int(s.size()); ++i) s[i] = i % 3 ? i % 2 : (UINT64_C(1) << 63) | (std::uint64_t(i) << 32);
		for (int i = 0; i < int(t.size()); ++i) t[i] = i % 3 ? (i + 1) % 2 : (UINT64_C(1) << 63) | (std::uint64_t(i + 3) << 32);
		check(s, t);
	}
}

void check_near() {
	for (int edits : {4, 5}) {
		std::string s(8193, 'a'), t = s;
		for (int i = 0; i < edits; ++i) {
			s[(i + 1) * int(s.size()) / 6] = char('b' + i);
			t[(i + 1) * int(t.size()) / 6] = char('x' + i);
		}
		check(s, t, int(s.size()) - edits);
	}
	std::string moved(8193, 'a');
	moved[moved.size() / 4] = 'b';
	moved[moved.size() / 2] = 'c';
	moved[3 * moved.size() / 4] = 'd';
	auto shifted = moved;
	shifted.erase(shifted.begin() + shifted.size() / 4);
	shifted.insert(shifted.begin() + 3 * moved.size() / 4, 'b');
	check(moved, shifted, int(moved.size()) - 1);
	std::mt19937 rng(913697);
	for (int alphabet : {2, 26, 256}) {
		std::string s(1025, '\0');
		for (char &x : s) x = char(rng() % alphabet);
		for (int edits : {1, 2, 4, 5, 8, 9}) {
			auto t = s;
			for (int i = 0; i < edits; ++i) t.erase(t.begin() + (i + 1) * 79);
			check(s, t, int(t.size()));
			t = s;
			for (int i = 0; i < edits; ++i) t[(i + 1) * 83] = char((static_cast<unsigned char>(t[(i + 1) * 83]) + 1) % alphabet);
			check(s, t);
			t = s;
			for (int i = 0; i < edits; ++i) {
				char x = t[(i + 1) * 73];
				t.erase(t.begin() + (i + 1) * 73);
				t.insert(t.begin() + (i + 1) * 97, x);
			}
			check(s, t);
		}
	}
	for (int n : {257, 513, 4097, 32769}) {
		std::vector<std::int64_t> s(n);
		for (int i = 0; i < n; ++i) s[i] = INT64_MIN + (std::int64_t(i) << 32);
		for (int edits : {1, 4, 5}) {
			if (n > 4097 && edits > 1) continue;
			auto t = s;
			for (int i = 0; i < edits; ++i) {
				int from = (i + 1) * n / 13, to = (i + 7) * n / 13;
				auto x = t[from];
				t.erase(t.begin() + from);
				t.insert(t.begin() + to, x);
			}
			check(s, t, unique_oracle(s, t));
			t = s;
			for (int i = 0; i < edits; ++i) {
				t.erase(t.begin() + (i + 1) * n / 13);
				t.insert(t.begin() + (i + 7) * n / 13, INT64_MAX - i);
			}
			check(s, t, unique_oracle(s, t));
		}
	}
}

void check_permutations() {
	std::mt19937 rng(673853);
	for (int n : {257, 513, 1025, 4097}) {
		std::vector<std::uint64_t> s(n);
		for (int i = 0; i < n; ++i) s[i] = UINT64_MAX - (std::uint64_t(i) << 32);
		auto t = s;
		std::reverse(t.begin(), t.end());
		check(s, t, 1);
		t = s;
		std::rotate(t.begin(), t.begin() + n / 3, t.end());
		check(s, t, unique_oracle(s, t));
		std::shuffle(t.begin(), t.end(), rng);
		check(s, t, unique_oracle(s, t));
		for (int i = 0; i < n / 3; ++i) t[rng() % n] = UINT64_MAX - 1;
		check(s, t, unique_oracle(s, t));
	}
}

int main() {
	check_masks();
	check_near();
	check_permutations();
}
