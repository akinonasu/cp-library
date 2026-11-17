---
title: 重み付き最長増加部分列
documentation_of: //sequence/weighted_lis.hpp
library_category: sequence
---

列の各要素に重みを付け、値が増加する部分列のうち、**重みの合計**が最大になるものを求めます。要素を飛ばして選べます。重みを全て 1 にすると、通常の [最長増加部分列](lis.md) と同じ最大長になります。

```cpp
#include "lib/sequence/weighted_lis.hpp"
#include <cassert>
#include <vector>

std::vector<int> a{3, 1, 2, 2, 4};
std::vector<long long> weights{4, 6, -2, 7, 3};
assert(weighted_lis_value(a, weights) == 16);
assert((weighted_lis(a, weights) == std::vector<int>{1, 3, 4}));
assert((weighted_lis_values(a, weights) == std::vector<long long>{4, 6, 4, 13, 16}));
```

列の長さを $$n$$、重みの要素型を `Weight` とします。

| 操作 | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `weighted_lis_value(a,weights,compare=std::less<>{})` | 狭義増加部分列の重み合計の最大値を `Weight` で返す | $$O(n\log n)$$ |
| `weighted_lis(a,weights,compare=std::less<>{})` | 最大合計となる部分列の元の添字を `std::vector<int>` で返す | $$O(n\log n)$$ |
| `weighted_lis_values(a,weights,compare=std::less<>{})` | 各位置を最後に選ぶ部分列の重み合計の最大値を `std::vector<Weight>` で返す | $$O(n\log n)$$ |
| 上の各関数の `<false>` 版 | 値が等しい要素も続けて選べる広義増加で同じ計算を行う | $$O(n\log n)$$ |

通常の LIS と同じく、最初のテンプレート引数 `Strict` の既定値は `true` です。`true` では続けて選ぶ値 `x,y` が `compare(x,y)==true` を満たす必要があります。`false` では `compare(y,x)==false` なら続けられます。例えば `weighted_lis_value<false>(a,weights)` で広義増加の最大合計を求めます。

`a` と `weights` は長さを揃え、`vector`・`array`・`span`・`deque`・`string` など、先頭から指定した位置の要素を定数時間で取得できる列にします。固定長の `span`、独自の allocator を指定した `vector` にも対応します。長さは `INT_MAX` 以下としてください。入力は変更しません。

値の型は整数・文字列・構造体などを使えます。`compare(x,y)` は `x` を `y` より前の値とするときに `true`、それ以外は `false` を返すものにします。同じ値同士では `false` とし、`x` が `y` より前、`y` が `z` より前なら `x` が `z` より前になるよう、順序を揃えてください。`std::greater<>{}` を渡せば減少する部分列を求められます。

`weights` には `int`・`long long`・`__int128_t`・浮動小数点などの数値型を使います。`Weight{}` が 0、`+` が加算、`<` が数値の大小比較になるものとし、コピー・代入できるようにしてください。全ての計算結果が `Weight` の範囲に収まる必要があり、浮動小数点では NaN を含めないものとします。`int` の重みを渡すと合計も `int` です。大きな合計が必要なら `long long` などで重みを渡してください。

添字は 0 始まりで、`weighted_lis` は昇順の添字列を返します。最適な答えが複数ある場合は、そのうち 1 つを返します。空の部分列も許すため、`weighted_lis_value` は 0 以上になり、重みが全て 0 以下なら最大合計は 0、復元結果は空列です。

`weighted_lis_values(a,weights)[i]` は位置 `i` を必ず最後に選びます。この値は負になる場合があり、位置 `i` までの列全体の最大合計とは異なります。例えば値 `[1,2,3]`・重み `[4,-10,3]` に対しては `[4,-6,7]` を返します。空入力では `weighted_lis_value` は 0、他の 2 関数は空配列を返します。

比較・要素の取得・重みの加算と比較を定数時間とすると、全操作の時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。長い文字列や多倍長数を使う場合は比較・演算などのコストが加わります。空入力は全操作で $$O(1)$$ 時間・追加空間です。前計算や共有表を呼び出し後に保持しません。
