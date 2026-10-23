---
title: "Top K Sum"
documentation_of: //data_structure/top_k_sum.hpp
---

多重集合の大きい順または小さい順に選んだ `k` 個の和と、`k` 番目の値を求めます。**k は問い合わせごとに指定し、集合には保持しません。** 重複・負数・同じ値の個数を指定する更新に対応します。追加する値の先読みや座標圧縮は不要です。

```cpp
#include "lib/data_structure/top_k_sum.hpp"

TopKSum<> top{3, 1, 5, 5};
auto a = top.sum(2);        // 10
auto b = top.sum(1);        // 5: k を自由に変更できる
auto c = top.sum(4);        // 14
auto third = top.kth(2);    // 3: 大きい順に 0 始まり 2 番目
top.erase(5);              // 5 を 1 個削除
top.replace(1, 8);
auto d = top.sum(2);        // 13: 8 + 5

SmallestKSum<> bottom{3, 1, 5, 5};
auto e = bottom.sum(2);     // 4: 小さい順に 2 個
auto second = bottom.kth(1); // 3

top.insert(7, 1000000000000LL);
auto f = top.sum(100);      // 701: 8 + 7 * 99
auto removed = top.erase(7, 999999999999LL);
```

`sum(k)` は `k = 0` なら 0、`k > size()` なら全要素の和です。`kth(k)` は 0 始まりで、`0 <= k < size()` が必要です。重複もそれぞれ 1 個として数えます。どちらも `const` オブジェクトから呼べます。

`TopKSum<T,Sum,Compare>` の既定値は `T = std::int64_t`、`Sum = std::common_type_t<T,std::int64_t>`、`Compare = std::greater<T>` です。`LargestKSum<T,Sum>` は大きい順、`SmallestKSum<T,Sum>` は小さい順に選びます。

`Sum` は和の型です。例えば `TopKSum<int>` の和は `std::int64_t`、64 bit を超える和には `TopKSum<std::int64_t,__int128_t>` を使います。`Sum(0)`、値から `Sum` への変換、加算代入、非負の `size_type` との乗算が必要です。値の変換と問い合わせの結果が表現できる型を指定してください。64 bit 以下の整数の和は内部で広げるため、問い合わせない部分の和が `Sum` を超えても扱えます。それ以外の型では、部分和と個数倍も表現できることが必要です。浮動小数点では丸め誤差が生じます。

`Compare` は狭義弱順序を満たし、先に並べる要素から選びます。比較上同値な値は個数をまとめて保持するため、`Sum` へ変換しても同じ値になる必要があります。

| 操作 | 内容・返り値 |
| --- | --- |
| `TopKSum()` / `TopKSum(compare)` | 空集合を作成 |
| `TopKSum(values)` / `TopKSum(values,compare)` | `vector<T>` または初期化子リストから構築 |
| `sum(k)` | 比較順で先頭から `min(k,size())` 個の和を `Sum` で返す |
| `sum_all()` | 全要素の和を `Sum` で返す |
| `kth(k)` | 比較順で 0 始まり `k` 番目の値を `T` で返す |
| `insert(value,copies = 1)` | `value` を `copies` 個追加 |
| `erase(value)` | 1 個削除。存在すれば `true`、なければ変更せず `false` |
| `erase(value,copies)` | 最大 `copies` 個削除し、削除した個数を返す |
| `erase_all(value)` | `value` をすべて削除し、削除した個数を返す |
| `replace(old,new)` | `old` を 1 個 `new` に置換。存在すれば `true`、なければ変更せず `false` |
| `size()` / `empty()` | 重複を含む要素数／空判定 |
| `count(value)` / `contains(value)` | 出現個数／存在判定 |
| `assign(values)` | 全要素を置き換える。比較関数は維持 |
| `reserve(distinct)` | 異なる値の個数を指定して領域を確保 |
| `clear()` | 全要素を削除。比較関数と確保済み容量は維持 |
| コピー／代入 | 独立した集合として複製 |
| 移動／移動代入 | 集合を移動。移動元は空になり、そのまま再利用可能 |
| `swap(t)` / `swap(s,t)` | 集合と比較関数を交換 |

`size_type` は `std::int64_t` です。k と更新個数は非負とし、全要素数がその最大値以下になるように使います。0 個の更新は何もしません。削除する個数が足りない場合は存在する分だけ削除します。

旧 `TopKSum(k,values)` は `TopKSum(values)` に、`set_k(k); sum()` は `sum(k)` に置き換えてください。`set_k`・`k()`・`selected_size()` と、引数なしの `sum()` は廃止しました。

異なる値の個数を $$d$$ として、`sum(k)`・`kth(k)`・削除・置換・存在判定・個数取得は $$O(\log d)$$ 時間、挿入は償却 $$O(\log d)$$ 時間です。k の変化幅や、一度に追加・削除する個数には依存しません。`sum_all`・`size`・`empty` と 0 個の更新は $$O(1)$$、異なる値が 1 個以下なら各操作は定数時間です。

入力要素数を $$m$$ として、構築は $$O(m\log m)$$ 時間・$$O(m)$$ 追加空間です。確保したノード数を $$D$$ として、`assign` は $$O(D+m\log m)$$、コピーと `clear` は $$O(D)$$、移動構築と交換は $$O(1)$$ 時間です。コピー・移動代入では移動先の旧ノード数も加わります。使用空間は $$O(D)$$ で、削除したノードの領域を再利用します。値・比較・和の演算を定数時間とした計算量です。
