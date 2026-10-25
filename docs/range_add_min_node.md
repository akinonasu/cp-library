---
title: 区間加算・最小値のノード演算
documentation_of: //components/range_add_min_node.hpp
---

[区間加算・区間最小値](range_add_range_min.md)の 8 子ノードを処理する内部部品です。通常は `data_structure/range_add_range_min.hpp` を include します。

`range_add_min_detail::Node<T>` の `T` は 64 bit 以下の標準の符号付き整数型、`U` は同じ幅の符号なし型です。配列にはノードの最小値からの差分を `U` で格納します。正規化済みの有効要素の最小値は 0 です。差分は `U` の全範囲を取り得るため、通常の経路は unsigned の順序を使います。

`values` は連続 8 要素の `U` 配列です。有効要素は先頭 `count` 個で、`1 <= count <= 8` とします。更新には読み書き可能な配列を渡します。64 bit の SIMD 経路では先頭アドレスを 32 バイト境界に揃えます。その他の経路でも 8 要素分の領域を用意します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `minimum<Narrow>(values,l,r)` | `const U*` と `unsigned l,r` を渡し、`0 <= l <= r <= count` の最小差分を `U` で返す。空区間の返り値は本文参照 | $$O(1)$$ |
| `normalize<Narrow>(values,count,negative,first,last,delta,x=0,dx=0,y=0,dy=0)` | `U*` の `[first,last)` に `U delta`、添字 `x,y` に `U dx,dy` を加え、有効要素を再び最小差分 0 に揃える。元の最小値に加える変化量を `U` で返す | $$O(1)$$ |
| `normalize_boundaries<Narrow>(left,right,right_count,negative,first,last,delta,left_change,right_change)` | 左の添字 `first` に `U left_change`、`[first+1,8)` に `U delta` を加える。右の `[0,last)` に `delta`、添字 `last` に `U right_change` を加える。両ブロックを正規化し、左右の最小値の変化量を `std::pair<U,U>` で返す | $$O(1)$$ |
| `change_one<Narrow>(values,count,index,delta,negative)` | `U*` の添字 `index` に `U delta` を加えて正規化する。元の最小値に加える変化量を `U` で返す。`delta = 0` なら何もしない | $$O(1)$$ |
| `full_minimum<Narrow>(values)` | `const U*` の 8 要素すべての最小差分を `U` で返す。パディングも参照する | $$O(1)$$ |
| `subtract(values,count,change)` | `U*` の先頭 `count` 要素から `U change` を引く。パディングは変更しない。返り値は `void` | $$O(1)$$ |
| `min_signed(x,y)` | AVX2 用の補助関数。`__m256i` の各 signed 64 bit レーンの最小値を `__m256i` で返す | $$O(1)$$ |
| `broadcast_min(value)` | AVX2 用の補助関数。`__m256i` の 4 レーンの signed 64 bit 最小値を、全レーンに複製した `__m256i` で返す | $$O(1)$$ |
| `reduce_min(x)` | AVX2 用の補助関数。`__m256i` の 4 レーンを signed 64 bit として比較し、最小値のビット表現を `U` で返す | $$O(1)$$ |
| `minimum_vector<Narrow>(values,l,r,neutral)` | AVX2 用の補助関数。`const U*` の `[l,r)` 以外を `U neutral` で置き換え、前半と後半の対応レーンを比較した 4 レーンの `__m256i` を返す。変換と中立値の条件は本文参照 | $$O(1)$$ |

`Narrow` は省略できない `bool` のテンプレート引数です。添字と `count,right_count,first,last,x,y,index` は `unsigned`、`negative` は `bool` です。`normalize` は `0 <= first <= last <= count`、`change_one` は `0 <= index < count` とします。`x,y` は `dx,dy` が 0 でも常に 8 未満にし、非ゼロの差分を渡す添字は `count` 未満にします。`normalize` では範囲と一点の重複や `x == y` も指定できます。重なる位置には変化量を合計して加え、更新後の実際の値と `Narrow` の条件を満たすようにします。空の `[first,last)` も指定でき、`dx,dy` のみを適用できます。表に示したもの以外に既定値はありません。

`normalize_boundaries` の `left,right` は重ならない別の 8 要素ブロックを指す `U*` です。左の有効要素数は 8、右は `1 <= right_count <= 8` とし、`0 <= first < 8`、`0 <= last < right_count` を満たします。左右をそれぞれ正規化しておき、返された変化量をそれぞれの親へ反映します。左右の境界位置には共通の `delta` を重ねず、`left_change,right_change` だけを加えます。

差分への加減算は `U` の巡回演算で行います。負の変化量も `U` に変換して渡し、実際の増減方向を `negative` で別に指定します。1 回の呼び出しで渡す非ゼロの変化量はすべて同じ増減方向にし、減少なら `negative=true`、増加なら `false` とします。`normalize_boundaries` の左右の変化量にも同じ条件が必要です。差分の最上位ビットから増減方向を決めないでください。呼び出し元は、各子が表す更新後の値が `T` の範囲に収まることを保証します。更新前の有効要素は正規化しておき、返された変化量を親ノードの最小値に加えます。

`Narrow=false` では、パディングを `std::numeric_limits<U>::max()` にします。負更新で差分が wrap する場合も扱い、`U` 全域の差分を比較します。

`minimum` の空区間は `Narrow=false` なら `std::numeric_limits<U>::max()` を返します。`Narrow=true` では、64 bit の SIMD 経路は `std::numeric_limits<T>::max()`、スカラー経路は `std::numeric_limits<U>::max()` を `U` で返します。空区間から通常の差分が得られるとみなして加算しないでください。

`min_signed`・`broadcast_min`・`reduce_min`・`minimum_vector` は AVX2 有効時だけ定義される、64 bit 整数用の補助関数です。`minimum_vector` は `0 <= l <= r <= 8` とし、指定区間が有効要素だけを含むようにします。レーン `j` は添字 `j` と `j+4` の小さい方を表します。`l == r` なら全レーンが `neutral` になります。レーン間の最小値はまだ取らず、`reduce_min` または `broadcast_min` と組み合わせます。

`minimum_vector<true>` は元の差分を signed 64 bit として比較します。`minimum_vector<false>` は読み込んだ各差分 `value` を `key = value ^ sign_bit` に変換し、`key` を signed 64 bit として比較します。これにより元の unsigned 順序と一致します。`neutral` はこの変換後の表現で渡し、signed の順序で全候補以上になる値を選びます。通常経路の中立値は `U(std::numeric_limits<T>::max())` です。`false` の返り値に `reduce_min` を適用した後は、さらに `sign_bit` を XOR して元の差分に戻します。配列に保存する差分自体は変換しません。

`Narrow=true` は 64 bit 整数の内部用経路です。`M = std::numeric_limits<T>::max()`、初期列の最大値と最小値の差を $$S_0$$、全区間加算を除く更新の絶対変化量累計を $$A$$ とし、$$\displaystyle L=\left\lfloor\frac{M}{3}\right\rfloor$$ とします。一点代入では新旧の値の差の絶対値を累計に加えます。呼び出し元が $$S_0+A<L$$ を保証し、構築後のパディングを $$2L$$ にします。SIMD の `normalize<true>` はパディングも含めて signed の最小値を計算し、8 レーンすべてからその値を引きます。

有効要素の正規化後の差分は $$[0,S_0+A]$$ に収まります。各ノードの最小値の変化量の絶対値は、その更新で加えた変化量の絶対値以下です。したがって、パディングから差し引かれる正規化量の累計の絶対値も $$A$$ 以下で、パディングの差分は $$[2L-A,2L+A]$$ に収まります。$$A<L$$ なのでパディングは $$L$$ より大きく、有効要素は $$L$$ 未満です。一時的な負の差分も $$-L$$ より大きく、パディングを含むすべての比較対象が signed 64 bit の範囲内です。これにより、パディングを含む signed の最小値計算と全レーンの正規化が成立します。

AVX2 の区間取得では `minimum_vector<true>` に `neutral=2L` を渡し、候補を 4 レーンのまま祖先へ持ち上げます。祖先へ移る際に加える差分の累計を $$C$$ とすると、これは祖先と子孫の最小値の差なので $$0\le C\le S_0+A<L$$ です。無効レーンの値も $$2L+C<3L\le M$$ に収まり、signed 加算はあふれません。有効な候補は祖先の最小値からの差分として $$L$$ 未満のままなので、無効レーンが最小値になることもありません。最後に一度だけレーン間の最小値を取ります。

範囲条件を満たさなくなる更新の前に、各段の末尾のパディングを `std::numeric_limits<U>::max()` に戻し、通常の経路に切り替えます。この管理は `RangeAddRangeMin` が行い、利用者の値域制限は増えません。

64 bit 整数の x86_64 ビルドで AVX-512 が有効なら AVX-512、次に AVX2 が有効なら AVX2 を使い、それ以外はスカラー処理です。AVX2 の通常経路は比較時だけ符号ビットを反転し、保存する差分の形式は変えません。

子の数は 8 に固定されているため、各操作は $$O(1)$$ 時間・追加空間です。
