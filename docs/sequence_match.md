---
title: "列の一致比較"
documentation_of: //components/sequence_match.hpp
---

```cpp
#include "lib/components/sequence_match.hpp"

std::vector<int> s{1, 2, 1, 2, 3};
sequence_match_detail::Matcher<int> match(s);
int prefix = match.forward(0, 2, 3);  // 2
int suffix = match.backward(2, 4, 2); // 2
```

列を所有しない内部部品です。元の列は比較中に有効である必要があります。`forward(i, j, limit)` は位置 `i, j` からの前方一致長、`backward(i, j, limit)` は位置 `i, j` の直前からの後方一致長です。比較する全要素が列の範囲内になる `limit >= 0` を指定します。

一意な表現を持つ整数は 64 bit 単位で比較します。x86_64 では CPU の対応を確認して長い比較に AVX2 を使います。`Matcher<T, false>` で AVX2 を無効にできます。一般の要素は `==` で比較します。

`index_runs(n)` は同一要素区間の境界を前計算します。`forward_counted`・`backward_counted` は `(一致長, 比較量)` を返し、最初の同一要素区間をまとめて飛ばします。前計算後は元の列を変更しないでください。`z_table`・`prefix_matches` は一致比較関数を受け取る内部の Z algorithm です。

長さを $$n$$ として、通常の比較は最悪 $$O(\mathrm{limit})$$ 時間・$$O(1)$$ 追加空間、同一要素区間の前計算は $$O(n)$$ 時間・空間、Z algorithm は合計 $$O(n)$$ 要素比較です。
