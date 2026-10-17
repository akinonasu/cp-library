---
title: "Default Init Allocator"
documentation_of: //components/default_init_allocator.hpp
redirect_from: /data_structure/default_init_allocator.hpp.html
---

```cpp
#include "lib/components/default_init_allocator.hpp"
#include <vector>

std::vector<int, DefaultInitAllocator<int>> a(n);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `DefaultInitAllocator<T>()` | `std::vector` などに渡すアロケータを構築 |
| `construct(p)` | `p` の位置に要素をデフォルト初期化。通常はコンテナから呼ばれる |
| `allocate(n)` / `deallocate(p,n)` | `std::allocator<T>` と同じ領域確保／解放 |

要素を値初期化せずデフォルト初期化します。整数などの要素は読み出す前に代入してください。

$$n$$ 要素の初期化は自明な型なら $$O(1)$$、その他は $$O(n)$$、空間は $$O(n)$$ です。メモリ確保のコストは除きます。
