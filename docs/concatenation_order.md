---
title: 文字列結合の辞書順比較・並べ替え
documentation_of: //string/concatenation_order.hpp
library_category: string
---

文字列や整数列を並べ替えて結合したときに、辞書順最小・最大となる順序や列を求めます。

```cpp
#include "lib/string/concatenation_order.hpp"
#include <vector>

std::vector<std::string> s{"b", "ba", "baa"};
int c = concatenation_compare("b", "ba"); // 1
auto p = concatenation_order(s);          // {2,1,0}
auto q = concatenation_order(s, true);    // {0,1,2}
auto small = min_concatenation(s);        // "baabab"
auto large = max_concatenation(s);        // "bbabaa"

std::vector<std::vector<int>> sequences{ {2}, {2, -1}, {2, -1, -1} };
auto indices = concatenation_order(sequences); // {2,1,0}
std::vector<int> smallest = min_concatenation(sequences); // {2,-1,-1,2,-1,2}
```

列の数を $$N$$、長さの合計を $$L$$ とします。比較する 2 列は `a,b` とし、長さを $$\lvert a\rvert,\lvert b\rvert$$ と表します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `concatenation_compare(a,b)` | `a+b` と `b+a` の辞書順比較を `int` で返す。小さい・等しい・大きいとき、それぞれ -1・0・1 | $$O(\lvert a\rvert+\lvert b\rvert)$$ |
| `concatenation_order(s,descending = false)` | 結合結果が辞書順最小となる元の添字の順列を `std::vector<int>` で返す。`true` なら最大 | $$O(N+L\log N)$$ |
| `min_concatenation(s)` | 辞書順最小の結合結果。文字列なら `std::string`、整数列なら `std::vector<T>` | $$O(N+L\log N)$$ |
| `max_concatenation(s)` | 辞書順最大の結合結果。文字列なら `std::string`、整数列なら `std::vector<T>` | $$O(N+L\log N)$$ |

`s` には `vector<string>`・`vector<string_view>`・配列・`span`・`list` などを渡せます。整数列は `vector<vector<T>>`、`vector<span<const T>>` などで指定でき、各列は `vector<T>`・`array<T,N>`・`span<T>` / `span<const T>` に対応します。`span` は長さを型で指定したもの、`vector` は独自の allocator を指定したものも使えます。全ての列の要素型 `T` を揃え、`bool` を除く 64 bit 以下の標準整数型にしてください。元の列を参照できる状態で呼び出し、処理中は要素を変更しないでください。

入力は変更しません。添字は `[0,N)` で、空列は順列の先頭に元の順序でまとめます。空でない列のうち `a+b == b+a` となるものも、元の順序を保ちます。空の入力に対しては空の順列・文字列または整数列を返します。

`concatenation_compare` は 2 つの `string_view`、同じ要素型の 2 つの `vector<T>`、または 2 つの `span<T>` / `span<const T>` を受け取ります。固定長の `span` にも対応します。一方が空なら 0 を返すため、空列を含む列のソートには `concatenation_order` を使ってください。

文字列は NUL を含む全 256 種類のバイトに対応し、バイトを符号なしとして比較します。整数列は要素型の通常の大小関係で比較するため、負数・型の最小値と最大値もそのまま使えます。文字列の場合も整数列の場合も、列数と各列の長さはそれぞれ `INT_MAX` 以下としてください。

並べ替え・結合は時間 $$O(N+L\log N)$$、追加空間 $$O(N+L)$$ です。$$N\le1$$ の並べ替え・結合は時間 $$O(N+L)$$ です。2 列の比較だけなら追加空間は $$O(1)$$ です。

参考：[AtCoder Algorithm Lectures：文字列の並べ替えと辞書順最小化](https://info.atcoder.jp/entry/algorithm_lectures/lex_min_string_concatenation)
