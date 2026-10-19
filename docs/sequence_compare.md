---
title: "共通接頭辞・接尾辞の比較"
documentation_of: //components/sequence_compare.hpp
---

```cpp
#include "lib/components/sequence_compare.hpp"
#include <string_view>

std::string_view a = "abcde", b = "abcxy";
int prefix = sequence_compare_detail::prefix(a.data(), b.data(), 5); // 3
int suffix = sequence_compare_detail::suffix(a.data() + a.size(),
                                            b.data() + b.size(), 5); // 0
```

2 つの配列の一致部分を調べる内部部品です。Run 列挙・LCS・編集距離で共有しています。文字列には `char`、整数列にはその要素型のポインターを渡します。返り値と `limit` は要素数です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `sequence_compare_detail::prefix(a,b,limit)` | `a[0]` と `b[0]` から前方に比較した一致長 | $$O(\mathrm{limit})$$ |
| `sequence_compare_detail::suffix(a_end,b_end,limit)` | 末尾の次の位置から後方に比較した一致長 | $$O(\mathrm{limit})$$ |
| `prefix<false>(a,b,limit)` / `suffix<false>(a_end,b_end,limit)` | AVX2 を使わずに同じ比較を実行 | $$O(\mathrm{limit})$$ |

`limit >= 0` とし、前方では `[a,a+limit)`・`[b,b+limit)`、後方では `[a_end-limit,a_end)`・`[b_end-limit,b_end)` が有効な配列範囲になるように指定してください。`limit == 0` では読み込まずに `0` を返し、型付きの null ポインターも渡せます。配列の外側に余分な領域を確保する必要はありません。

一意な表現を持つ整数型は 64 bit 単位で比較し、長く一致する場合は CPU の対応を確認して AVX2 を使います。一般の型は `==` で比較します。例えば浮動小数点の正負のゼロは等しい値として扱います。AVX2 非対応 CPU や Mac の ARM CPU では通常の比較を使います。

要素比較を $$O(1)$$ として、時間は最悪 $$O(\mathrm{limit})$$、追加空間は $$O(1)$$ です。
