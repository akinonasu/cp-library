---
title: "Fenwick Tree"
documentation_of: //data_structure/fenwick_tree.hpp
library_category: data_structure/range_query
---

```cpp
#include "lib/data_structure/fenwick_tree.hpp"
#include <cassert>
#include <vector>

int main() {
	FenwickTree<long long> tree(std::vector<long long>{1, 2, 3, 4});
	tree.add(1, 5);
	assert(tree.sum(1, 3) == 10);
	assert(tree.sum(3) == 11);
	tree.set(1, 2);
	assert(tree.get(1) == 2);
	assert(tree.lower_bound(3) == 1);
	assert(tree.upper_bound(3) == 2);
	assert(tree.lower_bound(11) == tree.size());
}
```

`FenwickTree<T,Padded = true>` として使います。添字は 0 始まり、区間は `[l,r)` です。`T{}` が加算の 0 を表し、加算・減算ができる型を指定します。加算の順序や括弧の位置を変えても同じ結果になり、加えた値を引けば元の値に戻る演算を使います。整数、modint などを指定できます。整数型は内部を符号なし整数にし、符号付きオーバーフローを避けます。取得する値・和は型のビット幅に応じた剰余です。`set` は変更前後の差が符号付きの型に収まらない場合も使えます。`bool` は指定できません。浮動小数点型は和の順序によって丸め誤差が変わります。

長さは非負の `int`、更新・取得位置は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。空列・空区間にも対応し、コピーは独立します。長さを $$n$$ とし、型の演算・比較・コピーは $$O(1)$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `FenwickTree<T>(n = 0)` | 長さ `n` の零列を構築 | $$O(n)$$ |
| `FenwickTree<T>(a)` | `const vector<T>&` の列で初期化 | $$O(n)$$ |
| `size()` | 長さを `int` で返す | $$O(1)$$ |
| `add(p,x)` | 要素 `p` に `x` を加算。返り値は `void` | $$O(\log n)$$ |
| `set(p,x)` | 要素 `p` に `x` を代入。返り値は `void` | $$O(\log n)$$ |
| `get(p)` | 要素 `p` の値を `T` で返す | $$O(\log n)$$ |
| `sum(r)` / `sum(l,r)` | `[0,r)`／`[l,r)` の和を `T` で返す。空区間は `T{}` | $$O(\log n)$$ |
| `lower_bound(x)` | `sum(p+1) >= x` を満たす最小の要素添字 `p` を `int` で返す | $$O(\log n)$$ |
| `upper_bound(x)` | `sum(p+1) > x` を満たす最小の要素添字 `p` を `int` で返す | $$O(\log n)$$ |

`lower_bound`・`upper_bound` は**全要素が非負**で、累積和が比較に使う型 `T` の範囲内に収まる場合に使います。`T` には通常の数値としての大小比較が必要で、加算・減算・比較を誤差なく行える型を使ってください。modint のように値の大小と加算が対応しない型には使いません。小さい順に並べた値の出現回数を保持すると、累積個数から要素の位置を検索できます。探索する際にも、要素に負の値を残さない範囲なら負の数の加算による個数の削除ができます。

探索で条件を満たす要素がなければ `n` を返します。`lower_bound(x)` は `x <= 0` のとき `0`、`upper_bound(x)` は `x < 0` のとき `0` を返します。空列はいずれの探索でも `0` を返します。要素が 0 の場合や重複する累積和にも対応します。`get`・`sum`・両探索・`size` は `const` オブジェクトでも使えます。

既定では内部配列の 1024 要素ごとに 1 要素分の余白を入れ、メモリアクセスが特定箇所に集中することによる速度低下を抑えます。`FenwickTree<T, false>` で無効にできます。効果は区間長・CPU・コンパイル条件に依存します。

構築・使用空間は $$O(n)$$、加算・代入・1 要素の取得・区間和・累積和からの検索は $$O(\log n)$$、`size()` は $$O(1)$$ です。構築後の各操作は追加空間 $$O(1)$$ で、空列に対する構築・和・探索は定数時間です。
