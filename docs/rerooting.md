---
title: "全方位木 DP"
documentation_of: //graph/rerooting.hpp
---

全頂点をそれぞれ根にした木 DP を計算します。辺の両方向の寄与も取得できます。次は、各頂点から全頂点までの距離の総和を求める例です。

```cpp
#include "lib/graph/rerooting.hpp"

using Data = std::pair<long long, int>;
Rerooting<Data> tree(3);
std::vector<long long> weight{2, 5};
tree.add_edge(0, 1);
tree.add_edge(1, 2);

const auto &answer = tree.build(Data{0, 0},
	[](Data a, Data b) {
		return Data{a.first + b.first, a.second + b.second};
	},
	[](Data x, int) {
		return Data{x.first, x.second + 1};
	},
	[&](Data x, int, int, int id) {
		return Data{x.first + weight[id] * x.second, x.second};
	});

long long sum = answer[1].first; // 7
auto from_zero = tree.edge_dp(0); // {2, 1}
```

`T` は頂点の DP 値、`E` は辺を通した寄与の型です。`E` を省略すると `T` と同じ型になります。`Data::first` が距離和、`second` が頂点数です。辺重みや頂点の値は、コールバックから参照します。

| 関数・型 | 内容・返り値 |
| --- | --- |
| `Rerooting<T,E = T>(n = 0)` | `n` 頂点の空の森を作る |
| `add_edge(u,v)` | 無向辺を追加し、0 始まりの辺 ID を返す |
| `build(unit,merge,add_vertex,lift,root = 0)` | 全方位木 DP を計算し、全頂点の答えを `const vector<T>&` で返す |
| `tree[v]` | 頂点 `v` を根とした答えを `const T&` で返す |
| `values()` | 全頂点の答えを `const vector<T>&` で返す |
| `edge_dp(id,reverse = false)` | 辺を通した寄与を `const E&` で返す。`add_edge(u,v)` に対して `false` は `u → v`、`true` は `v → u` |
| `edge(id)` | 入力した辺の両端 `(u,v)` を `const pair<int,int>&` で返す |
| `size()` / `empty()` / `edge_count()` | 頂点数／頂点数が 0 か／辺数 |
| `Vertex` / `Edge` | それぞれ `T` / `E` の型別名 |

| コールバック | 内容・返り値 |
| --- | --- |
| `merge(const E& a,const E& b) -> E` | 隣接する成分の寄与を結合する。結合則と両側の単位元 `unit` が必要。可換性・逆元は不要 |
| `add_vertex(const E& x,int v) -> T` | 頂点 `v` に集まった寄与に `v` 自身を加える |
| `lift(const T& x,int from,int to,int id) -> E` | 頂点 `from` を根とした値を、辺 `id` を通して `to` 側の寄与に変換する |

`edge_dp(id)` は、その辺を削除したときの送信元側の成分に対し、送信元の頂点を加え、最後に辺を通す変換を施した値です。受信先の頂点は含みません。例の `from_zero` は頂点 0 だけを含み、頂点 1 から測った距離が 2、頂点数が 1 です。

同じ頂点の隣接辺は `add_edge` の呼び出し順に結合します。親への辺を除くときも、残った辺の順序を変えません。各コールバックは引数と入力データだけで返り値が決まるようにします。`root` は内部の走査を始める頂点で、答えには影響しません。再帰は使いません。

入力は 0 始まりの木・森とし、自己ループ・多重辺・閉路を含めないでください。森では各頂点の属する連結成分について計算します。孤立点の答えは `add_vertex(unit,v)`、空の森の答えは空配列です。空でない場合は $$0\le\texttt{root}<n$$ とします。

頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下とします。

値型はコピー・代入可能としてください。`T` と `E` が異なる場合は `T` のデフォルト構築も必要です。戻り値をコピーしたくない場合は `const auto&` で受け取ります。`tree[v]`・`values()`・`edge_dp` は `build` 後に使います。辺の追加や再構築後は、以前取得した参照を使わないでください。入力データやコールバックを変えて `build` を呼び直すこともできます。

頂点数を $$n$$ とし、各コールバック・値のコピーを $$O(1)$$ とすると、構築・`build` は $$O(n)$$ 時間・空間です。`add_edge` は償却 $$O(1)$$、そのほかの取得は $$O(1)$$ です。
