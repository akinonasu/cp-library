---
title: "Dynamic Bit Vector"
documentation_of: //components/dynamic_bit_vector.hpp
redirect_from: /data_structure/dynamic_bit_vector.hpp.html
---

```cpp
#include "lib/components/dynamic_bit_vector.hpp"

DynamicBitVector<> bits(std::vector<bool>{0, 1, 0});
bits.insert(1, true);
bits.erase(0);
bits.set(0, false);
auto count = bits.rank1(3);
auto position = bits.select1(0);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `DynamicBitVector()` / `DynamicBitVector(a)` | 空列／`vector<bool>` から構築 |
| `build(a)` / `build(n,packed)` | bit 列から再構築／下位 bit から詰めた word 配列で再構築 |
| `size()` / `empty()` / `count()` | bit 数／空判定／1 の総数 |
| `access(i)` / `operator[](i)` | bit を取得 |
| `rank1(r)` / `rank0(r)` | $$[0,r)$$ の 1／0 の個数 |
| `rank1(l,r)` | `(rank1(l),rank1(r))` の組 |
| `select(b,k)` / `select1(k)` / `select0(k)` | 指定 bit の 0 始まり `k` 番目の位置。なければ -1 |
| `insert(i,b)` | bit を挿入し、挿入前の `rank1(i)` を返す |
| `erase(i)` / `set(i,b)` | bit を削除して返す／代入 |
| `push_back(b)` / `pop_back()` | 末尾に挿入／削除して bit を返す |
| `access_rank(i)` / `erase_rank(i)` / `set_rank(i,b)` | 操作前の `(rank1(i),bit)` を返し、取得／削除／代入 |
| `move_rank(from,to,b)` | 削除後の `to` に挿入し、削除前の順位・bit・挿入直前の順位を返す |
| `reserve(n)` / `clear()` | 領域予約／容量を残して空にする |
| `memory_usage()` | 確保済みノード領域のバイト数 |
| `swap(other)` | 内容を交換 |

添字と `select*` の順位は 0 始まりです。`rank1(r)`・`rank0(r)` は $$[0,r)$$ の個数、`rank1(l,r)` は両端の `rank1` の組です。`select1(k)`・`select0(k)` は該当する bit の位置を返し、なければ `-1` です。

`access(i)`・`operator[]` は bit の取得、`count()` は全体の 1 の個数です。`insert(i,b)` は挿入前の `rank1(i)`、`erase(i)` は削除した bit を返します。`access_rank(i)`・`erase_rank(i)`・`set_rank(i,b)` は操作前の `rank1(i)` と bit の組を返します。

`move_rank(from,to,b)` は `from` を削除した列の `to` に `b` を挿入し、操作前の `rank1(from)`・削除した bit・挿入直前の `rank1(to)` を tuple で返します。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。`push_back(b)`・`pop_back()` も利用できます。

要素数を $$n$$ として、構築は $$O(n)$$、挿入・移動は償却 $$O(\log n)$$、削除・更新・順位検索・取得は $$O(\log n)$$ です。サイズ・全体の個数・`clear` は $$O(1)$$、`reserve(N)` は $$O(N)$$、空間は $$O(N)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
