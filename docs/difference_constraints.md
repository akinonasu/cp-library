---
title: 牛ゲー（差分制約系）
documentation_of: //graph/difference_constraints.hpp
---

整数変数に対する差分制約を解きます。全ての制約を満たす解の存在判定、解の復元、変数間の差や各変数の取り得る最大値・最小値を求められます。

```cpp
#include "lib/graph/difference_constraints.hpp"

DifferenceConstraints<> system(3);
system.add_range(0, 1, 2, 5); // 2 <= x[1] - x[0] <= 5
system.add_lt(1, 2, 4);       // x[2] - x[1] < 4
system.fix(0, 10);            // x[0] = 10

auto result = system.solve();
if (result.feasible()) {
	const auto &x = result.values();
	assert(x[0] == 10 && 2 <= x[1] - x[0] && x[1] - x[0] <= 5);
	assert(result.max_difference(0, 2) == 8);
	assert(result.min_difference(0, 1) == 2);
	assert(result.upper_bound(2) == 18);
	assert(!result.lower_bound(2)); // 下に非有界
	auto upper = result.upper_bounds(0); // x[v] - x[0] の最大値: 0, 5, 8
}
```

`DifferenceConstraints<Cost = std::int64_t>(n = 0)` で変数の個数を指定します。変数番号は `[0,n)` です。`origin()` は定数 0 を表す補助頂点 `n` を返します。二変数の制約・差の問い合わせでは、この補助頂点も指定できます。例えば `add_lt(origin(),v,c)` は `x[v] < c` を表します。

| 制約追加 | 意味 |
| --- | --- |
| `add_le(u,v,c)` | `x[v] - x[u] <= c` |
| `add_ge(u,v,c)` | `x[v] - x[u] >= c` |
| `add_lt(u,v,c)` | `x[v] - x[u] < c` |
| `add_gt(u,v,c)` | `x[v] - x[u] > c` |
| `add_eq(u,v,c)` | `x[v] - x[u] == c` |
| `add_range(u,v,lower,upper)` | `lower <= x[v] - x[u] <= upper` |
| `add_upper_bound(v,c)` | `x[v] <= c` |
| `add_lower_bound(v,c)` | `x[v] >= c` |
| `fix(v,c)` | `x[v] == c` |
| `add_range(v,lower,upper)` | `lower <= x[v] <= upper` |

向きは全て **`x[v] - x[u]`** で統一しています。実数変数を扱う API ではありません。厳密不等号は整数であることを使い、`< c` を `<= c-1`、`> c` を `>= c+1` に変換します。`lower > upper` の区間や、自身との差に矛盾する制約も追加でき、`solve` で解なしになります。自己辺・多重制約にも対応します。

`DifferenceConstraints<>` は内部の辺重み・距離・解・上下限を全て `std::int64_t` で扱います。128 bit が必要な場合だけ、構築時に型を指定してください。

```cpp
DifferenceConstraints<> normal(n);          // 64 bit
DifferenceConstraints<__int128_t> large(n); // 128 bit
```

`Cost` は 16 byte 以下の符号付き整数型です。`value_type` は、`Cost` が 8 byte 以下なら `std::int64_t`、それより大きければ `__int128_t` です。`Cost` は制約の入力型、`value_type` は正規化済み制約・内部計算・戻り値の型になります。自動的な型の切り替えは行いません。

**制約の変換、Bellman–Ford の緩和、解の平行移動、再重み付け、最短距離、戻す上下限が選んだ型の範囲に収まるようにしてください。** これらが範囲を超えるときは `std::overflow_error` を投げます。負閉路を発見する前の距離更新であふれる場合も同様です。再重み付け後の重みと有限距離には、その型の最大値を無限大の番兵として予約しています。範囲超過を非有界として返すことはありません。

64 bit 版で `add_ge(u,v,INT64_MIN)` や `add_lt(u,v,INT64_MIN)` を使う場合、正規化した辺重みが収まらないため例外になります。これらや 64 bit を超える答えを扱う場合は `DifferenceConstraints<__int128_t>` を使ってください。通常の `cout` は `__int128_t` を直接出力できないので、必要なら [Fast I/O](fast_io.md) の `FastOutput::write` などを使ってください。

`n` は 0 以上 `INT_MAX-2` 以下としてください。空の系も扱えます。補助頂点に対する `add_le(origin(),origin(),-1)` のような制約は、変数がなくても矛盾として検出します。

`add_le`・`add_ge`・`add_lt`・`add_gt`・上下限は、追加した正規化済み制約の ID を `std::size_t` で返します。等式・区間・固定値は 2 本に変換し、2 個の ID を `std::pair` で返します。`constraint_count()` はこの辺数、`constraints()[id]` は `{from,to,weight}` で、`x[to] - x[from] <= weight` を表します。`reserve(count)` では正規化後の本数を指定します。

`solve()` の戻り値は、呼び出した時点の系を独立に保持する `Result` です。後から元の系へ制約を追加しても、以前の結果は変わりません。`solve` は元の系を変更せず、結果は元の系の寿命にも依存しません。

| 結果の操作 | 内容・返り値 |
| --- | --- |
| `feasible()` | 全制約を同時に満たせるか |
| `size()` / `origin()` | 変数数／定数 0 の補助頂点 |
| `values()` | 実現可能な解を `const vector<value_type>&` で取得。通常の変数のみ |
| `value(v)` | 解の 1 変数。`value(origin())` は 0 |
| `negative_cycle()` | 矛盾を示す正規化済み制約の閉路。解があれば空 |
| `upper_bounds(source)` | 全変数について `x[v] - x[source]` の最大値 |
| `upper_bounds()` | 全変数について `x[v]` の最大値 |
| `max_difference(u,v)` | `x[v] - x[u]` の最大値 |
| `min_difference(u,v)` | `x[v] - x[u]` の最小値 |
| `upper_bound(v)` / `lower_bound(v)` | `x[v]` の最大値／最小値 |

**解がない場合は `negative_cycle()` のみを参照し、解・上下限の取得は行わないでください。** 矛盾の検出は全成分を対象とするので、問い合わせる変数から離れた場所に矛盾がある場合も `feasible()` は偽になります。

`values()` は実現可能な解の一つで、各変数を最大化した解ではありません。返す参照は結果オブジェクトが保持する配列を指します。

最大値・最小値は `std::optional<value_type>` で返します。`std::nullopt` は、その方向へ非有界であることを表します。解がないという意味ではありません。有限なら、返す値は実際に取り得る最大値・最小値です。`-1` も有限な答えになり得ます。

`upper_bounds` は通常の変数数だけの `vector<optional<value_type>>` を返します。同じ始点から複数の差を問い合わせる場合は、`upper_bounds(source)` を一度だけ呼んで結果を参照すると、最短路計算を共有できます。単独の `max_difference` などは、呼び出すたびに最短路を計算します。

`negative_cycle()` の各要素は `{from,to,weight}` です。辺の向きに並び、最後の `to` が最初の `from` と一致し、重みの総和が負になります。元の系の `constraints()` に含まれる辺だけで構成し、`origin()` を含む場合もあります。これらの不等式を足し合わせると矛盾が得られます。

`x[v] - x[u] <= c` を重み c の辺 `u -> v` にし、全頂点へ重み 0 の辺を持つ仮想始点から [Bellman–Ford](bellman_ford.md) を実行します。負閉路がなければ距離を実現可能な解とし、定数 0 の頂点が 0 になるよう平行移動します。差の最大値は最短路に一致するので、解をポテンシャルとして非負重みに変換し、[Dijkstra](dijkstra.md) で問い合わせます。再帰は使いません。

変数と定数 0 の頂点を合わせた頂点数を $$N$$、正規化後の制約辺数を $$M$$ とします。制約追加は 1 回あたり償却 $$O(1)$$、`solve` は最悪 $$O(N+NM+M\log M)$$ 時間・$$O(N+M)$$ 空間です。128 bit 経路で使う比較ヒープも含めた上界で、64 bit 経路では radix heap を使います。差の上限・下限の取得は 1 回あたり $$O(N+M\log M)$$ 時間・$$O(N+M)$$ 追加空間です。同じ頂点同士の差は $$O(1)$$、既に求めた解・負閉路の参照は $$O(1)$$ 時間です。返す上下限の配列は $$O(N)$$ 空間です。
