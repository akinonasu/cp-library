---
title: "Partially Retroactive Priority Queue"
documentation_of: //data_structure/partially_retroactive_priority_queue.hpp
---

```cpp
#include "lib/data_structure/partially_retroactive_priority_queue.hpp"

PartiallyRetroactivePriorityQueue<long long> queue;
queue.insert(10, 5);
queue.insert(20, 2);
queue.insert_pop(30);
auto now = queue.top();
queue.set(10, 1);
queue.erase(30);
```

`PartiallyRetroactivePriorityQueue<T, Compare = std::less<T>, Time = std::int64_t>` は、過去の操作を変更し、現在の状態を問い合わせる優先度付きキューです。`std::less<T>` では最小値、`std::greater<T>` では最大値を先頭にします。同順位の要素は挿入時刻順です。時刻は64 bit以下の整数で、各時刻に置ける操作は1つです。

| 操作 | 内容・返り値 |
| --- | --- |
| `PartiallyRetroactivePriorityQueue(compare = {},seed = 0)` | 空の履歴を構築 |
| `PartiallyRetroactivePriorityQueue(updates,compare = {},seed = 0)` | 有効な操作履歴から構築 |
| `insert(t,x)` | 時刻 `t` に `push(x)` を追加 |
| `insert_pop(t)` | 時刻 `t` に先頭要素の削除を追加 |
| `set(t,x)` | 既存の `push` の値を変更 |
| `erase(t)` | 時刻 `t` の操作を取り消す |
| `top()` | 現在の先頭を `optional<T>` で取得。空なら `nullopt` |
| `size()` / `empty()` | 現在の要素数／空判定 |
| `contains_time(t)` / `operation_count()` | 時刻 `t` の操作の有無／操作数 |
| `assign(updates)` | 操作履歴をまとめて設定。無効なら false を返して元の履歴を保つ |
| `reserve(m)` / `clear()` | 領域予約／容量を残して履歴を初期化 |
| `memory_usage()` | 確保済み領域のバイト数 |
| `swap(other)` | 内容・比較関数・乱数生成器の状態を交換 |
| `top_ptr()` | 現在の先頭の `const T*`。空なら `nullptr`。次の変更操作まで有効 |

履歴変更は成功時に `true` を返します。時刻の衝突、対象操作の不在、`set` の対象が `pop` の場合や、空キューへの削除が発生する変更は `false` を返し、履歴を保ちます。

`updates` は `std::vector<Queue::update_type>` で、要素は `{時刻, std::optional<T>}`、`nullopt` は削除操作です。順不同で渡せます。`Queue(updates, compare = Compare{}, seed = 0)` による構築には有効な履歴を渡します。空列の構築は `Queue(compare = Compare{}, seed = 0)` です。`top_ptr()` の戻り値は次の変更操作まで使用できます。削除ノードは再利用し、`clear()` は確保容量を残します。

操作数を $$m$$、過去の最大操作数を $$M$$ とします。履歴変更は償却期待 $$O(\log m)$$、`contains_time` は期待 $$O(\log m)$$、現在の問い合わせ・`operation_count`・`memory_usage` は $$O(1)$$、構築は $$O(m\log m)$$、`assign` は $$O(M+m\log m)$$、`reserve`・`clear` は $$O(M)$$ です。空間は $$O(M)$$（予約容量が大きい場合はその容量分）です。比較・値のコピーを $$O(1)$$ とします。
