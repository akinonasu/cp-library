---
title: Offline Dynamic Connectivity（クエリ先読み）
documentation_of: //graph/connectivity/offline_dynamic_connectivity.hpp
---

無向グラフの辺の追加・削除と問い合わせを登録し、最後にまとめて計算します。Rollback Union Find を利用します。

```cpp
#include "lib/graph/connectivity/offline_dynamic_connectivity.hpp"
#include <cassert>

int main() {
	OfflineDynamicConnectivity graph(4);
	graph.link(0, 1);
	int a = graph.query_same(0, 1);
	graph.link(1, 2);
	int b = graph.query_size(0);
	graph.cut(1, 0);
	int c = graph.query_same(0, 2);
	int d = graph.query_component_count();
	auto answer = graph.solve();
	assert(answer[a] == 1 && answer[b] == 3);
	assert(answer[c] == 0 && answer[d] == 3);
}
```

頂点は `[0,n)` です。各 `query_*` の返り値は0始まりの問い合わせ ID で、`solve()` の配列をその ID で参照します。問い合わせは登録した位置での状態を調べます。辺の更新だけでは問い合わせ ID は増えません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `OfflineDynamicConnectivity(n=0)` | 頂点数だけを登録。初期状態では辺なし | $$O(1)$$ |
| `reset(n)` | 頂点数を変え、全ての更新・問い合わせを削除 | $$O(M+T)$$ |
| `reserve(updates,queries=0)` | 辺の更新回数・問い合わせ回数を目安に領域を確保 | $$O(M+T)$$ |
| `vertex_count()` / `query_count()` | 頂点数／登録した問い合わせ数 | $$O(1)$$ |
| `link(u,v)` | 無向辺を1本追加する操作を登録 | 償却 $$O(1)$$ |
| `cut(u,v)` | 無向辺を1本削除する操作を登録。存在しなければ何もしない | 償却 $$O(1)$$ |
| `query_same(u,v)` | 連結判定を登録し、問い合わせ ID を返す。答えは0または1 | 償却 $$O(1)$$ |
| `query_size(v)` | `v` の成分サイズの問い合わせを登録 | 償却 $$O(1)$$ |
| `query_component_count()` | 成分数の問い合わせを登録 | 償却 $$O(1)$$ |
| `solve()` | 登録した問い合わせの答えを `vector<int>` で返す | $$O(n+M+(T+K)\log n)$$ |
| `query()` | 任意の問い合わせ位置だけを登録し、問い合わせ ID を返す | 償却 $$O(1)$$ |
| `run(visit)` | 全問い合わせを `visit(id,const_uf)` で処理 | $$O(n+M+T+K\log n)$$ |
| `run(on_link,on_cut,visit)` | 成功した併合・その取り消しの通知を受けながら問い合わせを処理 | $$O(n+M+T+K\log n)$$ |

並行辺に対応します。同じ辺を2本追加すると、1本削除した後も連結関係に使われます。`link(0,1)` と `link(1,0)` は同じ辺として扱います。自己ループは連結性を変えないため、登録時に無視します。

`solve`・`run` は登録内容を変更しません。繰り返し計算でき、登録を追加してから再計算することもできます。その場合は以前の問い合わせも含めて全体を計算し直します。頂点数0でも `query_component_count` と `query()` は使えます。

## 任意の問い合わせ

複数の情報をまとめて調べたい場合は `query()` と `run` を使います。各問い合わせ位置の Rollback Union Find を読み取り専用で受け取れます。

```cpp
OfflineDynamicConnectivity graph(3);
graph.link(0, 1);
graph.query();
graph.link(1, 2);
graph.query();
graph.run([&](int id, const RollbackUnionFind &uf) {
	assert(uf.size(0) == id + 2);
});
```

`visit` は問い合わせ ID の昇順に1回ずつ呼びます。渡された `uf` はその呼び出しの中だけで使ってください。`leader`・`same`・`size`・`component_count`・`groups` を利用できます。独自の問い合わせ位置を作る `query()` を使った場合は、`solve()` ではなく `run` で答えを処理してください。既定の `query_*` も `run` の呼び出し対象になります。

## 成分の情報を追加する

`run(on_link,on_cut,visit)` の `on_link(root,child)` は、異なる成分を併合した直後に呼びます。新しい代表元は `root`、その子になった元の代表元は `child` です。既に連結な場合は呼びません。

`on_cut(root,child)` は、この併合を取り消す直前に呼びます。同じ組の `on_link` に対応し、取り消す順序は後入れ先出しです。これは入力の `cut(u,v)` の通知ではありません。内部の時間区間探索で併合を取り消すたびに呼ばれます。最後は全ての併合を取り消すため、`on_link` と `on_cut` の回数は一致します。

例えば初期頂点値の成分和を調べる場合は、外部の `sum` を次のように更新できます。

```cpp
std::vector<long long> sum = {1, 10, 100};
OfflineDynamicConnectivity graph(3);
graph.link(0, 1);
graph.query();
graph.run([&](int root, int child) {
	sum[root] += sum[child];
}, [&](int root, int child) {
	sum[root] -= sum[child];
}, [&](int id, const RollbackUnionFind &uf) {
	assert(id == 0 && sum[uf.leader(0)] == 11);
});
```

頂点値の加算も行う利用例は [Dynamic Graph Vertex Add Component Sum の検証コード](../test/offline_dynamic_connectivity.test.cpp) にあります。外部にも親を記録し、頂点への加算をその祖先まで反映することで、時刻順の加算と成分和を扱います。

独自の集計は、内部で選ばれる併合順に依存しないようにしてください。問い合わせの答えを次の辺更新の生成に使うことはできません。コールバックからこのグラフを変更しないでください。繰り返し `run` する場合、必要に応じて外部データも開始時の状態へ戻します。

辺の有効期間は、問い合わせがある時点だけに圧縮します。重複辺は本数が0と正の間で変わる区間にまとめ、問い合わせを含まない区間は捨てます。時間セグメント木の操作は1本の配列にまとめ、同じ成分を再度併合する処理とその履歴は省きます。ハッシュ表には依存せず、正規化した64 bitの辺キーを安定基数ソートすることで、同じ辺の登録順を保って有効期間を求めます。

頂点数を $$n$$、自己ループ以外の辺更新数を $$M$$、問い合わせ数を $$T$$、辺の有効区間数を $$I\leq M$$、時間セグメント木に置く辺 ID の総数を $$K=O(I\log T)$$ とします。`run` は $$O(n+M+T+K\log n)$$ 時間で、コールバックとその中で呼ぶ操作の計算量は別に加算します。`solve` は連結判定なども含めて $$O(n+M+(T+K)\log n)$$ 時間です。使用中のデータは $$O(n+M+T+K)$$ 空間です。保持する更新・問い合わせの数はそれぞれ `INT_MAX` 以下としてください。問い合わせがなければ `run` は何もしません。空の場合や長さ1の場合にも定数時間かかります。`reset` は確保済み領域を保持します。
