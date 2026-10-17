---
title: "SplitMix64"
documentation_of: //components/splitmix64.hpp
redirect_from: /math/splitmix64.hpp.html
---

```cpp
#include "lib/components/splitmix64.hpp"

SplitMix64 rng(123);
auto random = rng();
auto hash = splitmix64(key);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `SplitMix64(seed = 0)` | 64 bit 乱数生成器を構築 |
| `rng()` | 次の `uint64_t` 乱数を返す |
| `splitmix64(x)` | 64 bit 整数を混合して返す |

`SplitMix64(seed = 0)` は 64 bit 乱数生成器です。`splitmix64(x)` は加算前処理を含まない 64 bit のミキサーです。

生成・混合は時間・空間ともに $$O(1)$$ です。
