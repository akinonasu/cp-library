---
title: Project Selection（燃やす埋める）
documentation_of: //graph/flow/project_selection.hpp
---

各頂点を選ぶ・選ばないの 2 通りに分け、費用の総和を最小化します。利益は負の費用として加えます。選択を `true`（1、燃やす）、非選択を `false`（0、埋める）とします。反転を指定しない頂点では、最小カットの始点側が選択側です。

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
| `ProjectSelection(n,flipped)` | 長さ `n` の `vector<bool>` で、内部の選択を反転する頂点を指定する | $$O(n)$$ |
| `size()` | 利用者が指定した頂点数。内部の補助頂点は含まない | $$O(1)$$ |
| `is_flipped(v)` | 頂点 `v` の反転指定を返す | $$O(1)$$ |
| `add_constant(cost)` | 選択によらない費用を加える | $$O(1)$$ |
| `add_cost(v,cost)` | `v` を選ぶ場合に費用を加える | $$O(1)$$ |
| `add_cost(v,c0,c1)` | `v` を選ばない場合に `c0`、選ぶ場合に `c1` を加える | $$O(1)$$ |
| `add_profit(v,profit)` | `v` を選ぶ場合に利益を加える | $$O(1)$$ |
| `add_profit(v,p0,p1)` | `v` を選ばない場合に `p0`、選ぶ場合に `p1` の利益を加える | $$O(1)$$ |
| `add_cost_if(v,value,cost)` | `v` の選択が `value` の場合だけ費用を加える | $$O(1)$$ |
| `add_profit_if(v,value,profit)` | `v` の選択が `value` の場合だけ利益を加える | $$O(1)$$ |
| `add_cost_if(u,v,a,b,cost)` | 選択 `(u,v)` が `(a,b)` の場合だけ費用を加える | 償却 $$O(1)$$ |
| `add_profit_if(u,v,a,b,profit)` | 選択 `(u,v)` が `(a,b)` の場合だけ利益を加える | 償却 $$O(1)$$ |
| `add_penalty(u,v,cost)` | `u` を選び `v` を選ばない場合だけ費用を加える。`cost >= 0` | 償却 $$O(1)$$ |
| `add_difference_cost(u,v,cost)` | `u`・`v` の選択が異なる場合に費用を加える。`cost >= 0` | 償却 $$O(1)$$ |
| `add_same_cost(u,v,cost)` | `u`・`v` の選択が同じ場合に費用を加える。`cost >= 0` | 償却 $$O(1)$$ |
| `add_same_profit(u,v,profit)` | `u`・`v` の選択が同じ場合に利益を加える。`profit >= 0` | 償却 $$O(1)$$ |
| `add_difference_profit(u,v,profit)` | `u`・`v` の選択が異なる場合に利益を加える。`profit >= 0` | 償却 $$O(1)$$ |
| `add_pair_cost(u,v,c00,c01,c10,c11)` | 選択 `(u,v)` が `00,01,10,11` の各場合の費用を加える。下記の条件が必要 | 償却 $$O(1)$$ |
| `add_pair_profit(u,v,p00,p01,p10,p11)` | 選択 `(u,v)` が `00,01,10,11` の各場合の利益を加える。下記の条件が必要 | 償却 $$O(1)$$ |
| `add_all_profit(vertices,value,profit)` | 指定した全頂点の選択が `value` と一致する場合だけ利益を加える。`profit >= 0` | 償却 $$O(k+1)$$ |
| `add_any_cost(vertices,value,cost)` | 指定した頂点のうち少なくとも 1 つの選択が `value` と一致する場合に、費用を 1 回だけ加える。`cost >= 0` | 償却 $$O(k+1)$$ |
| `force(v,value)` | `v` の選択を `value` に固定する | 償却 $$O(1)$$ |
| `imply(u,v)` | `u` を選ぶなら `v` も選ぶという制約を加える | 償却 $$O(1)$$ |
| `imply(u,v,a,b)` | `u` の選択が `a` なら `v` の選択が `b` という制約を加える | 償却 $$O(1)$$ |
| `forbid(u,v,a,b)` | 選択 `(u,v)` が `(a,b)` となることを禁止する | 償却 $$O(1)$$ |
| `same(u,v)` | `u` と `v` の選択を一致させる | 償却 $$O(1)$$ |
| `different(u,v)` | `u` と `v` の選択を異ならせる | 償却 $$O(1)$$ |
| `at_most_one(u,v)` | 両方を選ぶことを禁止する | 償却 $$O(1)$$ |
| `at_least_one(u,v)` | 少なくとも片方を選ぶことを必須にする | 償却 $$O(1)$$ |
| `solve()` | 最小費用と選択を `optional<Result>` で返す。実行不能なら `nullopt` | 最大流の計算量 |

`Cost` は符号付き 64 bit 以下の標準整数型です。定数項・一変数の費用と利益は負でも構いません。呼び出した項は全て加算され、既存の項を上書きしません。`Result::cost` は最小費用、`profit()` はその符号を反転した最大純利益、`selected` は長さ `n` の `vector<bool>` です。費用の合計には `__int128_t` を使い、`total_type` で参照できます。出力には [FastOutput](https://qandori.github.io/cp-library/components/fast_io.hpp.html) が使えます。

`solve()` は問題を変更しません。繰り返し呼べ、求解後に項や制約を追加した場合は、次の `solve()` で再構築して解きます。コピーした問題は独立に変更できます。最適解が複数ある場合はそのうち 1 つを返します。

反転は構築時に指定し、問題の途中では変更しません。`flipped[v] == true` の頂点では、内部の変数を元の選択と逆向きに扱います。費用・利益・制約の引数と `Result::selected` は、全て元の選択の意味です。例えば `force(v,true)` は、反転指定によらず元の頂点を選ぶという制約です。

二変数コストは、反転後の表で劣モジュラ条件を満たす必要があります。元の選択に対する費用の条件は次の通りです。各費用自体は負でも構いません。`add_pair_profit` は、利益の符号を反転して同じ条件を判定します。

| `u`・`v` の反転指定 | 二変数コストの条件 |
| --- | --- |
| 同じ | $$c_{00}+c_{11}\leq c_{01}+c_{10}$$ |
| 異なる | $$c_{00}+c_{11}\geq c_{01}+c_{10}$$ |

`add_cost_if`・`add_profit_if` の二変数版にも同じ条件が適用されます。正の罰金なら、反転後の指定状態が異なる場合に使えます。正の報酬なら、反転後の指定状態が同じ場合に使えます。引数の額は負でも構いませんが、符号が逆なら許される指定状態も逆になります。額が 0 なら反転の制約はありません。

例えば、二部グラフの辺ごとに「両方を燃やすと罰金」「両方を埋めると罰金」を加える場合、片側だけを反転します。次の `0,2` と `1,3` が二部に分かれています。

```cpp
ProjectSelection<> problem(4, {false, true, false, true});
problem.add_cost_if(0, true, 3);          // 0 を燃やすと罰金 3
problem.add_cost_if(1, false, 5);         // 1 を埋めると罰金 5
problem.add_cost_if(0, 1, true, true, 7); // 両方を燃やすと罰金 7
problem.add_cost_if(2, 3, false, false, 4); // 両方を埋めると罰金 4
problem.add_all_profit({0, 2}, true, 11); // 両方を燃やすと報酬 11
problem.add_all_profit({1, 3}, false, 8); // 両方を埋めると報酬 8
problem.at_most_one(0, 1);              // 同時には選べない
problem.at_least_one(2, 3);             // 少なくとも一方を選ぶ
auto answer = problem.solve();
```

燃やす・埋めるの代表的な条件は、以下の操作で表せます。金額 `P` は正とします。

| 条件 | 操作 | 反転指定の条件 |
| --- | --- | --- |
| `u` を燃やすと罰金 | `add_cost_if(u,true,P)` | 任意 |
| `u` を埋めると罰金 | `add_cost_if(u,false,P)` | 任意 |
| `u` を燃やし `v` を埋めると罰金 | `add_cost_if(u,v,true,false,P)` | `u`・`v` で同じ |
| 両方を燃やすと罰金 | `add_cost_if(u,v,true,true,P)` | `u`・`v` で異なる |
| 両方を埋めると罰金 | `add_cost_if(u,v,false,false,P)` | `u`・`v` で異なる |
| 選択が同じなら罰金 | `add_same_cost(u,v,P)` | `u`・`v` で異なる |
| 選択が異なるなら罰金 | `add_difference_cost(u,v,P)` | `u`・`v` で同じ |
| 選択が同じなら報酬 | `add_same_profit(u,v,P)` | `u`・`v` で同じ |
| 選択が異なるなら報酬 | `add_difference_profit(u,v,P)` | `u`・`v` で異なる |
| 全員を燃やす／埋めると報酬 | `add_all_profit(vertices,value,P)` | 列内の反転指定が全て同じ |
| 一人でも燃やす／埋めると罰金 | `add_any_cost(vertices,value,P)` | 列内の反転指定が全て同じ |

同じ問題に追加する全ての項・制約が、同一の反転指定で条件を満たす必要があります。任意の二変数コストや、二部に分けられないグラフ上の「同じ選択なら罰金」を扱えるわけではありません。反転は呼び出しごとに独立に行うものではありません。

`add_all_profit` は、全員を選ぶ場合と全員を選ばない場合の両方に使えます。空の頂点列では常に利益を得ます。重複頂点を渡しても条件の意味は変わりませんが、処理量は重複を含む長さ `k` に比例します。補助頂点は最大 1 個、辺は最大 `k` 本追加します。

`add_any_cost` は、誰も指定状態でなければ費用を加えず、1 人以上なら人数によらず 1 回だけ費用を加えます。空の頂点列では常に費用 0 です。両関数とも、正の額を指定して列に 2 項目以上ある場合は、列内の反転指定を全て同じにしてください。列が空または 1 項目の場合、この条件は不要です。

```cpp
ProjectSelection<> model(3);
model.add_all_profit({0, 1, 2}, true, 20); // 全員を選ぶと利益 20
model.add_all_profit({0, 1}, false, 7);   // 両方を選ばないと利益 7
model.add_penalty(0, 2, 5);              // 0 だけを選ぶと費用 5
model.force(1, false);
auto answer = model.solve();
```

依存関係は閉路を持っていても構いません。必須・禁止などの矛盾は、制約辺だけで始点から終点へ到達できるかを調べて判定します。「無限大」の値は利用者が指定せず、有限容量の総和より 1 大きい値を内部で設定します。

`forbid(u,v,a,b)` は、反転後の `a` と `b` が異なる場合に使えます。`imply(u,v,a,b)` は、反転後の `a` と `b` が同じ場合に使えます。従って `same`・二引数の `imply` は両頂点の反転指定が同じ場合、`different`・`at_most_one`・`at_least_one` は異なる場合に使えます。対応しない項・制約の追加は入力条件違反として `assert` で検査し、実行不能を返す `nullopt` とは区別します。

`u == v` の二変数コスト・利益は `00` と `11` だけを使い、劣モジュラ条件は不要です。指定状態の二変数費用・利益で同じ頂点へ異なる状態を指定した場合は、常に寄与 0 です。`forbid(v,v,a,b)` も `a != b` なら制約なし、`a == b` ならその状態を禁止します。`different(v,v)` は実行不能、`at_most_one(v,v)` は非選択への固定、`at_least_one(v,v)` は選択への固定です。

各頂点の一変数項は差分へ集約します。その差分の絶対値と、変換後の有限容量辺の容量の総和を $$B$$ として、$$B<2^{64}-1$$ が必要です。二変数項から作る容量にも符号なし 64 bit を使います。項の集約・定数項・最終費用の計算途中は符号付き 128 bit に収めてください。補助頂点を含む変数数は `INT_MAX/2-3` 以下、始点・終点を含む全辺数は `INT_MAX/2` 以下とします。これらの条件は入力の前提で、実行不能を表す `nullopt` とは区別します。

元の頂点数を $$n$$、補助頂点・始点・終点を含む頂点数を $$V$$、一変数項の集約後の辺数を $$E$$ とします。構築は $$O(n)$$ 時間・空間、項と制約の保持は $$O(V+E)$$ 空間です。`solve()` はグラフの構築・制約の矛盾判定・解の復元に $$O(V+E)$$ 時間、最大流に一般の上界で $$O(V^2E)$$ 時間、追加空間は $$O(V+E)$$ です。
