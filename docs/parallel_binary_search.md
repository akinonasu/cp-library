---
title: "並列二分探索"
documentation_of: //data_structure/range_query/parallel_binary_search.hpp
---

更新を先頭から順に適用したとき、各問い合わせの条件が初めて成立する更新回数を求めます。複数の二分探索をまとめ、同じ時刻までの更新を共有します。

```cpp
#include "lib/data_structure/range_query/parallel_binary_search.hpp"
#include "lib/data_structure/union_find/union_find.hpp"
#include <vector>

int main() {
    std::vector<std::pair<int,int>> edges{ {0,1}, {2,3}, {1,2} };
    std::vector<std::pair<int,int>> queries{ {0,0}, {0,1}, {0,3}, {0,4} };
    UnionFind uf;
    auto answer = parallel_binary_search(int(edges.size()), int(queries.size()),
        [&] { uf = UnionFind(5); },
        [&](int id) { auto [u,v] = edges[id]; uf.merge(u,v); },
        [&](int id) { auto [u,v] = queries[id]; return uf.same(u,v); });
    // answer = {0, 1, 3, -1}
}
```

この例では、辺を追加して初めて連結になる時刻を求めています。初めから成立する問い合わせは `0`、最後まで成立しない問い合わせは `-1` です。

| 関数 | 内容・返り値 |
| --- | --- |
| `parallel_binary_search(update_count,query_count,reset,apply,check)` | 各問い合わせが初めて成立する更新回数を、問い合わせ ID 順の `std::vector<int>` で返す |

`update_count`・`query_count` は非負の `int` です。更新 ID は `[0,update_count)`、問い合わせ ID は `[0,query_count)` です。値や更新内容、問い合わせの引数は呼び出し側で保持します。

- `reset()`：共通の状態を、更新を 1 回も適用していない初期状態へ戻します。
- `apply(update_id)`：指定した更新を 1 回適用します。各周回では ID の昇順に `0` から呼ばれます。
- `check(query_id)`：その時点の状態で条件が成立するかを `bool` に変換できる値で返します。

更新を `k` 回適用した状態とは、更新 `0` から `k-1` までを適用した状態です。返り値の各要素は `[0,update_count]` の最小の成立回数、または `-1` です。

各問い合わせの条件は、更新回数について **false から true へ単調に変化する**必要があります。常に true・常に false の条件も扱えます。更新そのものは可換である必要はなく、登録した順番で適用します。`reset` は毎回同じ初期状態を再現してください。`check` の結果は呼び出し回数や問い合わせを評価する順番に依存させないでください。

例以外には、非負の加算を繰り返して区間和がしきい値に到達する時刻や、辺の追加によって連結成分の大きさがしきい値以上になる時刻などを求められます。

問い合わせがなければ空配列を返し、コールバックは呼びません。更新がない場合は `reset()` を 1 回呼び、初期状態で各問い合わせを判定します。条件や問い合わせの重複にも対応します。

`reset()` は関数内から最初に呼ばれるので、呼び出し前に状態を初期化する必要はありません。返却後の状態は初期状態や全更新後の状態とは限りません。コールバックを通じて同じ状態を参照するには、例のように参照キャプチャを使えます。

更新数を $$M$$、問い合わせ数を $$Q$$、初期化・更新 1 回・判定 1 回の時間をそれぞれ $$R,A,C$$ とします。$$M\ge2$$ の場合、全体は $$O((R+MA+QC)\log M)$$ 時間、呼び出し側の状態を除く追加空間は $$O(Q)$$ です。時刻ごとの並べ替えは整数キーの基数ソートで行い、1 周回 $$O(Q)$$ 時間です。$$M=0,1$$ では、初期化 1 回、更新は高々 $$M$$ 回、判定は高々 $$2Q$$ 回です。
