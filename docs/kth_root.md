---
title: 整数 k 乗根
documentation_of: //math/kth_root.hpp
redirect_from:
- /math/number_theory/kth_root.hpp.html
- /lightweight/math/number_theory/kth_root.hpp.html
library_category: math/number_theory
---

```cpp
#include "lib/math/kth_root.hpp"

auto a = kth_root(215, 3);                     // 5
auto b = kth_root(216, 3);                     // 6
auto c = kth_root(18446744073709551615ULL, 2);  // 4294967295
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `kth_root(n,k)` | $$\lfloor n^{1/k}\rfloor$$ を `uint64_t` で返す | $$O(1)$$（64 bit 固定） |

`n` と `k` は `uint64_t` で、$$0\le n<2^{64}$$、$$k\ge1$$ が条件です。負数と `k == 0` は扱いません。`n == 0` の答えは 0、`k == 1` の答えは `n` です。`k` は 64 を超えても構いません。

返り値は整数演算で確定し、完全 k 乗数の直前でも浮動小数点の丸め誤差は残りません。GCC・Clang と Mac の Apple Clang に対応します。

64 bit 固定のため時間・追加空間は $$O(1)$$ です。3〜15 乗根で二分探索まで必要になった場合の整数乗算回数は、$$n\ge2$$ として最悪 $$O((\log n/k)\log k)$$ です。
