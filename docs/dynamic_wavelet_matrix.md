---
title: Dynamic Wavelet Matrix
documentation_of: //data_structure/dynamic_wavelet_matrix.hpp
redirect_from:
- /data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp.html
- /lightweight/data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp.html
library_category: data_structure/wavelet_matrix
---

要素の途中への挿入・削除・代入と、区間内の値の個数や、小さい順で k 番目の値を求める列です。

```cpp
#include "lib/data_structure/dynamic_wavelet_matrix.hpp"

DynamicWaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
wm.insert(2, -7);
wm.erase(0);
wm.set(1, 6);
auto value = wm.kth_smallest(0, wm.size(), 2);
auto frequency = wm.count(0, wm.size(), 1);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の順位は 0 始まりです。`kth_*` の `k` は区間の長さ未満とします。

列長は `INT_MAX` 以下、挿入は `0 <= i <= size()`、削除・代入・取得は `0 <= i < size()`、区間は `0 <= l <= r <= size()` とします。`kth_*` は `0 <= k < r-l` が必要です。末尾の削除は非空の列に対して呼んでください。重複は個数分だけ数え、`select(x,k)` は元の列を左から数えた `k` 番目の出現位置を返します。空区間の個数・頻度は 0 です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicWaveletMatrix()` / `DynamicWaveletMatrix(a)` | 空／配列から構築 | 空: $$O(B)$$、配列: $$O(Bn)$$ |
| `build(a)` | 配列で再構築 | $$O(Bn)$$ |
| `insert(i,x)` / `erase(i)` / `set(i,x)` | 挿入／削除／代入。`erase` は削除した値を返す | 挿入・代入: 償却 $$O(B\log n)$$、削除: $$O(B\log n)$$ |
| `push_back(x)` / `pop_back()` | 末尾への挿入／削除。`pop_back` は削除した値を返す | 追加: 償却 $$O(B\log n)$$、削除: $$O(B\log n)$$ |
| `access(i)` / `operator[](i)` | 要素の取得 | $$O(B\log n)$$ |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ にある `x` の個数。値域外なら 0 | $$O(B\log n)$$ |
| `select(x,k)` | 0 始まり `k` 番目の `x` の位置。なければ -1 | $$O(B\log n)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の 0 始まり `k` 番目 | $$O(B\log n)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の個数 | $$O(B\log n)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の個数 | $$O(B\log n)$$ |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。なければ `nullopt` | $$O(B\log n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `reserve(n)` / `clear()` | 領域予約／容量を残して空にする | reserve: $$O(BN)$$、clear: $$O(B)$$ |
| `memory_usage()` | 確保済みノード領域のバイト数 | $$O(B)$$ |
| `swap(other)` | 内容を交換 | $$O(B)$$ |

型 `T` は `bool` を除く 64 bit 以下の整数型です。`DynamicWaveletMatrix<T,B>` で値の bit 数を指定でき、既定は `T` の bit 数です。$$1\le B\le\mathrm{sizeof}(T)\cdot8$$ とし、符号付き型の値域は $$[-2^{B-1},2^{B-1})$$、符号なし型は $$[0,2^B)$$ です。格納する値はこの範囲に収めてください。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。copy・move・`swap` に対応し、move 元は空の列として再利用できます。

特定の値の個数には `count(l,r,x)` を使います。整数型の最大値も渡せます。`range_freq(l,r,lower,upper)` は `lower >= upper` なら 0 を返します。

`prev_value(l,r,upper)` は境界を含まず、`next_value(l,r,lower)` は境界を含みます。空区間や条件を満たす値がない場合は `nullopt` です。`select(x,k)` は `x` が値域外の場合や `k` が負の場合も `-1` を返します。

include 前に `DYNAMIC_QUAD_DISABLE_SIMD` を定義すると、このクラスが CPU の機能を実行時に調べて選ぶ専用処理を無効にできます。`DYNAMIC_QUAD_DISABLE_AVX512` は、一部の CPU にある AVX-512 命令を使う専用処理を無効にします。

第3テンプレート引数に独自の `BitVector` を指定できます。`move_rank` と `access_rank` は省略できます。CPU 拡張命令をコンパイル時に指定する場合は、実行する CPU に合ったオプションを使ってください。

要素数を $$n$$、bit 数を $$B$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・クエリは $$O(B\log n)$$ です。サイズ取得は $$O(1)$$、`clear`・`memory_usage`・`swap` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
