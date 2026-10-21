---
title: Project Selection（燃やす埋める）
documentation_of: //graph/flow/project_selection.hpp
---

各頂点を選ぶ・選ばないの 2 通りに分け、費用の総和を最小化します。利益は負の費用として加えます。選択を `true`（1）、非選択を `false`（0）とし、最小カットの始点側を選択側に対応させます。

```cpp
#include "lib/graph/flow/project_selection.hpp"
#include <cassert>

ProjectSelection<> problem(2);
problem.add_profit(0, 10); // 0 を選ぶと利益 10
problem.add_cost(1, 4);    // 1 を選ぶと費用 4
problem.imply(0, 1);       // 0 を選ぶなら 1 も選ぶ

auto result = problem.solve();
assert(result && result->profit() == 6);
assert(result->cost == -6);
assert(result->selected[0] && result->selected[1]);
```

| 操作 | 内容・返り値 | 追加時の計算量 |
| --- | --- | --- |
| `ProjectSelection<Cost = std::int64_t>(n = 0)` | `n` 頂点の問題を作る。頂点番号は `[0,n)` | $$O(n)$$ |
| `size()` | 利用者が指定した頂点数。内部の補助頂点は含まない | $$O(1)$$ |
| `add_constant(cost)` | 選択によらない費用を加える | $$O(1)$$ |
| `add_cost(v,cost)` | `v` を選ぶ場合に費用を加える | $$O(1)$$ |
| `add_cost(v,c0,c1)` | `v` を選ばない場合に `c0`、選ぶ場合に `c1` を加える | $$O(1)$$ |
| `add_profit(v,profit)` | `v` を選ぶ場合に利益を加える | $$O(1)$$ |
| `add_profit(v,p0,p1)` | `v` を選ばない場合に `p0`、選ぶ場合に `p1` の利益を加える | $$O(1)$$ |
| `add_penalty(u,v,cost)` | `u` を選び `v` を選ばない場合だけ費用を加える。`cost >= 0` | 償却 $$O(1)$$ |
| `add_difference_cost(u,v,cost)` | `u`・`v` の選択が異なる場合に費用を加える。`cost >= 0` | 償却 $$O(1)$$ |
| `add_pair_cost(u,v,c00,c01,c10,c11)` | 選択 `(u,v)` が `00,01,10,11` の各場合の費用を加える。下記の条件が必要 | 償却 $$O(1)$$ |
| `add_all_profit(vertices,value,profit)` | 指定した全頂点の選択が `value` と一致する場合だけ利益を加える。`profit >= 0` | 償却 $$O(k+1)$$ |
| `force(v,value)` | `v` の選択を `value` に固定する | 償却 $$O(1)$$ |
| `imply(u,v)` | `u` を選ぶなら `v` も選ぶという制約を加える | 償却 $$O(1)$$ |
| `same(u,v)` | `u` と `v` の選択を一致させる | 償却 $$O(1)$$ |
| `solve()` | 最小費用と選択を `optional<Result>` で返す。実行不能なら `nullopt` | 最大流の計算量 |

`Cost` は符号付き 64 bit 以下の標準整数型です。定数項・一変数の費用と利益は負でも構いません。呼び出した項は全て加算され、既存の項を上書きしません。`Result::cost` は最小費用、`profit()` はその符号を反転した最大純利益、`selected` は長さ `n` の `vector<bool>` です。費用の合計には `__int128_t` を使い、`total_type` で参照できます。出力には [FastOutput](https://qandori.github.io/cp-library/components/fast_io.hpp.html) が使えます。

`solve()` は問題を変更しません。繰り返し呼べ、求解後に項や制約を追加した場合は、次の `solve()` で再構築して解きます。コピーした問題は独立に変更できます。最適解が複数ある場合はそのうち 1 つを返します。

二変数コストは劣モジュラ条件 $$c_{00}+c_{11}\leq c_{01}+c_{10}$$ を満たす必要があります。各費用自体は負でも構いません。`u == v` の場合は `c00`・`c11` だけを使い、劣モジュラ条件は不要です。任意の二変数コストを扱えるわけではありません。例えば「両方選んだ場合だけ正の費用」は、この選択の向きでは条件を満たしません。

`add_all_profit` は、全員を選ぶ場合と全員を選ばない場合の両方に使えます。空の頂点列では常に利益を得ます。重複頂点を渡しても条件の意味は変わりませんが、処理量は重複を含む長さ `k` に比例します。補助頂点は最大 1 個、辺は最大 `k` 本追加します。

```cpp
ProjectSelection<> model(3);
model.add_all_profit({0, 1, 2}, true, 20); // 全員を選ぶと利益 20
model.add_all_profit({0, 1}, false, 7);   // 両方を選ばないと利益 7
model.add_penalty(0, 2, 5);              // 0 だけを選ぶと費用 5
model.force(1, false);
auto answer = model.solve();
```

依存関係は閉路を持っていても構いません。必須・禁止などの矛盾は、制約辺だけで始点から終点へ到達できるかを調べて判定します。「無限大」の値は利用者が指定せず、有限容量の総和より 1 大きい値を内部で設定します。

各頂点の一変数項は差分へ集約します。その差分の絶対値と、変換後の有限容量辺の容量の総和を $$B$$ として、$$B<2^{64}-1$$ が必要です。二変数項から作る容量にも符号なし 64 bit を使います。項の集約・定数項・最終費用の計算途中は符号付き 128 bit に収めてください。補助頂点を含む変数数は `INT_MAX/2-3` 以下、始点・終点を含む全辺数は `INT_MAX/2` 以下とします。これらの条件は入力の前提で、実行不能を表す `nullopt` とは区別します。

元の頂点数を $$n$$、補助頂点・始点・終点を含む頂点数を $$V$$、一変数項の集約後の辺数を $$E$$ とします。構築は $$O(n)$$ 時間・空間、項と制約の保持は $$O(V+E)$$ 空間です。`solve()` はグラフの構築・制約の矛盾判定・解の復元に $$O(V+E)$$ 時間、最大流に一般の上界で $$O(V^2E)$$ 時間、追加空間は $$O(V+E)$$ です。
