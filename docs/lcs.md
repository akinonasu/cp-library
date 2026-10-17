---
title: "Longest Common Subsequence"
documentation_of: //string/lcs.hpp
---

```cpp
#include "lib/string/lcs.hpp"

int length = lcs_length(s, t);
std::string sequence = lcs(s, t);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `lcs_length(s,t)` | 最長共通部分列の長さ |
| `lcs(s,t)` | 最長共通部分列を 1 つ `std::string` で返す |

`std::string_view` で 2 つの文字列を受け取ります。`lcs` は最長共通部分列を 1 つ返します。文字はバイト単位で扱います。

長い方の長さを $$n$$、短い方を $$m$$ として、長さ計算は $$O(n\lceil m/64\rceil)$$、復元は $$O(n\lceil m/64\rceil+n\log n)$$、補助空間はともに $$O(n+m)$$ です。
