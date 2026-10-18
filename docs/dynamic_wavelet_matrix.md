---
title: "Dynamic Wavelet Matrix"
documentation_of: //data_structure/dynamic_wavelet_matrix.hpp
---

```cpp
#include "lib/data_structure/dynamic_wavelet_matrix.hpp"

DynamicWaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
wm.insert(2, -7);
wm.erase(0);
wm.set(1, 6);
auto value = wm.kth_smallest(0, wm.size(), 2);
auto frequency = wm.count(0, wm.size(), 1);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の順位は 0 始まりです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicWaveletMatrix()` / `DynamicWaveletMatrix(a)` | 空／配列から構築 | 空: $$O(1)$$、配列: $$O(Bn)$$ |
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
| `memory_usage()` | 確保済みノード領域のバイト数 | $$O(1)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

型 `T` は 64 bit 以下の整数型です。`DynamicWaveletMatrix<T,B>` で値の bit 数を指定でき、既定は `T` の bit 数です。符号付き型の値域は $$[-2^{B-1},2^{B-1})$$、符号なし型は $$[0,2^B)$$ です。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。

特定の値の個数には `count(l,r,x)` を使います。`range_freq(l,r,lower,upper)` は両端の共通する bit の経路を共有しますが、`count` は `x` の経路だけをたどります。`x+1` を計算しないため、整数型の最大値もそのまま渡せます。

`count` は先頭の全要素で共通する bit を順位検索なしで判定します。区間が最初から 1 要素なら、bit と順位をまとめて取得して一致を調べます。挿入・削除・代入後も同じ API を使えます。

`select` は候補が 1 要素になった後、残りの bit の一致を調べます。独自の `BitVector` に `access_rank` がない場合、`select` は従来の順位検索を使います。他の操作の `BitVector` 要件は変わりません。

x86_64 でコンパイル時に BMI2 が有効なら word 内の `select` に PDEP、AVX2 が有効なら 32・64 bit 整数の構築に SIMD を使います。それ以外では従来のスカラー処理を使い、Mac ARM でも利用できます。これらの命令を実行時に判定する仕組みではないため、コンパイルオプションは実行する CPU に合わせてください。

要素数を $$n$$、bit 数を $$B$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・問い合わせは $$O(B\log n)$$ です。サイズ取得は $$O(1)$$、`clear` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
