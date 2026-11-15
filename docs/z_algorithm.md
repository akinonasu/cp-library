---
title: Z algorithm
documentation_of: //string/z_algorithm.hpp
library_category: string/pattern_matching
---

```cpp
#include "lib/string/z_algorithm.hpp"

auto z = z_algorithm("ababa"); // {5,0,3,0,1}
auto a = z_algorithm(std::vector<int>{1, 2, 1, 2}); // {4,0,2,0}
```

`z_algorithm(sequence)` は、各位置からの接尾辞と列全体との最長共通接頭辞の長さを `vector<int>` で返します。非空列では `z[0]=sequence.size()`、空列では空配列です。

`std::string_view`・`std::vector<T,Allocator>`・`std::span<T>`・`std::span<const T>` を渡せます。`vector` は標準・独自どちらのアロケータでも使え、`span` は固定長・動的長の両方に対応します。`T` は等値比較が可能とし、`vector<bool>` は対象外です。整数は負数や `uint64_t` の最大値も元の型のまま使えます。入力は変更せず、結果は入力を参照しません。列長は `INT_MAX` 以下、結果の添字と長さは要素単位です。引数の既定値はありません。

```cpp
#include "lib/string/z_algorithm.hpp"
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

int main() {
	std::vector<std::int64_t> values{1000000000000LL, -7, 1000000000000LL, -7};
	auto z = z_algorithm(std::span(values));
	assert((z == std::vector<int>{4, 0, 2, 0}));
}
```

列長を $$n$$ として、時間・追加空間は $$O(n)$$ です。
