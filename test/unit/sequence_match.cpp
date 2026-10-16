#include "../../components/sequence_match.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

template <class T>
void check(const std::vector<T> &s) {
	int n = static_cast<int>(s.size());
	sequence_match_detail::Matcher<T> match(s);
	sequence_match_detail::Matcher<T, false> portable(s);
	sequence_match_detail::Matcher<T> indexed(s);
	indexed.index_runs(n);
	for (int i = 0; i <= n; ++i) for (int j = 0; j <= n; ++j) {
		for (int limit = 0; limit <= std::min(n - i, n - j); ++limit) {
			int length = 0;
			while (length < limit && s[i + length] == s[j + length]) ++length;
			assert(match.forward(i, j, limit) == length);
			assert(portable.forward(i, j, limit) == length);
			assert(indexed.forward_counted(i, j, limit).first == length);
		}
		for (int limit = 0; limit <= std::min(i, j); ++limit) {
			int length = 0;
			while (length < limit && s[i - length - 1] == s[j - length - 1]) ++length;
			assert(match.backward(i, j, limit) == length);
			assert(portable.backward(i, j, limit) == length);
			assert(indexed.backward_counted(i, j, limit).first == length);
		}
	}
	std::vector<int> z(n);
	sequence_match_detail::z_table(n, z.data(), [&](int i, int j, int limit) { return match.forward(i, j, limit); });
	for (int i = 0; i < n; ++i) assert(z[i] == match.forward(0, i, n - i));
	for (int m = 1; m <= n; ++m) {
		sequence_match_detail::z_table(m, z.data(), [&](int i, int j, int limit) { return match.forward(i, j, limit); });
		sequence_match_detail::prefix_matches(n - m + 1, m, z.data(),
			[&](int i, int j, int limit) { return match.forward(i, j, limit); },
			[&](int i, int length) { assert(length == match.forward(0, i, m)); });
	}
}

int main() {
	check(std::vector<char>{});
	std::mt19937_64 random(713);
	for (int n = 0; n <= 40; ++n) {
		for (int alphabet : {1, 2, 5, 256}) {
			std::vector<char> bytes(n);
			std::vector<std::uint32_t> words(n);
			std::vector<std::uint64_t> wide(n);
			std::vector<__uint128_t> huge(n);
			for (int i = 0; i < n; ++i) {
				auto value = random() % alphabet;
				bytes[i] = static_cast<char>(value);
				words[i] = value * 0x01020304;
				wide[i] = value * 0x0102030405060708ULL;
				huge[i] = (__uint128_t(value) << 87) + value;
			}
			check(bytes);
			check(words);
			check(wide);
			check(huge);
		}
	}
	for (int n : {63, 64, 65, 71, 72, 73, 95, 96, 97, 127, 128, 129, 257}) {
		std::vector<unsigned char> s(n + 1, 0xff);
		sequence_match_detail::Matcher<unsigned char> match(s);
		sequence_match_detail::Matcher<unsigned char, false> portable(s);
		for (int mismatch = 0; mismatch <= n; ++mismatch) {
			if (mismatch < n) s[mismatch] = 0;
			int forward = 0, backward = 0;
			while (forward < n && s[forward] == s[forward + 1]) ++forward;
			while (backward < n && s[n - backward] == s[n - backward - 1]) ++backward;
			assert(match.forward(0, 1, n) == forward && portable.forward(0, 1, n) == forward);
			assert(match.backward(n, n + 1, n) == backward && portable.backward(n, n + 1, n) == backward);
			if (mismatch < n) s[mismatch] = 0xff;
		}
	}
}
