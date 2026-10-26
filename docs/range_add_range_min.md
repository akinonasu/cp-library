---
title: 区間加算・区間最小値
documentation_of: //data_structure/range_add_range_min.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/range_add_range_min.hpp.html
- /lightweight/data_structure/segment_tree/range_add_range_min.hpp.html
---

整数列の区間加算・区間最小値・一点代入に対応します。`T` の既定値は `std::int64_t`、`value_type` は `T` です。

```cpp
#include "lib/data_structure/range_add_range_min.hpp"
#include <iostream>

int main() {
	RangeAddRangeMin<> values(std::vector<std::int64_t>{3, 1, 5});
	values.add(0, 2, 4);
	std::cout << values.prod(0, 3) << '\n';
	values.set(2, -1);
	std::cout << values.all_min() << '\n';
}
```

出力は次のとおりです。

```text
5
-1
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RangeAddRangeMin<T>()` | 空の `RangeAddRangeMin<T>` を構築 | $$O(1)$$ |
| `RangeAddRangeMin<T>(values)` | `const std::vector<T>&` の各値から構築。入力は変更しない | $$O(n)$$ |
| `RangeAddRangeMin<T>(n,value=0)` | `int n` 個の要素を `T value` で初期化 | $$O(n)$$ |
| コピー構築／コピー代入 | 独立した内容を複製。代入の返り値は `RangeAddRangeMin&` | $$O(n)$$ |
| 移動構築／移動代入 | 内容を移し、移動元を空にする。代入の返り値は `RangeAddRangeMin&` | $$O(1)$$ |
| `build(values)` | `const std::vector<T>&` から作り直す。返り値は `void` | $$O(n)$$ |
| `size()` / `empty()` | 要素数を `int`、空判定を `bool` で返す | $$O(1)$$ |
| `add(l,r,value)` / `apply(l,r,value)` | `int l,r` の半開区間 `[l,r)` に `T value` を加算。返り値は `void` | $$O(\log n)$$ |
| `prod(l,r)` / `min(l,r)` | `int l,r` の半開区間 `[l,r)` の最小値を `T` で返す | $$O(\log n)$$ |
| `all_prod()` / `all_min()` | 全要素の最小値を `T` で返す | $$O(1)$$ |
| `get(p)` | `int p` の値を `T` で返す | $$O(\log n)$$ |
| `set(p,value)` | `int p` に `T value` を代入。返り値は `void` | $$O(\log n)$$ |
| `values()` | 現在の全要素を添字順の `std::vector<T>` で返す | $$O(n)$$ |
| `memory_usage()` | 内部配列が確保したバイト数を `std::size_t` で返す | $$O(1)$$ |
| `swap(other)` / `swap(a,b)` | 同じ型の構造同士の内容を交換。返り値は `void` | $$O(1)$$ |

`T` は 64 bit 以下の標準の符号付き整数型です。要素・代入値・加算量と、各更新後のすべての要素が `T` の範囲に収まるように使ってください。負の値・負の加算・型の最小値と最大値に対応し、内部の高速経路のために値域を狭める必要はありません。要素の最大値と最小値の差が `T` の範囲を超えていても扱えます。初期値の重複もそのまま保持します。

これらの値が signed 32 bit に収まる場合は、`RangeAddRangeMin<std::int32_t>` を指定できます。ノード内の値を 32 bit で保持し、AVX2 では 8 要素をまとめて処理します。この場合も、要素の最大値と最小値の差を signed 32 bit に収める必要はありません。

区間は `0 <= l <= r <= size()`、位置は `0 <= p < size()`、長さは `0 <= n <= INT_MAX` です。空区間への加算は何もせず、空区間・空の構造の最小値は `std::numeric_limits<T>::max()` を返します。この値を通常の要素として格納することもできます。`get`・`set` には存在する位置を指定します。空の初期配列・`build` にも対応します。

取得操作は `const` オブジェクトから呼べ、内部状態を変更しません。コピーは独立した構造を作り、移動元は再利用できる空の構造になります。

内部は 8 分木です。x86_64 では、32 bit 整数の構築・区間操作に AVX2、64 bit 整数には AVX-512 または AVX2 を利用します。64 bit では AVX-512 が有効ならそちらを優先します。8 bit・16 bit 整数の構築・区間操作はスカラー処理です。

`values()` では葉の値の復元もまとめて処理します。x86_64 の AVX2 有効時は 8 bit・16 bit 整数でも SIMD を使い、64 bit では AVX-512 も利用します。対応する命令が無効な環境ではスカラー処理で同じ結果を返します。命令の選択はコンパイル時で、実行時の CPU 判定は行いません。ノード内の処理は[共通部品](range_add_min_node.md)を使います。

参考: [Library Checker の問題](https://judge.yosupo.jp/problem/range_add_range_min)、[参考実装](https://judge.yosupo.jp/submission/409052)。

構築・使用空間・`values` は $$O(n)$$、更新・区間最小値・一点の取得や代入は最悪 $$O(\log n)$$ 時間です。全区間への加算・全区間の最小値取得は $$O(1)$$ です。保持している全体最小の位置を含むと確認できる区間も $$O(1)$$ で取得します。全要素を同じ値で構築し、以降の更新を全区間への加算に限る場合、どの非空区間の最小値も型によらず $$O(1)$$ で取得します。
