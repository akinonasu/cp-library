---
title: Dijkstra の内部キュー
documentation_of: //components/dijkstra_queue.hpp
---

`shortest_path_detail::RadixQueue<T>` は [Dijkstra](https://qandori.github.io/cp-library/graph/shortest_path/dijkstra.hpp.html) が使う単調優先度キューです。通常は `dijkstra`・`dijkstra_path` を使います。`T` は 64 bit 以下の整数型、キーは非負です。頂点番号は非負の `int` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RadixQueue<T>()` | 下限 0 の空キューを作る | $$O(w)$$ |
| `empty()` | 要素がないか | $$O(1)$$ |
| `push(key,vertex)` | 最後に取り出したキー以上の要素を追加 | 償却 $$O(1)$$ |
| `pop()` | 最小キーと頂点の `pair<T,int>` を返して削除。非空が必要 | 償却 $$O(w)$$ |

要素が 32 個以下の間は、小さい固定配列から最小値を選びます。33 個以上になった後は radix heap を使います。全ての未処理キーが現在の下限から $$2^{31}-1$$ 以内なら、キーの下位 32 bit と頂点番号を 64 bit にまとめます。距離そのものは元の型で保持し、32 bit の境界を何度越えても扱えます。条件を満たさない追加があれば、未処理キーを復元して 64 bit の radix heap に移します。

バケットの最小キーを追加時に保存し、非空のバケットを bit 列で管理します。Dijkstra 法の最小距離を確定する順序は変えません。CPU 固有の命令を直接使わず、GCC・Clang・Mac で動作します。

次のバケットに要素が 1 個だけある場合は、その要素を直接返して再配置を省きます。

キーの bit 数を $$w\le64$$、同時に保持する要素数の最大を $$q$$ として、追加は償却 $$O(1)$$、取り出しは償却 $$O(w)$$、空間は $$O(q+w)$$ です。64 bit へ移行する 1 回だけは、その時点の要素数に比例する処理が加わります。
