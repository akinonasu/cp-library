---
title: "Top K Sum"
documentation_of: //data_structure/top_k_sum.hpp
library_category: data_structure/associative
---

値の追加・削除ができる多重集合です。小さい順／大きい順の `k` 番目の値、下位／上位 `k` 個の和、値を指定した境界検索、指定した値以上・以下や値の区間に含まれる要素の個数・和に対応します。重複する値は個数分だけ数えます。

```cpp
#include "lib/data_structure/top_k_sum.hpp"

TopKSum<> values{3, 1, 5, 5};
auto a = values.kth_smallest(0); // 1
auto b = values.kth_largest(1);  // 5
auto c = values.sum_smallest(2); // 4: 1 + 3
auto d = values.sum_largest(2);  // 10: 5 + 5
auto e = values.lower_bound(4); // optional(5)
auto f = values.upper_bound(5); // nullopt
auto g = values.count_geq(3);   // 3: 3, 5, 5
auto h = values.sum_leq(3);     // 4: 1 + 3
auto i = values.range_freq(3, 6); // 3: 3, 5, 5
auto j = values.range_sum(3, 6);  // 13: 3 + 5 + 5
auto [total, count] = values.range_sum_with_count(3, 6); // 13, 3

values.insert(7, 3);            // 7 を 3 個追加
values.erase(5);                // 5 を 1 個削除
values.replace(1, 8);           // 1 を 1 個 8 に置換
```

`kth_smallest(k)`・`kth_largest(k)` は 0 始まりで、`0 <= k < size()` が必要です。`sum_smallest(k)`・`sum_largest(k)` は `k >= 0` とし、`k = 0` なら 0、`k > size()` なら全要素の和を返します。問い合わせはすべて `const` オブジェクトから呼べます。

`lower_bound(x)` は `x` 以上の最小値、`upper_bound(x)` は `x` より大きい最小値を `std::optional<T>` で返します。該当する値がなければ `std::nullopt` です。標準の `std::less`・`std::greater` ではどちらも値の昇順で検索し、独自の比較関数ではその比較順で検索します。

`count_less(x)`・`sum_less(x)` は `x` 未満、`count_leq(x)`・`sum_leq(x)` は `x` 以下、`count_greater(x)`・`sum_greater(x)` は `x` より大きい、`count_geq(x)`・`sum_geq(x)` は `x` 以上の要素を対象にします。個数の返り値は `size_type`、和の返り値は `Sum` です。該当する要素がなければ 0 を返します。

`range_freq(lower,upper)`・`range_sum(lower,upper)` は `lower` 以上 `upper` 未満の個数・和を返します。`lower` が `upper` 以上なら、ともに 0 です。`range_sum_with_count(upper)` は `upper` 未満、`range_sum_with_count(lower,upper)` は同じ半開区間を対象とし、`std::pair<Sum,size_type>` で `{和, 個数}` を返します。空区間では `{Sum(0), 0}` です。

これらの大小は、標準の `std::less<T>`・`std::less<>`・`std::greater<T>`・`std::greater<>` では値の大小です。独自の `Compare` では、その比較順を小さい順として扱います。比較上同値な要素は、未満・超には含めず、以下・以上には含めます。

## API

| 操作 | 内容・返り値 |
| --- | --- |
| `TopKSum()` / `TopKSum(compare)` | 空集合を作成 |
| `TopKSum(values)` / `TopKSum(values,compare)` | `vector<T>` または初期化子リストから構築 |
| `kth_smallest(k)` / `kth_largest(k)` | 小さい順／大きい順の 0 始まり `k` 番目の値を `T` で返す |
| `sum_smallest(k)` / `sum_largest(k)` | 下位／上位 `min(k,size())` 個の和を `Sum` で返す |
| `order_of_key(x)` | `Compare` 上で `x` より前にある要素数。重複を含む |
| `lower_bound(x)` / `upper_bound(x)` | `x` 以上／`x` より大きい最初の値。なければ `nullopt` |
| `count_less(x)` / `sum_less(x)` | `x` 未満の要素数を `size_type`／和を `Sum` で返す |
| `count_leq(x)` / `sum_leq(x)` | `x` 以下の要素数を `size_type`／和を `Sum` で返す |
| `count_greater(x)` / `sum_greater(x)` | `x` より大きい要素数を `size_type`／和を `Sum` で返す |
| `count_geq(x)` / `sum_geq(x)` | `x` 以上の要素数を `size_type`／和を `Sum` で返す |
| `range_freq(lower,upper)` / `range_sum(lower,upper)` | `lower` 以上 `upper` 未満の要素数を `size_type`／和を `Sum` で返す |
| `range_sum_with_count(upper)` / `range_sum_with_count(lower,upper)` | `upper` 未満／`lower` 以上 `upper` 未満の `{和, 個数}` を `std::pair<Sum,size_type>` で返す |
| `sum_all()` | 全要素の和を `Sum` で返す |
| `insert(value,copies = 1)` | `value` を `copies` 個追加 |
| `erase(value)` | 1 個削除。存在すれば `true`、なければ `false` |
| `erase(value,copies)` | 最大 `copies` 個削除し、削除した個数を返す |
| `erase_all(value)` | `value` をすべて削除し、削除した個数を返す |
| `replace(old,new)` | `old` を 1 個 `new` に置換。存在すれば `true`、なければ `false` |
| `size()` / `empty()` | 重複を含む要素数／空判定 |
| `count(value)` / `contains(value)` | 出現個数／存在判定 |
| `kth(k)` / `sum(k)` | `Compare` 順の `k` 番目／先頭 `min(k,size())` 個の和 |
| `assign(values)` | 全要素を置き換える |
| `reserve(distinct)` | 呼び出しは不要。`distinct <= INT_MAX` を確認し、領域の事前確保は行わない |
| `clear()` | 全要素を削除 |
| コピー／代入 | 独立した集合として複製 |
| 移動／移動代入 | 集合を移動。移動元は空になり、再利用可能 |
| `swap(t)` / `swap(s,t)` | 集合と比較関数を交換 |

## 型と範囲

`TopKSum<T,Sum,Compare>` の既定値は `T = std::int64_t`、`Sum = std::common_type_t<T,std::int64_t>`、`Compare = std::greater<T>` です。`LargestKSum<T,Sum>` は `std::greater<T>`、`SmallestKSum<T,Sum>` は `std::less<T>` を使います。どちらの型でも、大小両方の順位・和・境界検索と、値の範囲を指定した個数・和を利用できます。独自の `Compare` を指定した場合は、その比較順を小さい順として扱います。

`order_of_key(x)` は `std::less` なら `x` 未満、`std::greater` なら `x` より大きい要素数を返します。

`Compare(a,b)` は `a` が `b` より前に来るかを返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。比較上同値な値は個数をまとめて扱うため、`Sum` へ変換しても同じ値になる必要があります。`assign`・`clear` は比較関数を維持します。

`Sum` は和の型です。例えば `TopKSum<int>` の和は `std::int64_t`、64 bit を超える和には `TopKSum<std::int64_t,__int128_t>` を使います。`Sum(0)`、値から `Sum` への変換、加算代入、非負の `size_type` との乗算が必要で、減算は不要です。値の変換と和の問い合わせの結果が表現できる型を指定してください。64 bit 以下の整数型では、個数だけを求める操作や選ばない部分について、和が `Sum` の範囲を超えていても扱えます。それ以外の型では、部分和と個数倍も表現できることが必要です。浮動小数点では丸め誤差が生じます。

`size_type` は `std::int64_t` です。更新個数は非負とし、全要素数がその最大値以下になるように使います。0 個の更新は何もしません。削除する個数が足りない場合は存在する分だけ削除します。

## 計算量

異なる値の個数を $$d$$ として、順位・和・境界検索・挿入・削除・置換・存在判定・個数取得は最悪 $$O(\log d)$$ 時間です。値の上下限や区間を指定する個数・和と `range_sum_with_count` も最悪 $$O(\log d)$$ 時間・$$O(1)$$ 追加空間です。`sum_all`・`size`・`empty`・`reserve` と 0 個の更新は $$O(1)$$、異なる値が 1 個以下なら各操作は定数時間です。

入力要素数を $$m$$ として、構築は $$O(m\log m)$$ 時間・$$O(m)$$ 追加空間です。`T` が `bool` 以外の 64 bit 以下の整数型で、比較が標準の `std::less`・`std::greater` の場合は、構築時間が $$O(m)$$ です。`assign` は構築時間に旧集合の $$O(d)$$ が加わります。コピーと `clear` は $$O(d)$$、移動構築と交換は $$O(1)$$ 時間です。コピー・移動代入では移動先の旧ノード数も加わります。使用空間は $$O(d)$$ で、異なる値を削除すると、その領域を解放します。値・比較・和の演算とノードの確保・解放を定数時間とした計算量です。
