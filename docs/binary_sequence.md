---
title: 0・1 の整数列の読み取り
documentation_of: //components/binary_sequence.hpp
library_category: components/data_structure
---

[DynamicBitset](dynamic_bitset.md)・[mod 2 行列](matrix_mod2.md) で、0・1 の整数列を 64 bit ごとにまとめる内部部品です。通常は公開ライブラリを直接使います。

```cpp
#include "lib/components/binary_sequence.hpp"
#include <array>
#include <cassert>
#include <cstdint>

int main() {
	std::array<int, 3> values{1, 0, 1};
	std::uint64_t word;
	binary_sequence_detail::pack(values, &word);
	assert(word == 5);
}
```

名前は全て `binary_sequence_detail` 内にあります。`Sequence<Range>` は、0・1 を読み取れる 1 次元の整数列を指定するための条件です。`vector<T>`・`vector<bool>`・`array<T,N>`・`span<T>` / `span<const T>`・通常の配列・`deque<T>` など、長さと各添字の要素を定数時間で取得できるものを扱います。要素型は `bool` を含む 64 bit 以下の標準整数型です。`std::string`・`char[N]` など `std::string_view` に変換できる文字列は含まず、`'0'`・`'1'` の文字を扱う処理とは分けます。`char` の 0・1 配列は `span` または `array` で渡します。`Matrix<Range>` は、同じ条件を満たす整数列を行として並べた 2 次元の配列を指定する条件です。

列長を $$n$$、格納に使う 64 bit 整数の個数を $$W=\lceil n/64\rceil$$ とします。

| 操作 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `size(values)` | 列長を `int` で返す。長さを取得できる列を受け取る | $$O(1)$$ | $$O(1)$$ |
| `pack(values,words)` | 添字順にビットへ格納し、`void` を返す | $$O(n)$$ | $$O(1)$$ |

長さは `INT_MAX` 以下、`pack` に渡す値は 0 または 1 にしてください。`words` は $$W$$ 個以上の `uint64_t` を書き込める領域の先頭を指すものとし、入力とは重ならないようにします。既定の引数はありません。入力は変更しません。`values[i]` は `words[i/64]` の下位から `i%64` 番目に格納されます。最後の整数のうち列長を超えるビットは 0 になります。列長 0 なら書き込みは行わず、`words` が `nullptr` でも構いません。

`size` は追加空間 $$O(1)$$、`pack` は書き込み先を除く追加空間 $$O(1)$$ です。`pack` の書き込み先は $$O(W)$$ 空間を使います。要素のコピーや比較を定数時間として扱い、共有する前計算やキャッシュは持ちません。
