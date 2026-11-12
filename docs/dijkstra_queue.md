---
title: Dijkstra の内部キュー
documentation_of: //components/dijkstra_queue.hpp
---

`shortest_path_detail::RadixQueue<T>` は [Dijkstra](https://qandori.github.io/cp-library/graph/dijkstra.hpp.html) が使う優先度付きキューです。最後に取り出したキー以上の値だけを追加できます。通常は `dijkstra`・`dijkstra_path` を使います。`T` は 64 bit 以下の整数型、キーは非負です。頂点番号は非負の `int` です。

`shortest_path_detail::DistanceQueue<T>` は、通常の Dijkstra とトポロジカル前処理で共有するキュー型です。64 bit 以下の整数距離には `RadixQueue<T>`、それ以外には二分ヒープの `BinaryQueue<T>` を使います。いずれも `empty()`・`push(key,vertex)`・`pop()` を持ち、`pop()` は最小キーと頂点の `pair<T,int>` を返します。要素数を $$q$$ として、二分ヒープの追加・削除は $$O(\log q)$$ 時間、空間は $$O(q)$$ です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RadixQueue<T>()` | 下限 0 の空キューを作る | $$O(w)$$ |
| `empty()` | 要素がないか | $$O(1)$$ |
| `push(key,vertex)` | 最後に取り出したキー以上の要素を追加 | 償却 $$O(1)$$ |
| `pop()` | 最小キーと頂点の `pair<T,int>` を返して削除。非空が必要 | 償却 $$O(w)$$ |

キーの bit 数を $$w\le64$$、同時に保持する要素数の最大を $$q$$ として、追加は償却 $$O(1)$$、取り出しは償却 $$O(w)$$、空間は $$O(q+w)$$ です。内部で 32 bit から 64 bit の管理へ切り替わる場合に限り、その時点の要素数に比例する処理が 1 回加わります。
