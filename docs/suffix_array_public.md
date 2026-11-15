---
title: Suffix Array
documentation_of: //string/suffix_array.hpp
library_category: string/substring
---

```cpp
#include "lib/string/suffix_array.hpp"
#include <cassert>

int main() {
	auto sa = suffix_array("banana");
	assert((sa == std::vector<int>{5, 3, 1, 0, 4, 2}));
}
```

| 関数 | 内容 |
| --- | --- |
| `suffix_array(s)` | `std::string_view`・`std::vector<T,Allocator>`・`std::span<T>`・`std::span<const T>` の空でない接尾辞を辞書順に並べ、その開始位置を `std::vector<int>` で返す |
| `suffix_array(s, upper)` | `std::vector<int,Allocator>`・`std::span<int>`・`std::span<const int>` を、各要素が `0 <= x <= upper` の整数列として処理する。`upper >= 0` |

`vector` は標準・独自どちらのアロケータでも使えます。`span` は固定長・動的長の両方に対応します。元の列は変更せず、返り値は入力を参照しません。位置は 0 始まりで `[0,s.size())` の範囲です。同じ値が複数回現れても使えます。整数列は負数や `uint64_t` の最大値を区別し、値の上限を指定しない呼び方では、利用者が値を番号に置き換える必要はありません。

長さは `INT_MAX` 以下です。空列は空配列、長さ 1 なら `[0]` を返します。文字列は符号なしバイト順です。一般の `T` は `<` による大小比較が必要です。`x < x` は偽とし、`x < y` かつ `y < z` なら `x < z` も成り立つようにします。また、どちら向きの `<` も偽となる要素を同値としたとき、`x` と `y`、`y` と `z` が同値なら `x` と `z` も同値となる必要があります。文字列と `vector<char>`・`span<char>`・`span<const char>` は、高位ビットのある文字で順序が異なる場合があります。後者は `char` の大小関係に従います。

```cpp
#include "lib/string/suffix_array.hpp"
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

int main() {
	std::vector<std::int64_t> values{1000000000000LL, -7, 1000000000000LL, -7};
	auto sa = suffix_array(std::span(values));
	assert((sa == std::vector<int>{3, 1, 2, 0}));
}
```

長さを $$n$$ として、文字列版は $$O(n)$$ 時間・空間、一般の列は $$O(n\log n)$$ 時間・$$O(n)$$ 空間です。整数の上限 `upper` を渡す版は、`upper <= INT_MAX-2` なら $$O(n+\mathrm{upper})$$ 時間・空間、より大きければ値を順序どおりに番号へ置き換える版へ切り替え、$$O(n\log n)$$ 時間・$$O(n)$$ 空間です。空列は定数時間・空間です。
