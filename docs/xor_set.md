---
title: XOR の最小値・最大値を求める整数集合
documentation_of: //data_structure/xor_set.hpp
library_category: data_structure/associative
---

整数の追加・削除と、指定値との XOR が最小／最大になる要素の取得に対応する集合です。同じ値を追加しても重複しません。

```cpp
#include "lib/data_structure/xor_set.hpp"
#include <cassert>

int main() {
	XorSet<30> values;
	assert(!values.min() && !values.max());
	values.insert(3);
	values.insert(10);
	assert(values.min() == 3 && values.max() == 10);
	assert(values.argmin_xor(8) == 10);
	assert(values.min_xor(8) == 2);
	assert(values.argmax_xor(8) == 3);
	assert(values.max_xor(8) == 11);
	values.erase(3);
}
```

`XorSet<Bits>` の `Bits` は既定で 30、範囲は 1〜64 です。`Bits <= 31` では値の型 `value_type` は `std::uint32_t`、`Bits >= 32` では `std::uint64_t` です。格納できる値は `[0,2^Bits)` です。`insert`・`erase`・`contains` にはこの範囲の値を渡し、XOR の問い合わせには任意の `value_type` の値を渡せます。`Bits = 64` では `UINT64_MAX` を含む `uint64_t` の全範囲を使えます。

```cpp
#include "lib/data_structure/xor_set.hpp"
#include <cassert>
#include <cstdint>

int main() {
	XorSet<64> values;
	values.insert(UINT64_MAX);
	values.insert(UINT64_C(1) << 63);
	assert(values.argmin_xor(0) == (UINT64_C(1) << 63));
	assert(values.max_xor(0) == UINT64_MAX);
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `XorSet<Bits>()` | 空集合を作成 |
| `size()` / `empty()` | 要素数を `int64_t` で返す／空判定 |
| `contains(x)` | `x` が含まれるか |
| `insert(x)` | `x` を追加し、追加できた場合に `true` |
| `erase(x)` | `x` を削除し、含まれていた場合に `true` |
| `argmin_xor(x)` / `argmax_xor(x)` | `x` との XOR が最小／最大になる集合の要素を `value_type` で返す |
| `min_xor(x)` / `max_xor(x)` | `x` と集合の要素との XOR の最小値／最大値を `value_type` で返す |
| `min()` / `max()` | 最小要素／最大要素を `std::optional<value_type>` で返す。空集合は `nullopt` |
| `clear()` | 容量を保って空集合にする |
| `swap(other)` | 内容を交換 |
| `memory_usage()` | 確保済みの動的領域のバイト数 |

`argmin_xor`・`argmax_xor`・`min_xor`・`max_xor` は空でない集合に対して呼んでください。`min()`・`max()` は空集合にも使え、重複挿入は結果に影響しません。コピーは独立した集合を作り、移動元は再利用できる空集合になります。

`Bits <= 31` では、$$U=2^{Bits}$$、これまでに確保した葉の最大数を $$L$$ とします。最初の挿入は管理領域の確保に $$O(U/512)$$ 時間、その後の挿入は償却 $$O(\log_{64}U)$$ 時間、削除・`min`・`max` と XOR の問い合わせは $$O(\log_{64}U)$$ 時間です。`contains`・`size`・`empty`・交換は $$O(1)$$、コピーと `clear` は $$O(U/512+L)$$ 時間、使用空間は $$O(U/512+L)$$ 個の固定長整数です。`Bits` が 9 以下なら管理領域は定数サイズです。

`32 <= Bits <= 64` では、使用ビット数を $$B=Bits$$、確保済みの管理領域の要素数を $$L$$ とします。挿入は償却 $$O(B)$$、削除・所属判定・`min`・`max`・XOR の問い合わせは最悪 $$O(B)$$、要素数・空判定・交換・移動構築は $$O(1)$$ 時間です。コピーは $$O(L)$$ 時間・空間、`clear` は $$O(L)$$ 時間以内で、予約領域を残します。保持空間は $$O(L)$$ です。管理領域は、これまでの最大同時要素数に比例し、値の範囲全体に比例する領域は確保しません。削除した領域は後の追加で再利用するため、要素数の最大値を増やさない追加・削除を繰り返しても保持領域は増え続けません。割り当てる内部要素数が `INT_MAX` を超える場合は `std::length_error` を送出します。

コピー代入・移動代入には、代入先の元の領域を解放する時間もかかります。`min`・`max` は空集合なら $$O(1)$$ 時間です。移動構築・交換・問い合わせの追加空間は $$O(1)$$ です。
