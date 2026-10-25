---
title: 整数 k 乗根
documentation_of: //math/number_theory/kth_root.hpp
redirect_from:
- /math/kth_root.hpp.html
---

```cpp
#include "lib/math/number_theory/kth_root.hpp"

auto a = kth_root(215, 3);                     // 5
auto b = kth_root(216, 3);                     // 6
auto c = kth_root(18446744073709551615ULL, 2);  // 4294967295
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `kth_root(n,k)` | $$\lfloor n^{1/k}\rfloor$$ を `uint64_t` で返す | $$O(1)$$（64 bit 固定） |

`n` と `k` は `uint64_t` で、$$0\le n<2^{64}$$、$$k\ge1$$ が条件です。負数と `k == 0` は扱いません。`n == 0` の答えは 0、`k == 1` の答えは `n` です。`k` は 64 を超えても構いません。

平方根・4 乗根・8 乗根は `sqrt`、それ以外の 3〜15 乗根は浮動小数の近似から候補を求め、整数演算で確定します。平方根の比較には `__uint128_t`、それ以外の累乗にはオーバーフローを検出する 64 bit 乗算を使います。候補が隣接する整数に補正できない場合は二分探索します。16 乗根以降は固定の小さな閾値表とビット長による判定で求めます。完全 k 乗数の直前でも丸め誤差は残りません。GCC・Clang と Mac の Apple Clang に対応します。

64 bit 固定のため時間・追加空間は $$O(1)$$ です。3〜15 乗根で二分探索まで必要になった場合の整数乗算回数は、$$n\ge2$$ として最悪 $$O((\log n/k)\log k)$$ です。
