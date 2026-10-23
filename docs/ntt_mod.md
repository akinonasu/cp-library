---
title: "NTT（法指定）"
documentation_of: //components/ntt_mod.hpp
redirect_from: /convolution/ntt_mod.hpp.html
---

```cpp
#include "lib/components/ntt_mod.hpp"

std::vector<std::uint32_t> a = {1, 2, 3, 4};
ntt_mod<998244353, 3>(a);
ntt_mod<998244353, 3>(a, true);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ntt_mod<mod,root>(a,inverse = false)` | `vector<uint32_t>` を指定法で NTT。`true` は正規化済みの逆変換 | $$O(n\log n+\log\mathrm{mod})$$ |

法 `mod` は $$2 < \mathrm{mod} < 2^{31}$$ を満たす素数、`root` は原始根です。要素は法未満、長さ $$n$$ は $$\mathrm{mod}-1$$ を割り切る 2 の冪とし、空配列は渡せません。入力を変換結果で上書きし、返り値は `void` です。逆変換には長さの逆数の乗算を含みます。

回転因子と Shoup 商を段ごとに準備します。x86 では AVX2 の有無を実行時に判定し、対応 CPU では 8 要素ずつ処理します。その他の CPU ではスカラー処理を使います。呼び出し元の `target` 指定は不要です。

時間は $$O(n\log n+\log\mathrm{mod})$$、追加空間は $$O(n)$$ です。
