---
title: "Generic Segtree Beats"
documentation_of: //data_structure/generic_segtree_beats.hpp
---

集約値と更新を定義して使う汎用の Segtree Beats です。区間除算、bitwise AND/OR など、通常の遅延セグメント木では一度に処理できない更新を扱います。[chmin・chmax・加算・代入の専用版](https://qandori.github.io/cp-library/data_structure/segtree_beats.hpp.html) もそのまま使えます。

```cpp
#include "lib/data_structure/generic_segtree_beats.hpp"

struct S { long long sum; int lo, hi, len; };
struct F { int value, divisor; };
S e() { return {0, 0, 0, 0}; }
S op(S a, S b) {
	if (!a.len) return b;
	if (!b.len) return a;
	return {a.sum + b.sum, std::min(a.lo, b.lo), std::max(a.hi, b.hi), a.len + b.len};
}
F id() { return {-1, 1}; }
F composition(F f, F g) {
	if (f.value >= 0) return f;
	if (g.value >= 0) return {g.value / f.divisor, 1};
	return {-1, int(std::min(100001LL, 1LL * f.divisor * g.divisor))};
}
bool mapping(F f, S &a) {
	if (!a.len || (f.value < 0 && f.divisor == 1)) return true;
	int lo = f.value < 0 ? a.lo / f.divisor : f.value;
	int hi = f.value < 0 ? a.hi / f.divisor : f.value;
	if (lo != hi) return false;
	a = {1LL * lo * a.len, lo, hi, a.len};
	return true;
}
using Tree = GenericSegtreeBeats<S, op, e, F, mapping, composition, id>;

Tree seg(std::vector<S>{S{2, 2, 2, 1}, S{5, 5, 5, 1}, S{6, 6, 6, 1}});
seg.apply(1, 3, F{-1, 2});
seg.apply(0, 2, F{3, 1});
long long sum = seg.prod(0, 3).sum;
```

この例は [ABC256 Ex](https://atcoder.jp/contests/abc256/tasks/abc256_h) の非負整数の除算・代入・区間和です。値は $$0$$ 以上 $$10^5$$ 以下とし、除数の合成を $$100001$$ で打ち切ります。それ以上の除数では必ず $$0$$ になるためです。汎用版自体には、この値域や操作の制限はありません。

| テンプレート引数・関数 | 定義 |
| --- | --- |
| `S` | 区間の集約値の型 |
| `op(a,b)` | 左の区間 `a` と右の区間 `b` を結合。結合法則が必要。非可換でも可 |
| `e()` | `op` の単位元。空区間の値 |
| `F` | 更新と、未伝播の更新の合成を表す型 |
| `mapping(f,s)` | `S &s` に更新 `f` を適用し、成功なら true、子へ降りる必要があれば false |
| `composition(f,g)` | `g` の後に `f` を適用する更新を返す |
| `id()` | 何もしない更新 |

| 操作 | 内容・返り値 |
| --- | --- |
| `GenericSegtreeBeats()` | 空で作成 |
| `GenericSegtreeBeats(n,value = e())` | 長さ `n`、各葉の値を `value` にして構築 |
| `GenericSegtreeBeats(a)` | `vector<S>` から構築 |
| `build(a)` | `vector<S>` で構築し直す |
| `apply(l,r,f)` | 半開区間 `[l,r)` に更新 `f` を適用 |
| `apply(p,f)` | 1 点に更新 `f` を適用 |
| `set(p,value)` | 1 点の集約値を `value` に置き換える |
| `get(p)` | 1 点の値を `S` で返す |
| `prod(l,r)` | `[l,r)` の集約値。空なら `e()` |
| `all_prod()` | 全体の集約値。空なら `e()` |
| `max_right(l,predicate)` | `predicate(prod(l,r))` が true である最大の `r` |
| `min_left(r,predicate)` | `predicate(prod(l,r))` が true である最小の `l` |
| `values()` | 遅延を伝播し、全葉の値を `vector<S>` で返す |
| `size()` / `empty()` | 要素数／空か |

`S`・`F` はコピーと代入ができる型にします。デフォルト構築や `F` 同士の等値比較は不要です。`n` での構築は各葉に `e()` を置くので、例のように区間長を持つ場合は長さ 1 の初期値を明示してください。

`mapping` が成功した場合、実際に全要素を更新した集約値と一致させます。単位更新は値を変えず、空区間と 1 要素の区間では必ず成功させてください。失敗時は `s` を変更しても構いません。合成した更新を子へ伝え、子の値から集約し直します。遅延伝播中に子で失敗した場合も、さらに降りて処理します。

`composition` は更新の実際の意味を保つように定義し、オーバーフローにも注意してください。`op` の結合法則、単位元、合成順序などの条件は、定義する `S`・`F` 側で満たす必要があります。

境界探索の述語は `e()` に対して true とし、区間を延ばしたとき true から false へ単調に変わるものを使います。副作用を持たせないでください。`all_prod`・`size`・`empty` 以外は、取得でも内部の遅延状態を変更します。

関数・型のコピーが定数時間であるとします。構築・空間は $$O(n)$$、`all_prod`・`size`・`empty` は $$O(1)$$ です。区間操作・1 点操作・境界探索は、通常の $$O(\log n)$$ に、作用の失敗によって追加で訪問するノード数が加わります。全 $$q$$ 操作での追加訪問数を $$E$$ とすると合計 $$O(n+q\log n+E)$$ です。`values()` は $$O(n)$$ に追加訪問数を加えた時間です。$$E$$ の償却評価は、利用する更新に応じて必要です。
