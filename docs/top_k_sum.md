---
title: "Top K Sum"
documentation_of: //data_structure/top_k_sum.hpp
---

多重集合のうち、大きい順または小さい順に選んだ `k` 個の和と、`k` 番目の値を求めます。**`sum(k)` のたびに異なる `k` を指定できます。** K を大きく変えても、各問い合わせは対数時間です。重複と負数も扱えます。

```cpp
#include "lib/data_structure/top_k_sum.hpp"

TopKSum<> top(std::vector<std::int64_t>{3, 1, 5, 5});
auto a = top.sum(2);          // 10: 大きい順に2個
auto b = top.sum(1);          // 5: 次の問い合わせでは1個
auto c = top.sum(4);          // 14: 次は4個
auto largest = top.kth(0);   // 5: 最大値
auto third = top.kth(2);     // 3: 大きい順に3番目
top.erase(5);                // 5を1個だけ削除
top.replace(1, 8);
auto d = top.sum(2);          // 13: 8 + 5

SmallestKSum<> bottom(std::vector<std::int64_t>{3, 1, 5, 5});
auto e = bottom.sum(2);       // 4: 小さい順に2個
auto second = bottom.kth(1); // 3: 小さい順に2番目

top.insert(7, 4);            // 7を4個追加
auto removed = top.erase(7, 3); // 3個削除。戻り値は3
```

`k = 0` なら和は 0、`k > size()` なら全要素の和です。空集合の和も 0 です。`k` は非負の要素数で、実際に選ぶ個数は `min(k,size())` です。

`kth(k)` の順位は **0 始まり**で、`0 <= k < size()` が必要です。重複も別々の要素として数えます。`TopKSum<>`・`LargestKSum<>` は大きい順、`SmallestKSum<>` は小さい順です。`const` オブジェクトでも取得でき、値を `T` として返します。集合も既定の K も変更しません。

`TopKSum<T, Sum, Compare>` の既定値は `T = std::int64_t`、`Sum = std::common_type_t<T,std::int64_t>`、`Compare = std::greater<T>` です。`TopKSum<>` と `LargestKSum<>` は大きい順、`SmallestKSum<>` は小さい順に選びます。両方の別名で `T` と `Sum` を指定できます。

`T` は値の型、`Sum` は和の型です。例えば `TopKSum<int>` の和は `std::int64_t` で返します。64 bit を超える和には `TopKSum<long long,__int128_t>` のように広い型を指定してください。値から `Sum` への変換と、取得する和が正しく表せることが必要です。64 bit 以下の整数型では、取得しない部分の和が `Sum` を超えても扱えます。`__int128_t` や独自の和の型では、部分和を含む加算も表現できることが必要です。浮動小数点では丸め誤差が生じます。

`Compare` が先に並べる要素から選びます。独自の比較関数は狭義弱順序を満たす必要があります。比較上同値の値は同じ要素として検索し、同順位のどの要素を選ぶかは保証しません。

| 操作 | 内容・返り値 |
| --- | --- |
| `TopKSum(values)` / `TopKSum(values,compare)` | `vector<T>` から構築。既定の K は 0 |
| `TopKSum(k = 0)` | 既定の K を指定して空集合を作成 |
| `TopKSum(k,values)` | `vector<T>` または初期化子リストから構築 |
| `TopKSum(k,compare)` / `TopKSum(k,values,compare)` | 比較関数の状態も指定 |
| `sum(k)` | 問い合わせごとに指定した K 個の和。既定の K は変更しない。`const` オブジェクトでも使用可能 |
| `kth(k)` | 比較順で 0 始まり `k` 番目の値を `T` で返す。`0 <= k < size()` が必要 |
| `insert(value)` | 1個追加 |
| `insert(value,count)` | 同じ値を `count` 個追加 |
| `erase(value)` | 1個削除。存在すれば `true`、なければ変更せず `false` |
| `erase(value,count)` | 同じ値を最大 `count` 個削除し、実際に削除した個数を `size_type` で返す |
| `replace(old,new)` | `old` の1個を `new` に置換。存在すれば `true`、なければ変更せず `false` |
| `set_k(k)` / `k()` | 引数なしの `sum()` で使う既定の K を変更／取得 |
| `sum()` | 既定の K 個の和を `Sum` で返す |
| `size()` / `selected_size()` | 全要素数／既定の K で選ぶ個数 |
| `empty()` / `contains(value)` / `count(value)` | 空判定／存在判定／出現個数 |
| `assign(values)` | 値を一括で置き換える。既定の K と比較関数は維持 |
| `clear()` | 全要素を削除。既定の K と比較関数は維持 |
| コピー／代入 | 独立した集合として複製 |
| 移動／移動代入 | 移動元は空集合になる。移動元の K は維持 |
| `swap(t)` / `swap(s,t)` | 集合・既定の K・比較関数を交換 |

従来の `TopKSum<>(k,values)`・`set_k(k)`・`sum()` もそのまま使えます。`sum(k)` は集合も既定の K も変更しません。既定の K は要素数を超えていても指定値を保持します。

点更新では元の値を配列に保持し、`replace(a[i],x)` の後に `a[i] = x` とします。スライド区間では `replace(出る値,入る値)` とすると、要素数を変えずに更新できます。

まとめて更新する `count` は非負の個数です。0 なら変更しません。削除は存在する分だけ行い、不足していても残りの集合を正しく保ちます。`size_type` は `std::size_t` です。`replace` は既存のノードを再利用します。削除済みの要素は保持しません。

操作前後の最大要素数を $$n$$、一括構築する要素数を $$m$$ とします。`sum(k)`・`kth(k)`・`set_k`・`insert(value)`・`erase(value)`・`replace`・`contains`・`count` は $$O(\log n)$$ 時間です。K の変化幅には依存しません。引数なしの `sum()`、`k()`・`size()`・`selected_size()`・`empty()` は $$O(1)$$ 時間です。空集合・要素 1 個の操作は定数時間です。

まとめて追加する個数を $$a$$、実際に削除する個数を $$e$$ として、`insert(value,a)` は $$O(a+\log n)$$、`erase(value,a)` は $$O(e+\log n)$$ 時間です。0 個の更新は $$O(1)$$ です。個数を圧縮する構造ではないため、追加した個数だけノードを保持します。

構築は $$O(m\log m)$$、`assign` は旧要素数も含めて $$O(n+m\log m)$$、コピー構築はコピー元の要素数に比例し、コピー代入では移動先の旧要素数も加わります。移動構築と交換は $$O(1)$$、移動代入は移動先の旧要素数に比例します。`clear` は $$O(n)$$、空間は $$O(n)$$ です。一括構築・`assign` 中は入力を含めて $$O(n+m)$$ を使います。値・比較・和の演算を定数時間とした計算量です。
