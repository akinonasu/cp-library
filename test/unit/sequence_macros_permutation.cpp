#include <algorithm>
#include <iterator>
#include <vector>
#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)
#include "sequence/permutation_rank.hpp"
#include "sequence/permutation_rank.hpp"
#include "sequence/weighted_lis.hpp"
#include "test/fixtures/sequence_macros.hpp"

int main() { test_sequence_macros(); }
