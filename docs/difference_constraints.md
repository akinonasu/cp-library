---
title: 牛ゲー（差分制約系）
documentation_of: //graph/shortest_path/difference_constraints.hpp
redirect_from:
  - /graph/difference_constraints.hpp.html
  - /lightweight/graph/difference_constraints.hpp.html
---

整数変数に対する差分制約を扱います。解の存在判定・復元と、変数間の差や各変数の最大値・最小値を求められます。

```cpp
#include "lib/graph/shortest_path/difference_constraints.hpp"

DifferenceConstraints<> system(3);
system.add_range(0, 1, 2, 5); // 2 <= x[1] - x[0] <= 5
system.add_lt(1, 2, 4);       // x[2] - x[1] < 4
system.fix(0, 10);            // x[0] = 10

auto result = system.solve();
if (result.feasible()) {
	const auto &x = result.values(); // 全制約を満たす解の一つ
	assert(x[0] == 10);
	assert(result.max_difference(0, 2) == 8);
	assert(result.min_difference(0, 1) == 2);
	assert(result.upper_bound(2) == 18);
	assert(!result.lower_bound(2)); // 下に非有界
	auto upper = result.upper_bounds(0); // x[v] - x[0] の最大値: 0, 5, 8
}
```

`DifferenceConstraints<Cost = std::int64_t>(n = 0)` で変数の個数を指定します。変数番号は `[0,n)`、`origin()` は定数 0 を表す頂点 `n` です。二変数の制約・差の問い合わせでは `origin()` も指定できます。差の向きは全て **`x[v] - x[u]`** です。

既定の計算・戻り値は `std::int64_t` です。128 bit が必要な場合は `DifferenceConstraints<__int128_t>` を使ってください。`Cost` は 16 byte 以下の符号付き整数型で、戻り値の `value_type` は `Cost` が 8 byte 以下なら `std::int64_t`、それ以外なら `__int128_t` です。値と途中計算が型の範囲に収まるようにしてください。扱えない値を検出すると `std::overflow_error` を投げます。

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

制約追加は ID を `std::size_t` で返し、等式・区間・固定値は 2 個の ID を `std::pair` で返します。`constraint_count()` はこの本数、`constraints()[id]` は `x[to] - x[from] <= weight` を表す `{from,to,weight}` です。`reserve(count)` にはこの本数の予約量を指定します。

`solve()` は `Result` を返します。結果は呼び出した時点の系を保持し、その後の制約追加に影響されません。

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

解がない場合は解・上下限を取得しないでください。`negative_cycle()` は元の制約辺を閉路の順に返し、その重みの総和は負になります。

`values()` は解の一つで、各変数を最大化した解ではありません。最大値・最小値は `std::optional<value_type>` で返し、`std::nullopt` はその方向に非有界であることを表します。有限な値は実際に取り得る極値です。`upper_bounds` は通常の変数に対応する `vector<optional<value_type>>` を返します。

同じ始点から複数の差を調べる場合は `upper_bounds(source)` を一度呼び、その結果を参照してください。`max_difference` などの個別の問い合わせは、呼び出すたびに計算を行います。

牛ゲーでは、制約を `x[v] - x[u] <= c` の形で表し、変数間の差の上限を最短路として求めます。**この形にそろえた全ての制約で `c >= 0` なら、[Dijkstra 法](dijkstra.md)をそのまま使えます。** `add_le(u,v,c)` のみで制約を追加する場合は、全ての `c` が非負であることが条件です。`origin()` に関する制約も含めます。

この条件を満たす場合、`solve()` は $$O(N+M)$$ 時間で済み、その後の上下限問い合わせは Dijkstra 法と同じ計算量です。負の `c` を含む一般の場合でも、`solve()` が成功した後の上下限問い合わせは同じ計算量になります。

変数と `origin()` を合わせた頂点数を $$N$$、`constraint_count()` を $$M$$ とします。64 bit の計算量では $$w=64$$ とします。

| 操作・条件 | 64 bit | 128 bit |
| --- | --- | --- |
| 制約追加 | 償却 $$O(1)$$ | 償却 $$O(1)$$ |
| `solve()`：一般の場合 | 最悪 $$O(N+NM)$$ | 最悪 $$O(N+NM+M\log M)$$ |
| `solve()`：上記の全ての `c` が非負の場合 | $$O(N+M)$$ | $$O(N+M)$$ |
| 上下限・差の最大値・最小値の取得（1 回） | $$O(N+Mw)$$ | $$O(N+M\log M)$$ |

`solve()` と上下限の取得は $$O(N+M)$$ 追加空間を使います。同じ変数同士の差の取得と、解・負閉路の参照は $$O(1)$$ 時間です。
