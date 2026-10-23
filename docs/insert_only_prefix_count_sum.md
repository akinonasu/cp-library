---
title: 挿入のみの区間和・個数（補助部品）
documentation_of: //components/insert_only_prefix_count_sum.hpp
---

[値の上限付き区間和](offline_range_sum.md)で使う、値の挿入と区間の和・個数の集計部品です。局所的な集計値が 64 bit に収まることを確認してから利用します。

`insert_only_prefix_detail::Packed(n,minimum,bits)` の `insert(p,delta)` は、位置 `p` に値 `minimum+delta` を挿入します。各位置への挿入は一度とし、各 65536 要素の区画内で個数と `delta` の和を連結した値が 64 bit に収まる `bits` を渡します。`prefix(r)` は `[0,r)`、`fold(l,r)` は `[l,r)` の集計を `CountSum{sum,count}` で返します。`sum` は `__int128_t` です。

構築・空間は $$O(n)$$、挿入・集計は $$O(\log n)$$ です。
