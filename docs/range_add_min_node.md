---
title: 区間加算・最小値のノード演算
documentation_of: //components/range_add_min_node.hpp
---

[区間加算・区間最小値](range_add_range_min.md)の 8 子ノードを処理する内部部品です。通常は `data_structure/range_add_range_min.hpp` を include します。

`range_add_min_detail::Node<T>` の `T` は 64 bit 以下の標準の符号付き整数型、`U` は同じ幅の符号なし型です。配列にはノードの最小値からの差分を `U` で格納します。正規化済みの有効要素の最小値は 0 です。差分は `U` の全範囲を取り得るため、通常の経路は unsigned の順序を使います。

`values` は読み書き可能な連続 8 要素の `U` 配列です。有効要素は先頭 `count` 個で、`1 <= count <= 8` とします。64 bit の SIMD 経路では先頭アドレスを 32 バイト境界に揃えます。その他の経路でも 8 要素分の領域を用意します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `minimum<Narrow>(values,l,r)` | `const U*` と `unsigned l,r` を渡し、`0 <= l < r <= count` の最小差分を `U` で返す。空区間は指定しない | $$O(1)$$ |
| `normalize<Narrow>(values,count,negative,first,last,delta,x=0,dx=0,y=0,dy=0)` | `U*` の `[first,last)` に `U delta`、添字 `x,y` に `U dx,dy` を加え、有効要素を再び最小差分 0 に揃える。元の最小値に加える変化量を `U` で返す | $$O(1)$$ |
| `change_one<Narrow>(values,count,index,delta,negative)` | `U*` の添字 `index` に `U delta` を加えて正規化する。元の最小値に加える変化量を `U` で返す。`delta = 0` なら何もしない | $$O(1)$$ |
| `full_minimum<Narrow>(values)` | `const U*` の 8 要素すべての最小差分を `U` で返す。パディングも参照する | $$O(1)$$ |
| `subtract(values,count,change)` | `U*` の先頭 `count` 要素から `U change` を引く。パディングは変更しない。返り値は `void` | $$O(1)$$ |
| `min_signed(x,y)` | AVX2 用の補助関数。`__m256i` の各 signed 64 bit レーンの最小値を `__m256i` で返す | $$O(1)$$ |
| `reduce_min(x)` | AVX2 用の補助関数。`__m256i` の 4 レーンを signed 64 bit として比較し、最小値のビット表現を `U` で返す | $$O(1)$$ |

`Narrow` は省略できない `bool` のテンプレート引数です。添字と `count,first,last,x,y,index` は `unsigned`、`negative` は `bool` です。`0 <= first <= last <= count`、`0 <= index < count` とします。`x,y` は `dx,dy` が 0 でも常に 8 未満にし、非ゼロの差分を渡す添字は `count` 未満にします。`normalize` の空の `[first,last)` は指定でき、`dx,dy` のみを適用できます。その他の引数に既定値はありません。

`min_signed`・`reduce_min` は AVX2 有効時の 64 bit 整数用補助関数です。

差分への加減算は `U` の巡回演算で行います。負の変化量も `U` に変換して渡し、実際の増減方向を `negative` で別に指定します。`negative` は非ゼロの各変化が減少なら `true`、増加なら `false` です。差分の最上位ビットから増減方向を決めないでください。呼び出し元は、各子が表す更新後の値が `T` の範囲に収まることを保証します。更新前の有効要素は正規化しておき、返された変化量を親ノードの最小値に加えます。

`Narrow=false` では、パディングを `std::numeric_limits<U>::max()` にします。負更新で差分が wrap する場合も扱い、`U` 全域の差分を比較します。

`Narrow=true` は 64 bit 整数の内部用経路です。`M = std::numeric_limits<T>::max()`、初期列の最大値と最小値の差を $$S_0$$、全区間加算を除く更新の絶対変化量累計を $$A$$ とし、$$\displaystyle L=\left\lfloor\frac{M}{3}\right\rfloor$$ とします。一点代入では新旧の値の差の絶対値を累計に加えます。呼び出し元が $$S_0+A<L$$ を保証し、構築後のパディングを $$2L$$ にします。SIMD の `normalize<true>` はパディングも含めて signed の最小値を計算し、8 レーンすべてからその値を引きます。

有効要素の正規化後の差分は $$[0,S_0+A]$$、パディングの差分は $$[2L-A,2L+A]$$ に収まります。パディングは常に有効要素より大きく、signed 64 bit の範囲内です。範囲条件を満たさなくなる更新の前に、各段の末尾のパディングを `std::numeric_limits<U>::max()` に戻し、通常の経路に切り替えます。この管理は `RangeAddRangeMin` が行い、利用者の値域制限は増えません。

64 bit 整数の x86_64 ビルドで AVX-512 が有効なら AVX-512、次に AVX2 が有効なら AVX2 を使い、それ以外はスカラー処理です。AVX2 の通常経路は比較時だけ符号ビットを反転し、保存する差分の形式は変えません。

子の数は 8 に固定されているため、各操作は $$O(1)$$ 時間・追加空間です。
