---
title: "Edit Distance"
documentation_of: //string/edit_distance.hpp
---

```cpp
#include "lib/string/edit_distance.hpp"

int distance = edit_distance("kitten", "sitting"); // 3
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `edit_distance(s,t)` | 挿入・削除・置換のコストを各 1 とする編集距離 | $$O(n\lceil m/64\rceil)$$ |

`std::string_view` で 2 つの文字列を受け取り、挿入・削除・置換をそれぞれコスト 1 とする編集距離を返します。文字はバイト単位で扱います。

共通する接頭辞・接尾辞を先に除き、残った部分を計算します。長い一致部分は Run 列挙と共通の比較部品で処理し、x86_64 では対応 CPU で AVX2 を使います。Mac の ARM CPU でも追加の設定なしで使えます。

長い方の長さを $$n$$、短い方を $$m$$ として、時間は $$O(n\lceil m/64\rceil)$$、補助空間は $$O(m)$$ です。
