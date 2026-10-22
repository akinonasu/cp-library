---
title: "Top K Sum"
documentation_of: //data_structure/top_k_sum.hpp
---

```cpp
#include "lib/data_structure/top_k_sum.hpp"

TopKSum<> top(2, {3, 1, 5, 5});
auto a = top.sum();           // 10: 大きい順に2個
top.erase(5);                 // 5を1個だけ削除
bool found = top.replace(1, 8);
auto b = top.sum();           // 13: 8 + 5
top.set_k(3);
auto c = top.sum();           // 16: 8 + 5 + 3

SmallestKSum<> bottom(2, {3, 1, 5, 5});
auto d = bottom.sum();        // 4: 小さい順に2個
```

多重集合のうち、大きい順または小さい順に選んだ `k` 個の和を管理します。重複と負数を扱えます。`k = 0` なら和は 0、`k > size()` なら全要素の和です。空集合の和も 0 です。`k` は非負の要素数で、実際に選ぶ個数は `min(k,size())` です。

`TopKSum<T, Sum, Compare>` の既定値は `T = std::int64_t`、`Sum = std::common_type_t<T,std::int64_t>`、`Compare = std::greater<T>` です。`TopKSum<>` と `LargestKSum<>` は大きい順、`SmallestKSum<>` は小さい順に選びます。両方の別名で `T` と `Sum` を指定できます。

`T` は値の型、`Sum` は和の型です。例えば `TopKSum<int>` の和は `std::int64_t` で計算します。64 bit を超える和には `TopKSum<long long,__int128_t>` のように広い型を指定してください。値から `Sum` への変換と、途中の加減算を含む和が正しく表せることが必要です。浮動小数点では丸め誤差が生じます。

`Compare` が先に並べる要素から選びます。独自の比較関数は狭義弱順序を満たす必要があります。比較上同値の値は同じ要素として検索し、同順位のどの要素を選ぶかは保証しません。

| 操作 | 内容・返り値 |
| --- | --- |
| `TopKSum(k = 0)` | 選ぶ個数を指定して空集合を作成 |
| `TopKSum(k,values)` | `vector<T>` または初期化子リストから構築 |
| `TopKSum(k,compare)` / `TopKSum(k,values,compare)` | 比較関数の状態も指定 |
| `insert(value)` | 1個追加 |
| `erase(value)` | 1個削除。存在すれば `true`、なければ変更せず `false` |
| `replace(old,new)` | `old` の1個を `new` に置換。存在すれば `true`、なければ変更せず `false` |
| `set_k(k)` / `k()` | 選ぶ個数を変更／取得。要素数を超えていても指定値を保持 |
| `sum()` | 選んだ要素の和を `Sum` で返す |
| `size()` / `selected_size()` | 全要素数／実際に選んでいる個数 |
| `empty()` / `contains(value)` / `count(value)` | 空判定／存在判定／出現個数 |
| `assign(values)` | 値を一括で置き換える。`k` と比較関数は維持 |
| `clear()` | 全要素を削除。`k` と比較関数は維持 |
| コピー／代入 | 独立した集合として複製 |
| 移動／移動代入 | 移動元は空集合になる。移動元の `k` は維持 |
| `swap(t)` / `swap(s,t)` | 集合・`k`・比較関数を交換 |

点更新では元の値を配列に保持し、`replace(a[i],x)` の後に `a[i] = x` とします。スライド区間では `replace(出る値,入る値)` とすると、要素数を変えずに更新できます。削除対象の出現回数を利用者が別途管理する必要はありません。

選ぶ要素と残りの要素を2本の平衡二分木で保持します。境界をまたぐ移動と `replace` はノードを再利用し、新しいノードを確保しません。削除済みの要素を後で処理する遅延削除は使わないため、保持するノード数は現在の要素数に比例します。

[手法と速度比較](https://github.com/qandori/cp-library/blob/main/test/benchmark/top_k_sum.md)を参照してください。

要素数を $$n$$、一括構築する要素数を $$m$$、`set_k` 前後で実際に選ぶ個数の差を $$d$$、`count` の返す個数を $$c$$ とします。`insert`・`erase`・`replace`・`contains` は $$O(\log n)$$、`count` は $$O(\log n+c)$$ 時間です。`set_k` は $$O(d+1)$$ 償却時間です。境界での移動には挿入位置のヒントとノードの抜き取りを使います。

構築は $$O(m\log m)$$、`assign` は旧要素数も含めて $$O(n+m\log m)$$、コピー構築はコピー元の要素数に比例し、コピー代入では移動先の旧要素数も加わります。移動構築と交換は $$O(1)$$、移動代入は移動先の旧要素数に比例します。`clear` は $$O(n)$$、それ以外の取得操作は $$O(1)$$ 時間です。空間は $$O(n)$$、一括構築・`assign` 中は入力を含めて $$O(n+m)$$ です。値・比較・和の演算を定数時間とした計算量です。
