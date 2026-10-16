---
title: "Quad Bit Vector"
documentation_of: //components/quad_bit_vector.hpp
redirect_from: /data_structure/quad_bit_vector.hpp.html
---

```cpp
#include "lib/components/quad_bit_vector.hpp"

std::vector<std::uint32_t> a = {0, 3, 1, 3};
QuadBitVector bits(a.size());
bits.build(a, 0);
auto count = bits.rank(4, 3);
```

`build(a, shift)` は各要素の下から `shift` 番目と次のビットを取り出します。`a.size()` は構築時の長さと一致させ、`shift` は 0〜30 とします。

x86 の AVX2 対応 CPU では、構築時に 8 要素ずつビットを取り出します。対応状況は実行時に判定し、その他の CPU や短い配列ではスカラー処理を使います。

`rank(r, digit)` は $$[0,r)$$ にある `digit` の個数、`below(r, digit)` は `digit` 未満の個数、`ranks(r)` は 4 種類すべての個数です。`digit` は 0〜3 です。

長さを $$n$$ として、構築は $$O(n)$$、問い合わせは $$O(1)$$、空間は $$O(n)$$ bit です。
