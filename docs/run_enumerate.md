---
title: "Run 列挙"
documentation_of: //string/run_enumerate.hpp
---

```cpp
#include "lib/string/run_enumerate.hpp"

auto runs = run_enumerate("ababa");
for (auto [period, l, r] : runs) {
	// {2, 0, 5}
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `run_enumerate(s)` | run の一覧を `vector<Run>` で返す。文字列・vector・span に対応 | $$O(n\log n)$$ |
| `run.period` / `run.l` / `run.r` | 最小周期／半開区間の左端／右端 | $$O(1)$$ |
| `a == b` / `a != b` / `a < b` / `a <= b` / `a > b` / `a >= b` | `Run` を `(period,l,r)` の辞書順で比較 | $$O(1)$$ |

`run_enumerate(s)` は `std::vector<Run>` を返します。各要素は最小周期 `period` と半開区間 `[l, r)` です。区間長は周期の 2 倍以上で、左右どちらにも同じ周期のまま延長できません。結果は重複がなく、`(period, l, r)` の辞書順です。空列・長さ 1 の列では空になります。

`std::string_view`・`std::vector<T>`（`bool` を除く）・`std::span<const T>` を渡せます。文字列はバイト単位で扱い、NUL を含めて全バイトを使用できます。一般の要素には、一致判定と整合する全順序が必要です。長さは `INT_MAX` 未満としてください。

Lyndon 文字列に基づく列挙を使い、比較量が増えた場合は Z algorithm による分割統治へ切り替えます。通常の列挙では先頭要素が異なる候補を除外し、不要な延長計算を省きます。長い同一要素区間はまとめて処理します。ハッシュを使わず、結果は決定的です。一様な列の判定では、対応する x86_64 CPU で AVX2 を使います。それ以外の環境では通常の比較に切り替わります。

長さを $$n$$ として、時間は最悪 $$O(n\log n)$$、空間は出力を含めて $$O(n)$$ です。
