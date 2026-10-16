---
title: "Fully Retroactive Priority Queue"
documentation_of: //data_structure/retroactive_priority_queue.hpp
---

```cpp
#include "lib/data_structure/retroactive_priority_queue.hpp"

RetroactivePriorityQueue<long long> queue;
queue.insert(10, 5);
queue.insert(20, 2);
queue.insert_pop(30);
auto past = queue.top(20);
auto now = queue.top();
queue.set(10, 1);
queue.erase(30);
```

`RetroactivePriorityQueue<T, Compare = std::less<T>, Time = std::int64_t, BlockSize = 256>` は、過去の操作を変更し、任意時点の状態を問い合わせる fully retroactive な優先度付きキューです。`std::less<T>` では最小値、`std::greater<T>` では最大値を先頭にします。

時刻は64 bit以下の整数で、各時刻に置ける操作は1つです。問い合わせは指定時刻の操作を実行した直後の状態で、引数なしでは履歴の末尾を参照します。比較上同順位の要素は、挿入時刻が早い方を優先します。

| 操作 | 内容 |
| --- | --- |
| `insert(t,x)` | 時刻 $$t$$ に `push(x)` を追加 |
| `insert_pop(t)` | 時刻 $$t$$ に先頭要素の削除を追加 |
| `set(t,x)` | 既存の `push` の値を変更 |
| `erase(t)` | 時刻 $$t$$ の操作を取り消す |
| `top(t)`・`top()` | 指定時刻・現在の先頭を `std::optional<T>` で取得 |
| `size(t)`・`empty(t)` | 指定時刻の要素数・空判定 |
| `size()`・`empty()` | 現在の要素数・空判定 |
| `contains_time(t)`・`operation_count()` | 操作の存在・操作数 |
| `assign(updates)` | 操作履歴をまとめて設定 |
| `reserve(m)`・`clear()` | 操作ノードを予約・履歴を初期化 |
| `memory_usage()` | 確保済み領域のバイト数 |

履歴変更は成功時に `true` を返します。時刻の衝突、対象操作の不在、`set` の対象が `pop` の場合や、変更後の履歴で空キューへの削除が発生する場合は `false` を返し、状態を保ちます。

`updates` は `std::vector<Queue::update_type>` で、要素は `{時刻, std::optional<T>}` です。`nullopt` は削除操作です。順不同で渡せます。`Queue(updates, compare = Compare{}, seed = 0)` でも構築できます。このコンストラクタには有効な履歴を渡します。空列の構築は `Queue(compare = Compare{}, seed = 0)` です。削除ノードは再利用し、`clear()` は操作ノードの確保容量を残します。

現在の操作数を $$m$$、過去の最大操作数を $$M$$、ブロック幅を $$B$$ とします。構築は $$O(m\log m)$$、`assign` は $$O(M+m\log m)$$、履歴変更は償却期待 $$O(B\log B+m/B+\log m)$$、`top` は期待 $$O(B\log B+(m/B)\log B\log m)$$ です。`size(t)`・`empty(t)`・`contains_time` は $$O(\log m)$$、現在の `size`・`empty`・`operation_count` は $$O(1)$$、`reserve`・`clear` は $$O(M)$$、`memory_usage` は $$O(m/B)$$ です。空間は $$O(M)$$（予約容量が大きい場合はその容量分）です。$$B\approx\sqrt m$$ とすると更新は償却期待 $$O(\sqrt m\log m)$$、`top` は期待 $$O(\sqrt m\log^2m)$$ です。比較・値のコピーを $$O(1)$$ とします。
