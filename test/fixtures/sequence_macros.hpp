#ifndef SEQUENCE_MACROS_HPP
#define SEQUENCE_MACROS_HPP

#ifndef all
#error "all macro must be preserved"
#endif

inline void test_sequence_macros() {
	std::vector<int> values{3, 1, 2};
	std::sort(all(values));
	assert((values == std::vector<int>{1, 2, 3}));
	std::vector<int> permutation{2, 0, 3, 1};
	assert(permutation_rank(permutation) == 13);
	assert(permutation_unrank(4, 13) == permutation);
	std::vector<int> symbols{300, 100, 200};
	std::vector<int> labelled{200, 300, 100};
	assert(permutation_rank(labelled, symbols) == 4);
	assert(permutation_unrank(symbols, 4) == labelled);
	auto alphabet_view = std::views::iota(10, 14);
	std::vector<int> view_permutation{13, 10, 12, 11};
	assert(permutation_rank(view_permutation, alphabet_view) == 19);
	assert(permutation_unrank(alphabet_view, 19) == view_permutation);
	std::vector<int> keys{3, 1, 2, 2, 4};
	std::vector<long long> weights{4, 6, -2, 7, 3};
	assert(weighted_lis_value(keys, weights) == 16);
	assert((weighted_lis(keys, weights) == std::vector<int>{1, 3, 4}));
	assert((weighted_lis_values(keys, weights) == std::vector<long long>{4, 6, 4, 13, 16}));
	auto key_view = std::views::iota(0, 4);
	std::vector<int> view_weights{2, -1, 4, 3};
	assert(weighted_lis_value(key_view, view_weights) == 9);
	assert((weighted_lis(key_view, view_weights) == std::vector<int>{0, 2, 3}));
	std::sort(all(symbols));
	assert((symbols == std::vector<int>{100, 200, 300}));
	struct All { bool all() const { return true; } };
	assert(All{}.all());
}

#endif
