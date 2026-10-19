---
title: "Kinetic Tournament (固定係数昇順・更新係数非負)"
documentation_of: //data_structure/segment_tree/kinetic_tournament.hpp
---

固定した昇順の係数列 $$V$$ と値の列 $$W$$ を管理します。区間内で $$W_i\gets W_i+aV_i+b$$ とする更新と、区間の最大値・その添字を求めます。$$a\geq0$$ が必要です。$$b$$ は負でも構いません。[JSC2024 決勝 D の公式解説](https://atcoder.jp/contests/jsc2024-final/editorial/10766)で使われる、区間加算を扱う Segtree Beats です。

```cpp
#include "lib/data_structure/segment_tree/kinetic_tournament.hpp"

int main() {
	std::vector<std::int64_t> coefficients{1, 3, 5};
	std::vector<std::int64_t> initial{10, 4, 0};
	KineticTournament<> tree(coefficients, initial);
	assert(tree.all_prod()->index == 0);
	tree.advance(0, 3, 2);
	assert(tree.all_prod()->value == 12);
	tree.apply(1, 3, 1, -2);
	auto best = tree.prod(0, 3);
	assert(best && best->index == 2 && best->value == 13);
	tree.add(0, 1, 5);
	assert(tree.all_prod()->index == 0 && tree.get(0) == 17);
	tree.erase(0);
	assert(!tree.get(0) && tree.active_size() == 2);
	tree.set(0, 100);
	assert(tree.get(0) == 100);
}
```

`V` には **重複を許す昇順** の列を渡します。内部では並べ替えません。添字の並びと係数は構築後に固定されます。`set(i,value)` が変更するのは値 $$W_i$$ で、係数 $$V_i$$ は変更しません。係数の変更には `build` による再構築が必要です。

区間は全て 0 始まりの半開区間 `[l,r)` です。`apply(l,r,a,b)` は、その時点で有効な区間内の各要素に `a*V[i]+b` を加えます。`add(l,r,b)` は一様加算、`advance(l,r,t)` は `t*V[i]` の加算です。`t` は絶対時刻ではなく、今回加える **非負の増分** です。区間ごとに別々の量を加えられます。空区間への更新は何もしません。

`KineticTournament<>` は最大値版、`KineticTournament<T,false>` は最小値版です。返り値は `optional<Result>` で、`Result.index` が添字、`Result.value` が値です。同値の場合、最大値版は添字が大きい方、最小値版は添字が小さい方を返します。全体であれば `all_prod()`、区間であれば `prod(l,r)` を呼びます。空区間や有効な要素がない区間では `nullopt` です。

`KineticTournament(V)` は全要素の値を 0 にします。第 2 引数に値を 1 つ渡すと全要素をその値にし、`vector<T>` を渡すと各要素を設定します。`KineticTournament(V,std::nullopt)` は全位置を無効にして作ります。個別に無効な位置を指定したい場合は `vector<optional<T>>` を渡し、無効な位置に `nullopt` を入れます。

`erase(i)` は位置 `i` を無効にします。その位置は区間更新・問い合わせから除外され、`set(i,value)` で再び有効にできます。**無効だった間の加算は、後から有効にした要素には反映されません。** 番兵の値で無効な要素を表さないため、型の最小値・最大値も有効な値として使えます。

`T` は符号付き 64 bit 以下の整数型で、既定は `int64_t` です。係数・更新の引数・各更新後の有効な要素の値は `T` に収めてください。差、積、遅延更新の合成は `__int128_t` で計算します。内部で累積する更新係数・加算量と、その評価途中も符号付き 128 bit に収めてください。浮動小数点や利用者のマクロは使いません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `KineticTournament()` | 長さ 0 の構造 | $$O(1)$$ |
| `KineticTournament(V,value=0)` | 係数列と共通の初期値で構築 | $$O(n)$$ |
| `KineticTournament(V,values)` | `vector<T>` / `vector<optional<T>>` の初期値で構築 | $$O(n)$$ |
| `KineticTournament(V,std::nullopt)` | 全位置を無効にして構築 | $$O(n)$$ |
| `build(V,value=0)` / `build(V,values)` / `build(V,std::nullopt)` | 上記と同じ指定で再構築 | 新旧の長さの合計に対して $$O(n)$$ |
| `apply(l,r,a,b=0)` | 区間で `W[i] += a*V[i]+b`。`a >= 0` | 償却 $$O(\log^2 n)$$、1 回の最悪 $$O(n)$$ |
| `add(l,r,b)` | 区間で `W[i] += b` | $$O(\log n)$$ |
| `advance(l,r,t=1)` | 区間で `W[i] += t*V[i]`。`t >= 0` | 償却 $$O(\log^2 n)$$、1 回の最悪 $$O(n)$$ |
| `all_apply(a,b=0)` | 全体で `W[i] += a*V[i]+b` | 償却 $$O(\log^2 n)$$、1 回の最悪 $$O(n)$$ |
| `prod(l,r)` | 区間の最適な添字と値 | $$O(\log n)$$ |
| `all_prod()` | 全体の最適な添字と値 | $$O(1)$$ |
| `set(i,value)` | 値を設定して有効にする。係数は維持 | $$O(\log n)$$ |
| `erase(i)` | 無効にする。有効だった場合は true | $$O(\log n)$$、既に無効なら $$O(1)$$ |
| `get(i)` | 現在値を `optional<T>` で返す。無効なら `nullopt` | $$O(\log n)$$、無効なら $$O(1)$$ |
| `values()` | 全位置の現在値を `vector<optional<T>>` で返す | $$O(n)$$ |
| `slopes()` | 固定係数列への const 参照 | $$O(1)$$ |
| `size()` / `active_size()` / `empty()` | 添字数 / 有効な要素数 / 有効な要素がないか | $$O(1)$$ |
| `clear()` | 係数列と長さを保って全位置を無効にする | $$O(n)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |
| コピー / ムーブ | 独立した複製 / 内容を移動。移動元は長さ 0 | コピー $$O(n)$$、ムーブ構築 $$O(1)$$ |

$$n$$ は添字数です。空間・構築は $$O(n)$$。初期構築後の $$q$$ 操作の合計は $$O(n\log n+q\log^2 n)$$ です。初期状態からの交差も数えるため、少数の更新だけで各更新が最悪 $$O(\log^2 n)$$ になるという保証ではありません。係数順が固定され、重み付き加算の係数が非負であることを使った計算量です。

各節点に、現在の勝者と、部分木の勝者が変わらずに進められる増分を保存します。更新がその範囲を超えたときだけ子へ降り、勝者を計算し直します。実際に保持するのは交差前に安全な整数ステップ数で、上限値を「交差なし」に使います。汎用の `GenericSegtreeBeats` を共有します。
