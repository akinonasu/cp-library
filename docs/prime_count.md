---
title: 素数計数
documentation_of: //math/number_theory/prime_count.hpp
---

```cpp
#include "lib/math/number_theory/prime_count.hpp"

auto a = prime_count(10);             // 4: 2, 3, 5, 7
auto b = prime_count(100000000000ULL); // 4118054813
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prime_count(n)` | `n` 以下の素数の個数を `std::uint64_t` で返す | $$\displaystyle O\!\left(\frac{n^{3/4}}{\log n}\right)$$ |

`n` は非負の `std::uint64_t` です。上限自身を含み、`prime_count(0)` と `prime_count(1)` は 0、`prime_count(2)` は 1 です。[素数篩](https://qandori.github.io/cp-library/math/number_theory/prime_sieve.hpp.html)の `prime_sieve(n)` は `n` 未満を列挙するため、上限の扱いが異なります。

全ての素数を列挙せず、奇数だけの部分篩と整数商の再利用で計算します。小さい素因数を除いた候補だけを残し、最後に素数の積による補正を行います。繰り返し現れる除算は逆数乗算と整数の補正に置き換えます。整数平方根も積を使って補正するため、浮動小数点の丸めによる数え違いはありません。

前計算表や共有する可変状態は持たず、呼び出しごとに必要な領域を確保して終了時に解放します。64 bit 全域を整数演算で扱いますが、計算時間とメモリは入力に応じて増えます。特に最大値付近では数十 GB の作業領域を必要とするため、実用上の上限は環境の資源に依存します。$$n=10^{11}$$ での主要配列は約 4 MiB です。測定条件と結果は [ベンチマーク](https://github.com/qandori/cp-library/blob/main/test/benchmark/prime_count.md)を参照してください。

時間は $$\displaystyle O\!\left(\frac{n^{3/4}}{\log n}\right)$$、追加空間は $$\displaystyle O(\sqrt n)$$ です。`n < 2` では定数時間で終了します。
