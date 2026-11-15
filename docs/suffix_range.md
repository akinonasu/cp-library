---
title: Suffix Range Core
documentation_of: //components/suffix_range.hpp
library_category: components/string
---

固定した本文の接尾辞配列上で、パターンの出現区間を結合・切り出しする内部部品です。`DynamicPatternMatching` の断片の照合に使います。編集可能なパターン全体の管理には `DynamicPatternMatching` を使います。

```cpp
#include "lib/components/suffix_range.hpp"

int main() {
	SuffixRangeIndex index("banana");
	auto an = index.concat(index.character('a'), index.character('n'));
	auto ana = index.concat(an, index.character('a'));
	assert(ana.length == 3 && ana.right - ana.left == 2);
	assert(index.suffix_array()[ana.left] == 3);
	auto na = index.substring(ana, 1, 3);
	assert(na.length == 2 && na.right - na.left == 2);
	auto [prefix, length] = index.longest_prefix("anax");
	assert(length == 3 && prefix.left == ana.left && prefix.right == ana.right);
	assert(index.empty_range().right == 7);
}
```

`Range` は `left`・`right`・`length` の 3 つの `int` を持ちます。本文に現れるパターンについて、接尾辞配列上の `[left,right)` が全出現位置、`length` がパターンのバイト数です。出現回数は `right-left`、各出現位置は `suffix_array()[left]`～`suffix_array()[right-1]` です。位置の昇順ではありません。

本文に現れない場合は `left == right` となり、`length` は引き続きそのパターンの長さを表します。空パターンの区間には `empty_range()` を使います。空パターンは本文長を $$n$$ として $$n+1$$ 箇所に現れます。接尾辞配列は空の接尾辞を含み、先頭の値は `n`、配列長は `n+1` です。

`concat(a,b)` は 2 つのパターンをこの順に連結した出現区間を返します。パターンそのものの文字列は不要です。`substring(range,l,r)` はパターン内の `[l,r)` を切り出します。切り出しには **元のパターンが本文に現れること** が必要です。元の出現区間が空の場合は使えません。

`longest_prefix(pattern)` は、渡した文字列の先頭から本文に一致する最長の部分と、その長さを返します。パターン全体が現れなくても、途中まで一致した区間を返します。最初のバイトも一致しない場合は、空パターンの区間と長さ 0 です。

本文は内部にコピーします。文字列版には `string_view` のほか `vector<char>`・`array<char,N>`・`span<char>`・`span<const char>` も渡せます。NUL を含む全 256 バイトに対応し、辞書順は符号なしバイトの順です。本文長は `INT_MAX/2` 未満、`concat` 後のパターン長は `INT_MAX` 以下にしてください。`Range` は同じ索引から取得したものを渡し、フィールドを書き換えずに使います。

整数列には、`char` 以外の整数型 `T` を指定した `BasicSuffixRangeIndex<T>` を使います。`T` は `bool` を除く 64 bit 以下の整数型で、負数や `uint64_t` の最大値も扱います。本文・`longest_prefix` のパターンは同じ型の `std::span<const T>`、またはそこへ変換できる `vector`・`array`・`span` で渡します。`character(c)` の値は `T`、`text()` の返り値は `std::span<const T>` です。辞書順は整数の大小順、位置と長さは要素単位になります。`SuffixRangeIndex` は文字列を扱う `BasicSuffixRangeIndex<char>` と同じ型です。


本文長を $$n$$、最長一致接頭辞の長さを $$\ell$$、本文の値の種類数を $$\sigma$$ とします。文字列では $$\sigma\leq256$$ です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SuffixRangeIndex()` / `BasicSuffixRangeIndex<T>()` | 空の本文の索引を作る | $$O(1)$$ |
| `SuffixRangeIndex(text)` / `BasicSuffixRangeIndex<T>(text)` | 本文の索引を作る | 文字列は $$O(n)$$、整数列は $$O(n\log n)$$ |
| `size()` / `text()` | 本文長 / 保存した本文の `string_view` または `span<const T>` | $$O(1)$$ |
| `suffix_array()` | 空の接尾辞を含む接尾辞配列への const 参照 | $$O(1)$$ |
| `empty_range()` | 空パターンの区間 | $$O(1)$$ |
| `character(c)` | 1 バイト / 1 要素のパターンの区間 | 文字列は $$O(1)$$、整数列は $$O(\log\sigma)$$ |
| `concat(a,b)` | 2 つのパターンを連結した区間 | $$O(\log n)$$ |
| `substring(range,l,r)` | 一致するパターン内の半開区間を切り出す | $$O(\log n)$$ |
| `longest_prefix(pattern)` | 最長一致接頭辞の `Range` と長さの組 | 文字列は $$O(\ell)$$、整数列は $$O(\ell\log\sigma)$$。不一致の判定は 1 要素分加わる |
| コピー構築・コピー代入 | 索引全体をコピー | $$O(n)$$。代入は旧領域の解放も加わる |
| ムーブ構築・ムーブ代入 | 索引を移動 | 構築 $$O(1)$$。代入は旧領域の解放も加わる |

本文長や種類数が 0・1 の場合は、対数を含む時間を定数として扱います。`substring` には `0 <= l <= r <= range.length` を渡します。保持空間・構築時の追加空間は $$O(n)$$、`substring` の追加空間は $$O(\log n)$$、他の問い合わせの追加空間は $$O(1)$$ です。
