---
title: Partially Retroactive Priority Queue
documentation_of: //data_structure/partially_retroactive_priority_queue.hpp
redirect_from:
- /data_structure/priority_queue/partially_retroactive_priority_queue.hpp.html
- /data_structure/advanced/partially_retroactive_priority_queue.hpp.html
- /lightweight/data_structure/advanced/partially_retroactive_priority_queue.hpp.html
library_category: data_structure/advanced
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

現在の要素集合の総和などは、変更通知を使って外部で管理できます。

```cpp
long long sum = 0;
auto changed = [&](std::int64_t, const long long &value, bool added) {
	sum += added ? value : -value;
};
PartiallyRetroactivePriorityQueue<long long> queue;
queue.insert(10, 5, changed);     // sum = 5
queue.insert(20, 2, changed);     // sum = 7
queue.insert_pop(30, changed);   // sum = 5
queue.set(10, 1, changed);       // sum = 2
queue.erase(30, changed);        // sum = 3
```

`PartiallyRetroactivePriorityQueue<T, Compare = std::less<T>, Time = std::int64_t>` は、過去の操作を変更し、現在の状態をクエリる優先度付きキューです。`std::less<T>` では最小値、`std::greater<T>` では最大値を先頭にします。同順位の要素は挿入時刻順です。時刻は64 bit以下の整数で、各時刻に置ける操作は1つです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PartiallyRetroactivePriorityQueue(compare = {},seed = 0)` | 空の履歴を構築 | $$O(1)$$ |
| `PartiallyRetroactivePriorityQueue(updates,compare = {},seed = 0)` | 有効な操作履歴から構築 | $$O(m\log m)$$ |
| `insert(t,x)` | 時刻 `t` に `push(x)` を追加 | 償却期待 $$O(\log m)$$ |
| `insert_pop(t)` | 時刻 `t` に先頭要素の削除を追加 | 償却期待 $$O(\log m)$$ |
| `set(t,x)` | 既存の `push` の値を変更 | 償却期待 $$O(\log m)$$ |
| `erase(t)` | 時刻 `t` の操作を取り消す | 償却期待 $$O(\log m)$$ |
| `insert(t,x,f)` / `insert_pop(t,f)` | 操作を追加し、現在の集合の変化を `f` に通知 | 償却期待 $$O(\log m)$$ |
| `set(t,x,f)` / `erase(t,f)` | 操作を変更・取消し、現在の集合の変化を `f` に通知 | 償却期待 $$O(\log m)$$ |
| `top()` | 現在の先頭を `optional<T>` で取得。空なら `nullopt` | $$O(1)$$ |
| `size()` / `empty()` | 現在の要素数／空判定 | $$O(1)$$ |
| `contains_time(t)` / `operation_count()` | 時刻 `t` の操作の有無／操作数 | contains_time: 期待 $$O(\log m)$$、operation_count: $$O(1)$$ |
| `assign(updates)` | 操作履歴をまとめて設定。無効なら false を返して元の履歴を保つ | $$O(M+m\log m)$$ |
| `assign(updates,f)` | 履歴を設定し、旧集合の削除・新集合の追加を通知。無効なら通知しない | $$O(M+m\log m)$$ |
| `reserve(m)` / `clear()` | 領域予約／容量を残して履歴を初期化 | $$O(M)$$ |
| `clear(f)` | 現在の全要素の削除を通知して初期化 | $$O(M)$$ |
| `for_each(f)` | 現在の全要素について `f(挿入時刻,const T&)` を挿入時刻順に呼ぶ | $$O(m)$$ |
| `memory_usage()` | 確保済み領域のバイト数 | $$O(1)$$ |
| `swap(other)` | 内容・比較関数・乱数生成器の状態を交換 | $$O(1)$$ |
| `top_ptr()` | 現在の先頭の `const T*`。空なら `nullptr`。次の変更操作まで有効 | $$O(1)$$ |

履歴変更は成功時に `true` を返します。時刻の衝突、対象操作の不在、`set` の対象が `pop` の場合や、空キューへの削除が発生する変更は `false` を返し、履歴を保ちます。

変更通知は `f(挿入時刻,const T&,bool added)` です。`added=true` は現在の集合への追加、`false` は現在の集合からの削除です。指定した操作の値ではなく、過去の変更の結果として現在の集合で増減する要素を通知します。同じ値も挿入時刻で区別でき、総和・XOR・頻度・外部の集合などを管理できます。コールバックは呼び出しごとに渡し、キューには保存しません。コピーできないコールバックにも対応します。

`insert`・`insert_pop`・`erase` は成功時に 1 要素、`set` は削除・追加を各高々 1 要素通知します。`set` は削除を先に通知し、内部の一時的な追加と削除は通知しません。変更を拒否した場合は通知しません。`assign` は同じ要素でも旧集合から一度削除して新集合へ追加します。既存のキューから集計を始める場合は `for_each` で初期化できます。継続して集計する場合は、全ての変更に同じ通知処理を渡してください。`swap`・コピー・ムーブ・通知なしの変更は通知しません。

通知中の参照はその呼び出し中だけ有効です。コールバックから同じキューを参照・変更したり、例外を投げたりしないでください。通知なしの API は従来どおり使えます。

`updates` は `std::vector<Queue::update_type>` で、要素は `{時刻, std::optional<T>}`、`nullopt` は削除操作です。順不同で渡せます。`Queue(updates, compare = Compare{}, seed = 0)` による構築には有効な履歴を渡します。空列の構築は `Queue(compare = Compare{}, seed = 0)` です。`top_ptr()` の戻り値は次の変更操作まで使用できます。削除ノードは再利用し、`clear()` は確保容量を残します。

操作数を $$m$$、過去の最大操作数を $$M$$ とします。履歴変更は償却期待 $$O(\log m)$$、`contains_time` は期待 $$O(\log m)$$、現在のクエリ・`operation_count`・`memory_usage` は $$O(1)$$、構築は $$O(m\log m)$$、`assign` は $$O(M+m\log m)$$、`reserve`・`clear` は $$O(M)$$、`for_each` は $$O(m)$$ です。変更通知付きも通知処理が $$O(1)$$ なら同じ計算量です。空間は $$O(M)$$（予約容量が大きい場合はその容量分）です。比較・値のコピーを $$O(1)$$ とします。
